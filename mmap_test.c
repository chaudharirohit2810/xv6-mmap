#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

// Mmap tests
void mmapMultiTest(int);                    // Check multiple private maps and munmaps
void anomMmapTest();                        // Check private anonymous mappings
void exceedSizeMmapTest();                  // Check when the mmap size exceeds KERNBASE
void exceedCountMmapTest(int);              // Check when mmap region count exceeds
void mmapPrivateFileMappingForkTest(int);   // Test for private file mapping with fork
void mmapSharedFileMappingForkTest(int);    // Test for shared file mapping with fork
void mmapAnonForkTest(void);                // Test for fork with anonymous mapping
void mmapSharedWritableMappingTest(int fd); // the mapping is shared and there is write permission on it but file is opened as read only

int main(int args, char *argv[]) {
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    printf(1, "File does not exist\n");
    exit();
  }
  //  mmapMultiTest(fd);
  //  anomMmapTest();
  //  mmapPrivateFileMappingForkTest(fd);
  //  mmapSharedFileMappingForkTest(fd);
  mmapSharedWritableMappingTest(fd);
  // mmapAnonForkTest();
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

// ------------------------------------------------- private file backed mapping with fork test -------------------------------------------------------
void mmapPrivateFileMappingForkTest(int fd) {
  printf(1, "Fork with private mapping test\n");
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  char *ret2 = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 300);
  int pid = fork();
  if (pid == 0) {
    printf(1, "\n\n-------------Child process-----------\n\n");
    printf(1, "Child Mapping 1:\n%s\n", ret);
    for (int i = 0; i < 50; i++) {
      ret2[i] = 'a';
    }
    printf(1, "Child Mapping 2:\n%s\n", ret2);
    sleep(3);
  } else {
    wait();
    printf(1, "\n\n------Parent process----------\n\n");
    printf(1, "Parent Mapping 1:\n%s\n", ret);
    printf(1, "Parent Mapping 2:\n%s\n", ret2);
  }
}

// ------------------------------------------------- shared file backed mapping with fork test -------------------------------------------------------
void mmapSharedFileMappingForkTest(int fd) {
  printf(1, "Fork with shared mapping test\n");
  char *ret2 = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 300);
  int pid = fork();
  if (pid == 0) {
    printf(1, "-------------Child process-----------\n");
    for (int i = 0; i < 50; i++) {
      ret2[i] = 'a';
    }
    printf(1, "Child Mapping 2:\n%s\n", ret2);
  } else {
    wait();
    printf(1, "\n\n------Parent process----------\n");
    printf(1, "Parent Mapping 2:\n%s\n", ret2);
    if (munmap(ret2, 200) < 0) {
      printf(1, "mmap failed\n");
    }
  }
}

// -------------------------------------------------- fork with mmap anonymous test ---------------------------------------------
void mmapAnonForkTest(void) {
  printf(1, "Fork with shared mapping test\n");
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  int *ret2 = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 300);
  int pid = fork();
  if (pid == 0) {
    printf(1, "-------------Child process-----------\n");
    for (int i = 0; i < 50; i++) {
      ret[i] = 'a';
    }
    printf(1, "Child Mapping 1:\n%s\n", ret);
    printf(1, "Child Mapping 2:\n");
    for (int i = 0; i < 50; i++) {
      ret2[i] = i;
    }
    for (int i = 0; i < 50; i++) {
      printf(1, "%d\t", ret2[i]);
    }
    printf(1, "\n");
  } else {
    wait();
    printf(1, "\n\n------Parent process----------\n");
    printf(1, "Parent Mapping 1:\n%s\n", ret);
    printf(1, "Parent Mapping 2:\n");
    for (int i = 0; i < 50; i++) {
      ret2[i] = i;
    }
    for (int i = 0; i < 50; i++) {
      printf(1, "%d\t", ret2[i]);
    }
    printf(1, "\n");
  }
}

// ---------------------------- the mapping is shared and there is write permission on it but file is opened as read only ------------------
void mmapSharedWritableMappingTest(int fd) {
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if ((void *)ret == (void *)-1) {
    printf(1, "Mmap failed\n");
    return;
  }
  for (int i = 0; i < 100; i++) {
    ret[i] = 'a';
  }
  munmap(ret, 200);
}
