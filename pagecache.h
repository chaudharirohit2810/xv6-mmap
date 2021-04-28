// Number of elements in pagecache
#define NPAGECACHE 30

// Main pagecache structure
struct cached_page {
  int dev;
  int inode_number;
  int refCount;
  int offset;
  char *page;
	struct sleeplock lock;
};
