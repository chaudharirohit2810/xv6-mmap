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

// ------------------------------------------------------- Mmap Utils -----------------------------------------------------------------

// Zero the whole mmap region structure
void zero_mmap_region_struct(struct mmap_region *mr) {
  mr->virt_addr = 0;
  mr->size = 0;
  mr->flags = 0;
  mr->protection = 0;
  mr->f = 0;
  mr->offset = 0;
  mr->flags = 0;
}

// Copy the mmap regions from src to dest
void copy_mmap_struct(struct mmap_region *mr1, struct mmap_region *mr2) {
  mr1->virt_addr = mr2->virt_addr;
  mr1->size = mr2->size;
  mr1->flags = mr2->flags;
  mr1->protection = mr2->protection;
  mr1->f = mr2->f;
  mr1->offset = mr2->offset;
}

// Print all the memory mappings
void printMaps(struct proc *p) {
  int i = 0;
  cprintf("Total maps: %d\n", p->total_mmaps);
  while (i < p->total_mmaps) {
    cprintf("Virtual address: %p\tSize: %d\tisShared: %d\n", p->mmaps[i].virt_addr, p->mmaps[i].size, p->mmaps[i].flags & MAP_SHARED);
    i += 1;
  }
}

// Get physical Address of page from virtual address of process
uint getPhysicalPage(struct proc *p, uint tempaddr, pte_t **pte) {
  *pte = walkpgdir(p->pgdir, (char *)tempaddr, 0);
  if (!*pte) {
    cprintf("unmapPage Error: Pte does not exist\n\n");
    return -1;
  }
  uint pa = PTE_ADDR(**pte);
  if (pa == 0) {
    cprintf("unmapPage Error: Physical page not stored at pte\n\n");
    return -1;
  }
  return pa;
}

// Copy mmaps from parent to child process
int copyMaps(struct proc *parent, struct proc *child) {
  pte_t *pte;
  int i = 0;
  while (i < parent->total_mmaps) {
    uint virt_addr = parent->mmaps[i].virt_addr;
    int protection = parent->mmaps[i].protection;
    int isshared = parent->mmaps[i].flags & MAP_SHARED;
    uint size = parent->mmaps[i].size;
    uint start = virt_addr;
    for (; start < virt_addr + size; start += PGSIZE) {
      uint pa = getPhysicalPage(parent, start, &pte);
      if (pa == 0) {
        cprintf("Error while copying memory mappings\n");
        return -1;
      }
      if (isshared) {
        char *parentmem = (char *)P2V(pa);
        if (mappages(child->pgdir, (void *)start, PGSIZE, V2P(parentmem), PTE_U | protection) < 0) {
          cprintf("CopyMaps: mappages failed\n");
        }
      } else {
        char *mem = kalloc();
				if(!mem) {
					 cprintf("CopyMaps: Kalloc failed\n");
					 return -1;
				}
        char *parentmem = (char *)P2V(pa);
        memmove(mem, parentmem, PGSIZE);
        if (mappages(child->pgdir, (void *)start, PGSIZE, V2P(mem), PTE_U | protection) < 0) {
          cprintf("CopyMaps: mappages failed\n");
          return -1;
        }
      }
      copy_mmap_struct(&child->mmaps[i], &parent->mmaps[i]);
    }
    i += 1;
  }
	child->total_mmaps = parent->total_mmaps;
  return 0;
}

// To find the mmap region virtual address
static int findMmapAddr(struct proc *p, int size) {
  // If any page is not mapped yet
  if (p->mmaps[0].virt_addr == 0) {
    if (PGROUNDUP(MMAPBASE + size) >= KERNBASE) {
      cprintf("Address exceeds KERNBASE\n");
      return -1;
    }
    //    cprintf("Mmap address: %p Index: %d\n", PGROUNDUP(MMAPBASE), 0);
    p->mmaps[0].virt_addr = PGROUNDUP(MMAPBASE);
    p->mmaps[0].size = size;
    return 0; // Return the page rounded address
  }

  // TODO: Check if page can be mapped between MMAPBASE and first mapping

  // Find the map address
  int i = 0;
  while (i < 30 && p->mmaps[i + 1].virt_addr) {
    int start_addr = PGROUNDUP(p->mmaps[i].virt_addr + p->mmaps[i].size);
    int end_addr = PGROUNDUP(p->mmaps[i + 1].virt_addr);
    if (end_addr - start_addr > size) {
      break;
    }
    i += 1;
  }
  // If i is 30 then no more mapping possible return -1
  if (i + 1 == 30) {
    cprintf("Mmap region count exceeded\n");
    return -1;
  }

  // Right shift all mappings
  int length = 0;
  while (p->mmaps[length].virt_addr) {
    length += 1;
  }
  int j = length;
  while (j > i + 1) {
    copy_mmap_struct(&p->mmaps[j], &p->mmaps[j - 1]);
    j--;
  }

  // Check if the mmapaddr is greater than KERNBASE
  uint mmapaddr = PGROUNDUP(p->mmaps[i].virt_addr + p->mmaps[i].size);
  if (PGROUNDUP(mmapaddr + size) >= KERNBASE) {
    cprintf("Address exceeds KERNBASE\n");
    return -1;
  }
  // Store the virtual_address in mapping
  p->mmaps[i + 1].virt_addr = mmapaddr;
  p->mmaps[i + 1].size = size;
  return i + 1; // Return the index of mmap mapping
}

// ------------------------ File Backed Private Mapping ------------------------------

