#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "defs.h"
#include "memlayout.h"


// Main mmap system call
void* my_mmap(int addr, struct file* f, int size, int offset, int flags, int protection) {	
	uint tempaddr = 0x60000000 + offset * 2;
	tempaddr = PGROUNDUP(tempaddr);
	struct proc* p = myproc(); // Current running process
	// Get the page from page cache
	char* page = getPage(f->ip, offset, f->ip->inum); 
	if(page == (char*)-1) { // Check if getPage failed
			return (void*)-1;
	}
	char* temp = kalloc(); // Allocate a temporary page
	memmove(temp, page, PGSIZE); // Copy the content from page cache to allocated page	

	// Map the page to user process
	if(mappages(p->pgdir, (void*)tempaddr, 4096, V2P(temp), PTE_P|PTE_U) < 0) {
			return (void*)-1;
	}

	return (void*)tempaddr; 
}

int my_munmap(int addr, int size) {
	cprintf("This is munmap system call\n");
	return 0;
}
