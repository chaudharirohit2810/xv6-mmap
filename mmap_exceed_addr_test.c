#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

int main(int args, char *argv[]) {
  int size = 50960000;
	for(int i = 0; i < 31; i++) {
  	char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
		printf(1, "Return address: %p %d\n", ret, ret);
  	if (ret == (void *)-1) {
    	printf(1, "Mmap failed!!\n");
    	exit();
  	}
	}
  exit();
}
