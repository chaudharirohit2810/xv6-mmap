# mmap in xv6
This project implements ```mmap()``` and ```munmap()``` system calls in xv6. 
mmap and munmap system calls provide extensive control over the process address space. According to linux manual, 
creates a new mapping in the virtual address space of the calling process. 
It can be used to use to **map file into memory, share physical memory between the processes or to allocate large chunks of memory (like malloc)**.

## Implementations:
- Pagecache implementation: 
	- Implemented pagecache on top of buffer cache.
	- Used to retrieve & store contents of file in physical memory to reduce the overall read time 
	- synchronized the pagecache & handled coherency
- Lazy mappings: 
	- Page is mapped only when it is needed, this helps in optimal utilization of physical memory 
- Private memory mapping (MAP_PRIVATE):
	- Creates in private mapping
	- Updates to mapping are not visible to child processes
- Shared memory mapping (MAP_SHARED):
  	- Mapping is shared between multiple processes
	- Updated to mapping are visible to child processes sharing the same mapping 
- File Backed memory mapping: 
	- Mapping is backed by file, pagecache is used for this
- Anonymous memory mapping (MAP_ANONYMOUS): 
	- Mapping is not backed by file and contents are initialized to 0
- Fixed address memory mapping (MAP_FIXED): 
	- Mapping is mapped at exactly the address provided in the arguement
	- If the mapping is not possible returns error 


