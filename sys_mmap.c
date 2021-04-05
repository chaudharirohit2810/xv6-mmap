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
#include "mmap.h"

// Function to map the private page to process
int mapPrivatePage(char* page, struct proc* p, uint mmapaddr, int protection) {
	char* temp = kalloc(); // Allocate a temporary page
	memmove(temp, page, PGSIZE); // Copy the content from page cache to allocated page	
	// Map the page to user process
	if(mappages(p->pgdir, (void*)mmapaddr, PGSIZE, V2P(temp), PTE_U|protection) < 0) {
			return -1;
	}
	return PGSIZE;
}

// Function to map anonymous private page
int mapAnonPage(struct proc* p, uint mmapaddr, int protection, int size) {
		int i = 0;
		for(;i < size; i += PGSIZE) {
			char* mapped_page = kalloc();
			memset(mapped_page, 0, PGSIZE);
			if(mappages(p->pgdir, (void*)mmapaddr + i, PGSIZE, V2P(mapped_page), PTE_U|protection) < 0) {
				cprintf("Map Anon: mappages failed\n");
				return -1;
			}
		}
		return size;
}

// Main function of mmap system call
void* my_mmap(int addr, struct file* f, int size, int offset, int flags, int protection) {	
	uint mmapaddr = MMAPBASE + offset * 2;
	mmapaddr = PGROUNDUP(mmapaddr);
	struct proc* p = myproc(); // Current running process

	if(!(flags & MAP_ANONYMOUS)) { // File backed mapping
		// Get the page from page cache
		char* page = getPage(f->ip, offset, f->ip->inum); 
		if(page == (char*)-1) { // allocation of page from page cache failed
			return (void*)-1;
		}
	
		if((flags & MAP_PRIVATE)) {	// Private file-backed Mapping	
			if(mapPrivatePage(page, p, mmapaddr, protection) == -1) {
				return (void*)-1;
			}
		}else { // Shared file-backed mapping
			cprintf("Shared file mapping not done!!\n");
			return (void*)-1;
		}
	}else { // Anonymous mapping	
		if(mapAnonPage(p, mmapaddr, protection, size) < 0) {
				return (void*)-1;
		}
	}

	return (void*)mmapaddr; 
}

// Main function of munmap system call
int my_munmap(int addr, int size) {
	cprintf("This is munmap system call\n");
	return 0;
}
