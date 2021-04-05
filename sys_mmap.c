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
	cprintf("Page Address: %p\n", temp);
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
	uint mmapaddr = PGROUNDUP(MMAPBASE);
	struct proc* p = myproc(); // Current running process

	// Find empty memory map region
	int i = 0;
	for(;i < 30; i++) {
			if(p->mmaps[i].virt_addr == 0) {
					break;
			}
	}
	// If the structure for memory mapping not available
	if(i == 30) {
			return (void*)-1;
	}
	cprintf("mymmap: Allocated %d th entry in array\n\n", i);

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
	
	// Store info in process
	p->mmaps[i].virt_addr = mmapaddr;
	p->mmaps[i].size = size;
	p->mmaps[i].flags = flags;
	p->mmaps[i].protection = protection;
	p->mmaps[i].offset = offset;
	p->mmaps[i].f = f;

	return (void*)mmapaddr; 
}

// Zero the whole mmap region structure
void zero_mmap_region_struct(struct mmap_region* mr) {	
	mr->virt_addr = 0;
	mr->size = 0;
	mr->flags = 0;
	mr->protection = 0;
	mr->f = 0;
	mr->offset = 0;
	mr->flags = 0;
	mr->isshared = -1;
}


// Main function of munmap system call
int my_munmap(uint addr, int size) {
	cprintf("This is munmap system call\n");
	struct proc* p = myproc();
	pte_t *pte;
	addr = PGROUNDUP(addr);	
	int i = 0;
	int total_size = 0;

	for(; i < 30; i++) {
		if(p->mmaps[i].virt_addr == addr) {
			cprintf("\n\n--------- Page Found for unmapping -------------\n\n");
			total_size = p->mmaps[i].size;
			zero_mmap_region_struct(&p->mmaps[i]);
			break;
		}
	}
	// Page with given address does not exist
  if(i == 30 || total_size == 0) {
		cprintf("unmapPage Error: Addr not present in mappings\n\n");
		return -1;
	}

	// Free the allocated page
	int currsize = 0;
	for(; currsize < total_size; currsize += PGSIZE) {
		uint tempaddr = addr + currsize;
		pte = walkpgdir(p->pgdir, (char*)tempaddr, 0); 	
		if(!pte) {
			cprintf("unmapPage Error: Pte does not exist\n\n");
			return -1;
		}
		uint pa = PTE_ADDR(*pte);
		if(pa == 0) {
			cprintf("unmapPage Error: Physical page not stored at pte\n\n");
			return -1;
		}
		char* v = P2V(pa);
		kfree(v);
		*pte = 0;
	}

	return 0;
}
