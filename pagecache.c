#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "pagecache.h"

// Page cache structure
struct {
  struct cached_page pages[NPAGECACHE];
  int totalCount;
  struct spinlock lock;
} pagecache;

// To intialize the page cache
void pagecacheinit(void) {
  initlock(&pagecache.lock, "pagecache");
  for (int i = 0; i < NPAGECACHE; i++) {
    pagecache.pages[i].page = kalloc();
    if (!pagecache.pages[i].page) {
      cprintf("pagecacheinit: Kalloc failed");
      return;
    }
    initsleeplock(&pagecache.pages[i].lock, "pagecache page");
    memset(pagecache.pages[i].page, 0, PGSIZE);
  }
  pagecache.totalCount = 0;
  cprintf("Initialized the page cache\n");
}

// To find page in page cache make sure you are holding lock on pagecache
static struct cached_page *findPage(uint inum, int offset, int dev) {
  if (!holding(&pagecache.lock))
    panic("Pagecache: find page");
  for (int i = 0; i < NPAGECACHE; i++) {
    if (pagecache.pages[i].inode_number == inum &&
        pagecache.pages[i].offset == offset && pagecache.pages[i].dev == dev) {
      return &pagecache.pages[i];
    }
  }
  return 0;
}

// Update page in pagecache on file write
void updatePage(int offset, int inum, int dev, char *addr, int size) {
  // Acquire lock on pagecache
 	acquire(&pagecache.lock);
  int alligned_offset = offset - offset % PGSIZE;
  int start_addr = offset % PGSIZE;
  struct cached_page *res = findPage(inum, alligned_offset, dev);
  // Release lock on pagecache page
  release(&pagecache.lock);
  if (!res) {
    return;
  }
  // Acquire lock on pagecache page
  acquiresleep(&res->lock);
  char *page = res->page;
  memmove(page + start_addr, addr, size);
  releasesleep(&res->lock);
  // Release lock on pagecache
  return;
}

// Check if page is present in page cache if yes return it or if absent allocate
// a new one
struct cached_page *getPage(struct inode *ip, int offset, int inum, int dev) {
  offset -= offset % PGSIZE;
	acquire(&pagecache.lock);
  // Find the page in page cache
  struct cached_page *res = findPage(inum, offset, dev);
  if (res) {
    acquiresleep(&res->lock);
    release(&pagecache.lock);
    return res;
  }
  // If the page is not present then use one which has refcount as 0
  struct cached_page *allocpagecache = &pagecache.pages[pagecache.totalCount++];
  release(&pagecache.lock);
  acquiresleep(&allocpagecache->lock);
  if (pagecache.totalCount == NPAGECACHE) {
    pagecache.totalCount = 0;
  }
  // Read from the disk into page cache
  memset(allocpagecache->page, 0, PGSIZE);
  int n = readi(ip, allocpagecache->page, offset, PGSIZE);
  if (n == -1) {
    return (struct cached_page *)-1;
  }
  // Set the inode number & offset and increment the reference count
  allocpagecache->dev = dev;
  allocpagecache->inode_number = inum;
  allocpagecache->offset = offset;
  return allocpagecache;
}

// Copy the content from pagecache to destination
int copyPage(struct inode *ip, int offset, int inum, int dev, char *dest,
             int size, int dest_offset) {
  struct cached_page *page = getPage(ip, offset, inum, dev);
  // allocation of page from page cache failed
  if (page == (struct cached_page *)-1) {
    return 0;
  }
  if (!holdingsleep(&page->lock))
    panic("pagecache copy page");
  // Copy the content from page cache to allocated page
  memmove(dest, page->page + dest_offset, size);
  // Release the cached page sleep lock
  releasesleep(&page->lock);
  return 0;
}
