#include "pagecache.h"
#include "types.h"
#include "defs.h"
#include "mmu.h"

struct pagecache pages[30];

// To intialize the page cache
void 
pagecacheinit(void) {
		for(int i = 0; i < 30; i++) {
				pages[i].page = kalloc();
				memset(pages[i].page, 0, PGSIZE);
		}
		cprintf("Initialized the page cache\n");
}

// To find page in page cache
static char* findPage(uint inum, int offset) {
		for(int i = 0; i < 30; i++) {
				if(pages[i].inode_number == inum && pages[i].offset == offset) {
						cprintf("\n\nPage found returning: %p\n\n", pages[i].page);
						return pages[i].page;
				}
		}
		return (char*)-1;
}


// Check if page is present in page cache if yes return it or if absent allocate a new one
char* getPage(struct inode* ip, int offset, int inum) {
	int i = 0;	
	offset -= offset % PGSIZE;

	// Find the page in page cache
	char* res = findPage(inum, offset);
	if(res != (char*)-1) {
			return res;
	}

	// If the page is not present then use one which has refcount as 0
	cprintf("\n\nPage not present, use one from page cache\n\n\n");
	for(i = 0; i < 30; i++) {
			if(pages[i].refCount == 0) {
					break;
			}
	}	
	// free page not available in cache
	if(i == 30) {
			cprintf("getPage Error: No page available\n");
			return (char*)-1;
	}
	
	// Read from the disk into page cache	
	memset(pages[i].page, 0, PGSIZE);
  int n = readi(ip, pages[i].page, offset, PGSIZE - 1); 		
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
