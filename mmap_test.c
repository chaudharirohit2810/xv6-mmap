#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

// Anonymous tests
void anon_private_test(void);                       // Check private anonymous mappings
void anon_shared_test(void);                               // Shared anonymous mapping test
void anon_exceed_size_test(void);                          // Check when the mmap size exceeds KERNBASE
void anon_exceed_count_test(void);                         // when mmap count exceeds 30 (mmap array limit)
void anon_fork_test(void);                                 // Test for fork with anonymous mapping
void anon_missing_flags_test(void);                        // Missing flags test
void anon_given_addr_test(void);                           // Test when explicit address is provided
void anon_invalid_addr_test(void);                         // When the address provided by user is less than MMAPBASE
void anon_overlap_given_addr_test(void);                   // When the address is provided by user and it overlaps with existing mapping
void anon_intermediate_given_addr_test(void);              // When the provided address can be mapped between two already provided address
void anon_intermediate_given_addr_not_possible_test(void); // When the provided address is between two mappings but mapping is not possible due to size
void anon_write_on_ro_mapping_test(void);                  // When there is only read permission on mapping but user tries to write

// Mmap tests
void mmapMultiTest(int); // Check multiple private maps and munmaps
void mmapWriteFileTest(int);
void exceedCountMmapTest(int);              // Check when mmap region count exceeds
void mmapPrivateFileMappingForkTest(int);   // Test for private file mapping with fork
void mmapSharedFileMappingForkTest(int);    // Test for shared file mapping with fork
void mmapSharedWritableMappingTest(int fd); // the mapping is shared and there is write permission on it but file is opened as read only

void anonymous_tests(void) {
  anon_private_test();
  anon_shared_test();
  // anon_write_on_ro_mapping_test();
  anon_missing_flags_test();
  anon_exceed_count_test();
  anon_exceed_size_test();
  anon_given_addr_test();
  anon_invalid_addr_test();
  anon_overlap_given_addr_test();
  anon_intermediate_given_addr_test();
  anon_intermediate_given_addr_not_possible_test();
}

int main(int args, char *argv[]) {
  int fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    printf(1, "File does not exist\n");
    exit();
  }
  anonymous_tests();
  exit();
}

void mmapWriteFileTest(int fd) {
  int size = 100;
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "MMap failed\n");
    exit();
  }
  ret[99] = '\0';
  printf(1, "Data: %s\n", ret);
  char a[100];
  for (int i = 0; i < 100; i++)
    a[i] = 'a';
  char b[200];
  read(fd, b, 200); // Just to use as lseek
  // Write some data to file
  write(fd, a, 100);
  write(fd, a, 100);
  char *ret2 = (char *)mmap((void *)0, size * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
  if (ret2 == (void *)-1) {
    printf(1, "MMap failed\n");
    exit();
  }
  ret2[199] = '\0';
  printf(1, "Data: %s\n", ret2);
  munmap(ret, size);
  munmap(ret2, size * 2);
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
  ret = (char *)mmap((void *)0x70000000, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 100);
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

// Missing flags Test: Missing MAP_PRIVATE or MAP_SHARED in flags
void anon_missing_flags_test(void) {
  printf(1, "anonymous missing flags test\n");
  int size = 10000;
  int *ret = (int *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);
  if (ret != (void *)-1) {
    printf(1, "anonymous missing flags test failed\n");
    munmap((void *)ret, size);
    exit();
  }
  printf(1, "anonymous missing flags test ok\n");
}

// anonymous mapping when size exceeds KERNBASE
void anon_exceed_size_test(void) {
  printf(1, "anonymous exceed mapping size test\n");
  int size = 600 * 1024 * 1024; // 600 MB
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (ret != (void *)-1) {
    printf(1, "anonymous exceed mapping size test failed\n");
    munmap((void *)ret, size);
    exit();
  }
  printf(1, "anonymous exceed mapping size test ok\n");
}

// anonymous mapping count test when it exceeds 30
void anon_exceed_count_test(void) {
  printf(1, "anonymous exceed mapping count test\n");
  int size = 5096;
  int count = 50;
  uint arr[50];
  int i = 0;
  for (; i < count; i++) {
    void *ret = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    arr[i] = (uint)ret;
    if (ret == (void *)-1) {
      break;
    }
  }
  if (i == 30) {
    for (int j = 0; j < i; j++) {
      int ret = munmap((void *)arr[j], size);
      if (ret == -1) {
        printf(1, "anonymous exceed mapping count test failed: at %d munmap\n", j);
        exit();
      }
    }
    printf(1, "anonymous exceed mapping count test ok\n");
  } else {
    printf(1, "anonymous exceed mapping count test failed: %d total mappings\n", i);
    exit();
  }
}

// Simple private anonymous mapping test with maping having both read and write permission and size greater than two pages
void anon_private_test(void) {
  printf(1, "anonymous private mapping test\n");
  int size = 10000;
  int *ret = (int *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "anonymous private mapping test failed\n");
    exit();
  }
  for (int i = 0; i < size / 4; i++) {
    ret[i] = i;
  }
  int res = munmap((void *)ret, size);
  if (res == -1) {
    printf(1, "anonymous private mapping test failed\n");
    exit();
  }
  printf(1, "anonymous private mapping test ok\n");
}

