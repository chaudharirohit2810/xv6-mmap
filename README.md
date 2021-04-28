# mmap in xv6

## Features:
- Pagecache implementation: 
	- Implemented pagecache on top of buffer cache. 
	- synchronized the pagecache & handled coherency
- Lazy mappings: 
	- Page is mapped only when it is needed, this helps in optimal utilization of physical memory 
- Private memory mapping (MAP_PRIVATE):
	- Creates in private mapping
	- Updates to mapping are not visible to child processes
- Shared memory mapping (MAP_SHARED):
  - Mapping is shared between multiple processes
	- Updated to mapping are visible to child processes sharing the same mapping 
- File Backed memory mapping: Mapping is backed by file
- Anonymous memory mapping (MAP_ANONYMOUS): Mapping is not backed by file and contents are initialized to 0
- Fixed address memory mapping (MAP_FIXED): Mapping is mapped at exactly the address provided in the arguement 


