#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

// Anonymous tests
void anon_private_test(void);       // Check private anonymous mappings
void anon_exceed_size_test(void);   // Check when the mmap size exceeds KERNBASE
void anon_exceed_count_test(void);  // when mmap count exceeds 30 (mmap array limit)
void anon_fork_test(void);          // Test for fork with anonymous mapping
void anon_missing_flags_test(void); // Missing flags test

// Mmap tests
void mmapMultiTest(int);                    // Check multiple private maps and munmaps
void exceedCountMmapTest(int);              // Check when mmap region count exceeds
void mmapPrivateFileMappingForkTest(int);   // Test for private file mapping with fork
void mmapSharedFileMappingForkTest(int);    // Test for shared file mapping with fork
void mmapSharedWritableMappingTest(int fd); // the mapping is shared and there is write permission on it but file is opened as read only

int main(int args, char *argv[]) {
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    printf(1, "File does not exist\n");
    exit();
  }
  //  mmapMultiTest(fd);
  //  anon_private_test();
  //  anon_missing_flags_test();
  // anon_exceed_count_test();
  //  mmapPrivateFileMappingForkTest(fd);
  //  mmapSharedFileMappingForkTest(fd);
  //  mmapSharedWritableMappingTest(fd);
  anon_fork_test();
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

// <!! ------------------------------------------ Anonymous mappings test -------------------------------------------------- !!>

// Missing flags Test
void anon_missing_flags_test(void) {
  int size = 10000;
  int *ret = (int *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
  int res = munmap((void *)ret, size);
  printf(1, "munmap return value: %d\n", res);
}

// anonymous mapping exceeds KERNBASE
void anon_exceed_size_test(void) {
  int size = 50960000;
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  printf(1, "Return address: %p\n", ret);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
  munmap((void *)ret, size);
}

// anonymous mapping count test when it exceeds 30
void anon_exceed_count_test(void) {
  int size = 5096;
  for (int i = 0; i < 50; i++) {
    void *ret = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret == (void *)-1) {
      printf(1, "Mmap failed! iteration: %d\n", i);
      return;
    }
  }
}

// Private anonymous mapping
void anon_private_test(void) {
  int size = 10000;
  int *ret = (int *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "Mmap failed!!\n");
    exit();
  }
  for (int i = 0; i < size / 4; i++) {
    ret[i] = i;
  }
  for (int i = 0; i < size / 4; i++) {
    printf(1, "%d\t", ret[i]);
  }
  printf(1, "\n");
  int res = munmap((void *)ret, size);
  printf(1, "munmap return value: %d\n", res);
}

// fork syscall with anonymous mapping test
void anon_fork_test(void) {
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);  // Private mapping
  int *ret2 = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 300); // Shared mapping
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
