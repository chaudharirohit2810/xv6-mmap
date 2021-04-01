#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "defs.h"
#include "buf.h"
#include "memlayout.h"

void* my_mmap(int addr, struct file* f, int size, int offset, int flags, int protection) {
	cprintf("this is mmap system call\n");
	return (void*)0;
}

