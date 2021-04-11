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

// Copy the mmap regions
// first arguement: dest
// second arguement: src
void copy_mmap_struct(struct mmap_region *mr1, struct mmap_region *mr2) {
  mr1->virt_addr = mr2->virt_addr;
  mr1->size = mr2->size;
  mr1->flags = mr2->flags;
  mr1->protection = mr2->protection;
  mr1->f = mr2->f;
  mr1->offset = mr2->offset;
  mr1->isshared = mr2->isshared;
}

void printMaps(struct proc *p) {
  int i = 0;
  cprintf("\n\n-------------All the mappings------------\n");
  while (i < 30 && p->mmaps[i].virt_addr) {
    cprintf("Virtual address: %p\tSize: %d\n", p->mmaps[i].virt_addr, p->mmaps[i].size);
    i += 1;
  }
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
  if (i >= 29) {
    cprintf("Mmap region count exceeded\n");
    return -1;
  }
  // Find total number of mappings
  int length = 0;
  while (p->mmaps[length].virt_addr) {
    length += 1;
  }
  // Right shift all mappings
  int j = length;
  while (j > i + 1) {
    copy_mmap_struct(&p->mmaps[j], &p->mmaps[j - 1]);
    j--;
  }
  uint mmapaddr = PGROUNDUP(p->mmaps[i].virt_addr + p->mmaps[i].size);
  // If the mmapaddr is greater than KERNBASE
  if (PGROUNDUP(mmapaddr + size) >= KERNBASE) {
    cprintf("Address exceeds KERNBASE\n");
    return -1;
  }
  // Store the virtual_address in mapping
  p->mmaps[i + 1].virt_addr = mmapaddr;
  p->mmaps[i + 1].size = size;
  cprintf("Mmap address: %p Index: %d\n", p->mmaps[i + 1].virt_addr, i + 1);

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
  int tempsize = size;
  int i = 0;

  // copy the file content from page cache to allocated memory
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

  struct proc *p = myproc();
  // Map the page to user process
  if (mappages(p->pgdir, (void *)mmapaddr, PGSIZE, V2P(temp), PTE_U | protection) < 0) {
    return -1;
  }

  return size;
}

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

// Main function of mmap system call
void *my_mmap(int addr, struct file *f, int size, int offset, int flags, int protection) {
  struct proc *p = myproc(); // Current running process
  int i = findMmapAddr(p, size);

  if (i == -1) {
    return (void *)-1;
  }
  uint mmapaddr = p->mmaps[i].virt_addr;

  // If the structure for memory mapping not available
  if (i == 30) {
    return (void *)-1;
  }

  if (!(flags & MAP_ANONYMOUS)) { // File backed mapping
    // Private file-backed Mapping
    if ((flags & MAP_PRIVATE)) {
      if (mapPrivateMain(f, mmapaddr, protection, offset, size) == -1) {
        return (void *)-1;
      }
    } else { // Shared file-backed mapping
      cprintf("Shared file mapping not done!!\n");
      return (void *)-1;
    }
  } else { // Anonymous mapping
    if (mapAnonPage(p, mmapaddr, protection, size) < 0) {
      return (void *)-1;
    }
  }

  // Store info in process
  p->mmaps[i].flags = flags;
  p->mmaps[i].protection = protection;
  p->mmaps[i].offset = offset;
  p->mmaps[i].f = f;

  return (void *)mmapaddr;
}

// Zero the whole mmap region structure
void zero_mmap_region_struct(struct mmap_region *mr) {
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
  struct proc *p = myproc();
  pte_t *pte;
  addr = PGROUNDUP(addr);
  int i = 0;
  int total_size = 0;

  for (; i < 30; i++) {
    if (p->mmaps[i].virt_addr == addr) {
      cprintf("unmapping: %p\n", addr);
      total_size = p->mmaps[i].size;
      break;
    }
  }
  // Page with given address does not exist
  if (i == 30 || total_size == 0) {
    cprintf("unmapPage Error: Addr not present in mappings\n\n");
    return -1;
  }

  // Free the allocated page
  int currsize = 0;
  for (; currsize < total_size; currsize += PGSIZE) {
    uint tempaddr = addr + currsize;
    pte = walkpgdir(p->pgdir, (char *)tempaddr, 0);
    if (!pte) {
      cprintf("unmapPage Error: Pte does not exist\n\n");
      return -1;
    }
    uint pa = PTE_ADDR(*pte);
    if (pa == 0) {
      cprintf("unmapPage Error: Physical page not stored at pte\n\n");
      return -1;
    }
    char *v = P2V(pa);
    kfree(v);
    *pte = 0;
  }

  while (i < 30 && p->mmaps[i + 1].virt_addr) {
    copy_mmap_struct(&p->mmaps[i], &p->mmaps[i + 1]);
    i += 1;
  }
  zero_mmap_region_struct(&p->mmaps[i]);

  return 0;
}
