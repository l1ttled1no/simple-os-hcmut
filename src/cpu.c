#include <stdio.h>
#include "cpu.h"
#include "mem.h"
#include "mm.h"

int calc(struct pcb_t * proc) {
	return ((unsigned long)proc & 0UL);
}

int alloc(struct pcb_t * proc, uint32_t size, uint32_t reg_index) {
	addr_t addr = alloc_mem(size, proc);
	if (addr == 0) {
		return 1;
	}else{
		proc->regs[reg_index] = addr;
		return 0;
	}
}

int free_data(struct pcb_t * proc, uint32_t reg_index) {
	return free_mem(proc->regs[reg_index], proc);
}

int read(
		struct pcb_t * proc, // Process executing the instruction
		uint32_t source, // Index of source register
		uint32_t offset, // Source address = [source] + [offset]
		uint32_t destination) { // Index of destination register
	
	BYTE data;
	if (read_mem(proc->regs[source] + offset, proc,	&data)) {
		proc->regs[destination] = data;
		return 0;		
	}else{
		return 1;
	}
}

int write(
		struct pcb_t * proc, // Process executing the instruction
		BYTE data, // Data to be wrttien into memory
		uint32_t destination, // Index of destination register
		uint32_t offset) { 	// Destination address =
					// [destination] + [offset]
	return write_mem(proc->regs[destination] + offset, proc, data);
} 

int run(struct pcb_t * proc) {
	/* Check if Program Counter point to the proper instruction */
	if (proc->pc >= proc->code->size) {
		return 1;
	}
	
	struct inst_t ins = proc->code->text[proc->pc];
	proc->pc++;
	int stat = 1;
	switch (ins.opcode) {
	case CALC:
		printf("Calc \n");
		stat = calc(proc);
		break;
	case ALLOC:

		printf("Proc %d: alloc %d %d\n",proc->pid,ins.arg_0, ins.arg_1);
#ifdef MM_PAGING
		stat = pgalloc(proc, ins.arg_0, ins.arg_1);
		print_pgtbl(proc, 0, -1);
#else
		stat = alloc(proc, ins.arg_0, ins.arg_1);
#endif
		
		break;
#ifdef MM_PAGING
	case MALLOC:
		printf("Malloc %d %d \n",ins.arg_0, ins.arg_1);
		stat = pgmalloc(proc, ins.arg_0, ins.arg_1);
		print_pgtbl(proc, 0, -1);
		break;
#endif
	case FREE:
#ifdef MM_PAGING
		printf("Free %d: %d\n",proc->pid,ins.arg_0);
		stat = pgfree_data(proc, ins.arg_0);
#else
		stat = free_data(proc, ins.arg_0);
#endif
		break;
	case READ:
#ifdef MM_PAGING
		printf("Read %d: %d %d %d\n",proc->pid,ins.arg_0, ins.arg_1,ins.arg_2);
		stat = pgread(proc, ins.arg_0, ins.arg_1, ins.arg_2);
#else
		stat = read(proc, ins.arg_0, ins.arg_1, ins.arg_2);
#endif
		break;
	case WRITE:
#ifdef MM_PAGING
		stat = pgwrite(proc, ins.arg_0, ins.arg_1, ins.arg_2);
#else
		stat = write(proc, ins.arg_0, ins.arg_1, ins.arg_2);
#endif
		break;
	default:
		stat = 1;
	}
	return stat;

}


