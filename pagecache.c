#include "pagecache.h"
#include "types.h"
#include "defs.h"
#include "mmu.h"

// Page cache structure
// TODO: Later think of something like hash map and linked list
struct pagecache pages[NPAGECACHE];
int totalCount = 0;

// To intialize the page cache
void pagecacheinit(void) {
  for (int i = 0; i < NPAGECACHE; i++) {
    pages[i].page = kalloc();
    if (!pages[i].page) {
      cprintf("pagecacheinit: Kalloc failed");
      return;
    }
    memset(pages[i].page, 0, PGSIZE);
  }
  cprintf("Initialized the page cache\n");
}

// To find page in page cache
static struct pagecache *findPage(uint inum, int offset, int dev) {
  for (int i = 0; i < NPAGECACHE; i++) {
    if (pages[i].inode_number == inum && pages[i].offset == offset && pages[i].dev == dev) {
      //      cprintf("found returning: %p\n", pages[i].page);
      return &pages[i];
    }
  }
  return 0;
}

void updatePage(int offset, int inum, int dev, char* addr, int size) {
	int alligned_offset = offset - offset % PGSIZE;
	int start_addr = offset % PGSIZE;
  struct pagecache* res = findPage(inum, alligned_offset, dev);
	if(!res) {
		 return;
	}
	char* page = res->page;
	memmove(page + start_addr, addr, size);
	return;
}

// Check if page is present in page cache if yes return it or if absent allocate a new one
char *getPage(struct inode *ip, int offset, int inum, int dev) {
  offset -= offset % PGSIZE;
  // Find the page in page cache
  struct pagecache *res = findPage(inum, offset, dev);
  if (res) {
    return res->page;
  }
  // If the page is not present then use one which has refcount as 0
  //  cprintf("not present, use one from page cache\n");
  struct pagecache *allocpagecache = &pages[totalCount++];
  if (totalCount == NPAGECACHE) {
    totalCount = 0;
  }
  // Read from the disk into page cache
  memset(allocpagecache->page, 0, PGSIZE);
  int n = readi(ip, allocpagecache->page, offset, PGSIZE);
  if (n == -1) {
    cprintf("getPage Error: Read from disk failed\n");
    return (char *)-1;
  }
  // Set the inode number & offset and increment the reference count
  allocpagecache->dev = dev;
  allocpagecache->inode_number = inum;
  allocpagecache->offset = offset;
  return allocpagecache->page;
}
