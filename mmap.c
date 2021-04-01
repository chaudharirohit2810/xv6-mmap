#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(int args, char* argv[]) {
	int size = 1024;
	char data[1024];
	int fd = open(argv[1], O_RDONLY);
	void* ret = mmap((void *)data, size, 2, 3, fd, 20);
//	printf(1, "Return value: %d\n", (int)ret);
	exit();
}

