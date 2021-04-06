#include "pagecache.h"
#include "types.h"
#include "defs.h"
#include "mmu.h"

// Page cache structure
// TODO: Later think of something like hash map and linked list 
struct pagecache pages[NPAGECACHE];

// To intialize the page cache
void 
pagecacheinit(void) {
		for(int i = 0; i < NPAGECACHE; i++) {
				pages[i].page = kalloc();
				memset(pages[i].page, 0, PGSIZE);
		}
		cprintf("Initialized the page cache\n");
}

// To find page in page cache
static struct pagecache* findPage(uint inum, int offset) {
		for(int i = 0; i < NPAGECACHE; i++) {
				if(pages[i].inode_number == inum && pages[i].offset == offset) {
						cprintf("found returning: %p\n", pages[i].page);
						return &pages[i];
				}
		}
		return 0;
}


// Check if page is present in page cache if yes return it or if absent allocate a new one
char* getPage(struct inode* ip, int offset, int inum) {
	int i = 0;	
	offset -= offset % PGSIZE;

	// Find the page in page cache
	struct pagecache* res = findPage(inum, offset);
	if(res) {
			return res->page;
	}

	// If the page is not present then use one which has refcount as 0
	cprintf("not present, use one from page cache\n");
	for(i = 0; i < NPAGECACHE; i++) {
			if(pages[i].refCount == 0) {
					break;
			}
	}	
	// free page not available in cache
	if(i == NPAGECACHE) {
			cprintf("getPage Error: No page available\n");
			return (char*)-1;
	}
	
	// Read from the disk into page cache	
	memset(pages[i].page, 0, PGSIZE);
  int n = readi(ip, pages[i].page, offset, PGSIZE); 		
	if(n == -1) {
			cprintf("getPage Error: Read from disk failed\n");
			return (char*)-1;
	}
	// Set the inode number & offset and increment the reference count	
	pages[i].inode_number = inum;
	pages[i].offset = offset;
	pages[i].refCount++;	

	return pages[i].page;
}

// Decrease the reference count when the page is unmapped
int freePage(int inum, int offset) {
	cprintf("Decreasing the reference count\n");
	offset -= offset % PGSIZE;
	struct pagecache* res = findPage(inum, offset);
	if(!res) {
		return -1;
	}
	res->refCount -= 1;
	return 0;
}
