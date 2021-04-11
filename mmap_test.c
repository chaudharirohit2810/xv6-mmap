#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

// Mmap tests
void mmapMultiTest(int);       // Check multiple private maps and munmaps
void anomMmapTest();           // Check private anonymous mappings
void exceedSizeMmapTest();     // Check when the mmap size exceeds KERNBASE
void exceedCountMmapTest(int); // Check when mmap region count exceeds

int main(int args, char *argv[]) {
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    printf(1, "File does not exist\n");
    exit();
  }
  //	mmapMultiTest(fd);
  anomMmapTest();
  exit();
}

// ----------------------------------- Test to check multiple private maps and munmaps ---------------------------------------------
void mmapMultiTest(int fd) {
  int size = 5096;
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }

  char *ret2 = (char *)mmap((void *)0, size * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
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
  munmap((void *)ret2, size * 2);

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
}

// ------------------------------------------- Test to check anonymous mmap -----------------------------------------------------
void anomMmapTest(void) {
  int size = 5000;
  int *ret = (int *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
  for (int i = 0; i < size / sizeof(int); i++) {
    ret[i] = i;
  }
  for (int i = 0; i < size / sizeof(int); i++) {
    printf(1, "%d\t", ret[i]);
  }
  printf(1, "\n");
  int res = munmap((void *)ret, size);
  printf(1, "munmap return value: %d\n", res);
}

// ---------------------------------------------- Exceed size private mmap test ----------------------------------------------
void exceedSizeMmapTest(void) {
  int size = 50960000;
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  printf(1, "Return address: %p\n", ret);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
  munmap((void *)ret, size);
}

// ----------------------------------------------- Exceed mmap region count test -----------------------------------------------
void exceedCountMmapTest(int fd) {
  int size = 5096;
  for (int i = 0; i < 31; i++) {
    char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
    if (ret == (void *)-1) {
      printf(1, "Mmap failed!!\n");
      exit();
    }
  }
}
