// Main mmap function
void *my_mmap(int addr, struct file *f, int size, int offset, int flags, int protection);

// Main munmap function
int my_munmap(int addr, int size);
