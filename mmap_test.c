#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

int main(int args, char* argv[]) {
	int size = 1024;
	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		printf(1, "File does not exist\n");
		exit();
	}
	char* ret = (char*) mmap((void *)0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	printf(1, "Mmap return address: %p\n\n\n", ret);
	if(ret == (void*)-1) {
		printf(1, "Mmap failed!!\n");
		exit();
	}
	for(int i = 0; i < 10; i++) {
			ret[i] = 'a';
	}
	printf(1, "%s\n", ret);
	ret = (char*) mmap((void *)0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	printf(1, "Mmap return address: %p\n\n\n", ret);
	if(ret == (void*)-1) {
		printf(1, "Mmap failed!!\n");
		exit();
	}
	for(int i = 0; i < 10; i++) {
			ret[i] = 'a';
	}
	printf(1, "%s\n", ret);
	int res = munmap((void*)ret, size);
	printf(1, "Munmap return value: %d\n", res);
	exit();
}