// Function to map the private page to process
static int mapPrivatePage(struct file *f, uint mmapaddr, int protection, int offset, int size) {
  char *temp = kalloc(); // Allocate a temporary page
  if (!temp) {
    cprintf("mapPrivatePage Error: kalloc failed, free Memory not available\n");
    return -1;
  }
  memset(temp, 0, PGSIZE);
  // copy the file content from page cache to allocated memory
  int tempsize = size;
  int i = 0;
  while (tempsize != 0) {
    // Get the page from page cache
    char *page = getPage(f->ip, offset + PGSIZE * i, f->ip->inum, f->ip->dev);
    if (page == (char *)-1) { // allocation of page from page cache failed
      return -1;
    }
    int curroff = offset % PGSIZE;
    int currsize = PGSIZE - curroff > tempsize ? tempsize : PGSIZE - curroff;
    memmove(temp + size - tempsize, page + curroff, currsize); // Copy the content from page cache to allocated page
    tempsize -= currsize;
    offset = 0;
    i += 1;
  }
  // Map the page to user process
  struct proc *p = myproc();
  if (mappages(p->pgdir, (void *)mmapaddr, PGSIZE, V2P(temp), PTE_U | protection) < 0) {
    return -1;
  }
  return size;
}

// Function which does file backed memory mapping
static int mapPrivateMain(struct file *f, uint mmapaddr, int protection, int offset, int size) {
  int currsize = 0;
  int mainsize = size;
  for (; currsize < mainsize; currsize += PGSIZE) {
    int mapsize = PGSIZE > size ? size : PGSIZE;
    if (mapPrivatePage(f, mmapaddr + currsize, protection, offset + currsize, mapsize) < 0) {
      return -1;
    }
    size -= PGSIZE;
  }
  return size;
}

// <!!-------------------------------------------------- Anonymous Mapping ------------------------------------------------------ !!>
// Function to map anonymous private page
static int mapAnonPage(struct proc *p, uint mmapaddr, int protection, int size) {
  int i = 0;
  for (; i < size; i += PGSIZE) {
    char *mapped_page = kalloc();
    if (!mapped_page) {
      cprintf("Map Anon: Kalloc failed\n");
      return -1;
    }
    memset(mapped_page, 0, PGSIZE);
    if (mappages(p->pgdir, (void *)mmapaddr + i, PGSIZE, V2P(mapped_page), PTE_U | protection) < 0) {
      cprintf("Map Anon: mappages failed\n");
      deallocuvm(p->pgdir, mmapaddr + i - PGSIZE, mmapaddr);
      kfree(mapped_page);
      return -1;
    }
  }
  return size;
}

// <!! -------------------------------------------------- Main Functions ------------------------------------ !!>

// mmap system call main function
void *my_mmap(int addr, struct file *f, int size, int offset, int flags, int protection) {
  struct proc *p = myproc(); // Current running process
  int i = findMmapAddr(p, size);
  if (i == -1) {
    return (void *)-1;
  }
  uint mmapaddr = p->mmaps[i].virt_addr;
  if (!(flags & MAP_ANONYMOUS)) { // File backed mapping
		if((flags & MAP_SHARED) && (protection & PROT_WRITE) && !f->writable) {
			return (void*)-1;
		}
    if (mapPrivateMain(f, mmapaddr, protection, offset, size) == -1) {
      return (void *)-1;
    }
  } else { // Anonymous mapping
    if (mapAnonPage(p, mmapaddr, protection, size) < 0) {
      return (void *)-1;
    }
  }
  // Store mmap info in process's mmap array
  p->mmaps[i].flags = flags;
  p->mmaps[i].protection = protection;
  p->mmaps[i].offset = offset;
  p->mmaps[i].f = f;
  p->total_mmaps += 1;

  return (void *)mmapaddr;
}

// Main function of munmap system call
int my_munmap(uint addr, int size) {
  struct proc *p = myproc();
	printMaps(p);
  pte_t *pte;
  addr = PGROUNDUP(addr);
  int i = 0;
  int total_size = 0;
  // Find the mmap entry
  for (; i < 30; i++) {
    if (p->mmaps[i].virt_addr == addr) {
      cprintf("unmapping: %p\n", addr);
      total_size = p->mmaps[i].size;
      break;
    }
  }
  // Page with given address does not exist
  if (i == 30 || total_size == 0) {
    cprintf("unmapPage Error: Addr not present in mappings\n");
    return -1;
  }
  uint isanon = p->mmaps[i].flags & MAP_ANONYMOUS;
  uint isshared = p->mmaps[i].flags & MAP_SHARED;
  if (isshared && !isanon && (p->mmaps[i].protection & PROT_WRITE)) {
    // write into the file
		p->mmaps[i].f->off = p->mmaps[i].offset;
		if(filewrite(p->mmaps[i].f, (char*)p->mmaps[i].virt_addr, p->mmaps[i].size) < 0) {
			cprintf("unmapPage Error: File write failed\n");
			return -1;
		}
  }
  // Free the allocated page
  int currsize = 0;
  for (; currsize < total_size; currsize += PGSIZE) {
    uint tempaddr = addr + currsize;
    uint pa = getPhysicalPage(p, tempaddr, &pte);
    if (pa == 0) {
      cprintf("unmapPage Error: Corresponding physical address page missing\n");
      return -1;
    }
    char *v = P2V(pa);
    kfree(v);
    *pte = 0;
  }
  // Left shift the mmap array
  while (i < 30 && p->mmaps[i + 1].virt_addr) {
    copy_mmap_struct(&p->mmaps[i], &p->mmaps[i + 1]);
    i += 1;
  }
  zero_mmap_region_struct(&p->mmaps[i]);
  p->total_mmaps -= 1;
  return 0;
}