// Shared mapping test
void anon_shared_test(void) {
  printf(1, "anonymous shared mapping test\n");
  int size = 10000;
  int *ret = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Shared mapping
  if (ret == (void *)-1) {
    printf(1, "anonymous shared mapping test failed\n");
    exit();
  }
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < size / 4; i++) {
      ret[i] = i;
    }
		exit();
  } else {
    wait();
    for (int i = 0; i < size / 4; i++) {
      if (ret[i] != i) {
        printf(1, "anonymous shared mapping test failed\n");
        exit();
      }
    }
    int res = munmap((void *)ret, size);
    if (res == -1) {
      printf(1, "anonymous shared mapping test failed\n");
      exit();
    }
    printf(1, "anonymous shared mapping test ok\n");
  }
}

// When there is only read permission on mapping but user tries to write
void anon_write_on_ro_mapping_test(void) {
  printf(1, "anonymous write on read only mapping test\n");
  int pid = fork();
  if (pid == -1) {
    printf(1, "anonymous write on read only mapping failed in fork\n");
  }
  if (pid == 0) {
    int size = 10000;
    int *ret = (int *)mmap((void *)0, size, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret == (void *)-1) {
      printf(1, "anonymous write on read only mapping test failed\n");
      exit();
    }
    for (int i = 0; i < size / 4; i++) {
      ret[i] = i;
    }
    int res = munmap((void *)ret, size);
    if (res == -1) {
      printf(1, "anonymous write on read only mapping test failed\n");
      exit();
    }
    printf(1, "anonymous simple private mapping test ok\n");
  } else {
    wait();
  }
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

// mmap when the valid address is provided by user
void anon_given_addr_test(void) {
  printf(1, "anonymous valid provided address test\n");
  char *ret = (char *)mmap((void *)0x60001000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "anonymous valid provided address test failed: at mmap\n");
    exit();
  }
  int res = munmap(ret, 200);
  if (res == -1) {
    printf(1, "anonymous valid provided address test failed: at munmap\n");
    exit();
  }
  printf(1, "anonymous valid provided address test ok\n");
}

// mmap when provided address is less than MMAPBASE
void anon_invalid_addr_test(void) {
  printf(1, "anonymous invalid provided address test\n");
  char *ret = (char *)mmap((void *)0x50001000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret != (void *)-1) {
    printf(1, "anonymous invalid provided address test failed\n");
    munmap(ret, 200);
    exit();
  }
  printf(1, "anonymous invalid provided address test ok\n");
}

// test when the address is provided by user and it overlaps with existing address
void anon_overlap_given_addr_test(void) {
  printf(1, "anonymous overlapping provided address test\n");
  char *ret = (char *)mmap((void *)0x60001000, 10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "anonymous overlapping provided address test failed: at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0x60001000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret2 == (void *)-1 || ret2 == (void *)0x60001000) {
    printf(1, "anonymous overlapping provided address test failed: at second mmap\n");
    exit();
  }
  int res = munmap(ret, 10000);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at first munmap\n");
    exit();
  }
  res = munmap(ret2, 200);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at first munmap\n");
    exit();
  }
  printf(1, "anonymous overlapping provided address test ok\n");
}

// test when the mapping is possible between two mappings
void anon_intermediate_given_addr_test(void) {
  printf(1, "anonymous intermediate provided address test\n");
  char *ret = (char *)mmap((void *)0, 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "anonymous intermediate provided address test failed: failed at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0x60003000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret2 == (void *)-1) {
    printf(1, "anonymous intermediate provided address test failed: failed at second mmap\n");
    munmap(ret, 1000);
    exit();
  }
  char *ret3 = (char *)mmap((void *)0x60000100, 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret3 != (void *)0x60001000) {
    printf(1, "anonymous intermediate provided address test failed: failed at third mmap\n");
    munmap(ret, 1000);
    munmap(ret2, 200);
    exit();
  }
  int res = munmap(ret, 1000);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at first munmap\n");
    munmap(ret2, 200);
    munmap(ret3, 1000);
    exit();
  }
  res = munmap(ret2, 200);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at second munmap\n");
    munmap(ret3, 1000);
    exit();
  }
  res = munmap(ret3, 1000);
  if (res == -1) {
    printf(1, "anonymous overlapping provided address test failed: at third munmap\n");
    exit();
  }
  printf(1, "anonymous intermediate provided address test ok\n");
}

// mmap when the mapping is not possible between two mappings
void anon_intermediate_given_addr_not_possible_test(void) {
  printf(1, "anonymous intermediate provided address not possible test\n");
  char *ret = (char *)mmap((void *)0, 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "anonymous intermediate provided address not possible test failed: failed at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0x60003000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret2 == (void *)-1) {
    printf(1, "anonymous intermediate provided address not possible test failed: failed at second mmap\n");
    exit();
  }
  char *ret3 = (char *)mmap((void *)0x60000100, 10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret3 == (void *)0x60000100) {
    printf(1, "anonymous intermediate provided address not possible test failed: failed at third mmap\n");
    exit();
  }
  munmap(ret, 1000);
  munmap(ret2, 200);
  munmap(ret3, 10000);
  printf(1, "anonymous intermediate provided address not possible test ok\n");
}
