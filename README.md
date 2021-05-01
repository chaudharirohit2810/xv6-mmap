# mmap in xv6

mmap and munmap provided extensive control over the process address space. 
According to linux manual, mmap() creates a new mapping in the virtual address space of the calling process. 
The starting address for the new mapping is specified in addr and the length argument specifies the length of the mapping

## Features:

- Pagecache implementation:
  - Implemented pagecache on top of buffer cache.
  - synchronized the pagecache & handled coherency
- Lazy mappings:
  - Page is mapped only when it is needed, this helps in optimal utilization of physical memory
- Private memory mapping (MAP_PRIVATE):
  - Creates a private mapping
  - Updates to mapping are not visible to child processes
- Shared memory mapping (MAP_SHARED): 
	- Mapping is shared between multiple processes
  - Updated to mapping are visible to child processes sharing the same mapping
- File Backed memory mapping:
  - Mapping is backed by file
	- Pagecache is used to store the file contents which helps in effective retrieval of file data
- Anonymous memory mapping (MAP_ANONYMOUS):
  - Mapping is not backed by file and contents are initialized to 0
- Fixed address memory mapping (MAP_FIXED):
  - Mapping is mapped at exactly the address provided in the arguement
  - If the mapping is not possible returns error
