#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

int main(int args, char *argv[]) {
  int size = 5096;
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    printf(1, "File does not exist\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
//  printf(1, "%s\n", ret);
  char* ret2 = (char *)mmap((void *)0, size * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret2 == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
  for (int i = 0; i < 10; i++) {
    ret2[i] = 'a';
  }
	
	// 3rd mmap
  ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
	munmap((void*)ret2, size * 2);
	
  
	// 4th mmap
	ret = (char *)mmap((void *)0, size * 3, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }

	ret = (char *)mmap((void *)0, size * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }

  munmap((void *)ret, size);
  //	printf(1, "Munmap return value: %d\n", res);
  exit();
}
