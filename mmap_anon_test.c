#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

int main(int args, char *argv[]) {
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
  int res = munmap((void *)ret, size);
  printf(1, "munmap return value: %d\n", res);
  //	printf(1, "%s\n", ret);
  exit();
}
