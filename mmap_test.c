#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"

// File backend mappings test
void file_invalid_fd_test();                          // Invalid fds provided to file backed mapping
void file_invalid_flags_test();                       // Invalid flags provided to file backend mapping
void file_writeable_shared_mapping_on_ro_file_test(); // writeable shared mapping on file opened in read only mode
void file_ro_shared_mapping_on_ro_file_test();        // read only shared mapping on file opened in read only mode
void file_exceed_size_test();                         // File backed mapping size exceeds KERNBASE
void file_exceed_count_test();                        // File backend mapping count exceeds mmap array limit
void file_private_mapping_perm_test();                // Mapping permissions test on private file backed mapping

// Anonymous tests (14 tests)
void anon_private_test();                              // private anonymous mapping test
void anon_shared_test();                               // Shared anonymous mapping test
void anon_private_fork_test();                         // Private anonymous mapping with fork test
void anon_shared_multi_fork_test();                    // Shared mapping with multiple forks test
void anon_exceed_size_test();                          // Mapping exceeds KERNBASE due to size test
void anon_exceed_count_test();                         // Mapping count exceeds 30 (mmap array limit) test
void anon_private_shared_fork_test();                  // Private & Shared anonymous mapping together with fork test
void anon_missing_flags_test();                        // Invalid flags to mmap test
void anon_given_addr_test();                           // Mapping with valid user provided address test
void anon_invalid_addr_test();                         // Mapping with invalid user provided address test
void anon_overlap_given_addr_test();                   // Mapping with user provided address overlapping with existing mapping test
void anon_intermediate_given_addr_test();              // Mapping with user provided address can be fit between two existing mappings test
void anon_intermediate_given_addr_not_possible_test(); // Mapping with user provided address that cannot be fit between two existing mappings test
void anon_write_on_ro_mapping_test();                  // Trying to write read only mapping test

// Mmap tests
void mmapMultiTest(int); // Check multiple private maps and munmaps
void mmapWriteFileTest(int);
void exceedCountMmapTest(int);              // Check when mmap region count exceeds
void mmapPrivateFileMappingForkTest(int);   // Test for private file mapping with fork
void mmapSharedFileMappingForkTest(int);    // Test for shared file mapping with fork
void mmapSharedWritableMappingTest(int fd); // the mapping is shared and there is write permission on it but file is opened as read only
void anon_private_test_when_munmap_size_is_not_total();

void file_tests(int fd) {
  file_invalid_fd_test();
  file_invalid_flags_test();
  file_writeable_shared_mapping_on_ro_file_test();
  file_ro_shared_mapping_on_ro_file_test();
  file_private_mapping_perm_test();
  file_exceed_size_test();
  file_exceed_count_test();
}

void anonymous_tests(void) {
  anon_private_test();
  anon_shared_test();
  anon_private_fork_test();
  anon_shared_multi_fork_test();
  anon_private_shared_fork_test();
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

// Utility strcmp function
int my_strcmp(const char *a, const char *b, int n) {
  while (n > 0 && *a && *b) {
    n--, a++, b++;
  }
  if (n == 0) {
    return 0;
  }
  return 1;
}

int main(int args, char *argv[]) {
  int fd = open("PASSWD", O_RDWR);
  if (fd == -1) {
    printf(1, "File does not exist\n");
    exit();
  }
  // file_tests(fd);
  // anonymous_tests();
  anon_private_test_when_munmap_size_is_not_total();
  exit();
}

// Simple private anonymous mapping test with maping having both read and write permission and size greater than two pages
void anon_private_test_when_munmap_size_is_not_total() {
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
  int res = munmap((void *)ret, 5);
  res = munmap((void *)ret, 5);
  for (int i = 2048; i < size / 4; i++) {
    printf(1, "%d\n", ret[i]);
  }
  if (res == -1) {
    printf(1, "anonymous private mapping test failed\n");
    exit();
  }
  printf(1, "anonymous private mapping test ok\n");
}

// <!!-------------------------------------------------- File Backed mapping -------------------------------------------------- !!>
// When invalid fd is provided to mapping
void file_invalid_fd_test() {
  printf(1, "file backed mapping invalid file descriptor test\n");
  int size = 100;
  int fds[3] = {-1, 10, 18}; // Negative fd, fd in range but does not exist, fd out of range
  for (int i = 0; i < 3; i++) {
    char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fds[i], 0);
    if (ret == (void *)-1) {
      continue;
    }
    printf(1, "file backed mapping invalid file descriptor test failed\n");
    exit();
  }
  printf(1, "file backed mapping invalid file descriptor test ok\n");
}

// When invalid flags are provided to mapping
void file_invalid_flags_test(int fd) {
  printf(1, "file backed mapping invalid flags test\n");
  int size = 100;
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, 0, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed mapping invalid flags test ok\n");
    return;
  }
  printf(1, "file backed mapping invalid flags test failed\n");
  exit();
}

// When file has only read only permission but mapping is shared with Write permission
void file_writeable_shared_mapping_on_ro_file_test() {
  printf(1, "file backed writeable shared mapping on read only file test\n");
  int fd = open("README", O_RDONLY);
  if (fd == -1) {
    printf(1, "file backed writeable shared mapping on read only file test failed: at open\n");
    exit();
  }
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed writeable shared mapping on read only file test ok\n");
    close(fd);
    return;
  }
  printf(1, "file backed writeable shared mapping on read only file test failed\n");
  exit();
}

// When file has only read only permission but mapping is shared with only read permission
void file_ro_shared_mapping_on_ro_file_test() {
  printf(1, "file backed read only shared mapping on read only file test\n");
  int fd = open("README", O_RDONLY);
  if (fd == -1) {
    printf(1, "file backed read only shared mapping on read only file test failed: at open\n");
    exit();
  }
  char *ret = mmap((void *)0, 200, PROT_READ, MAP_SHARED, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed read only shared mapping on read only file test failed\n");
    exit();
  }
  printf(1, "file backed read only shared mapping on read only file test ok\n");
  int res = munmap(ret, 200);
  if (res == -1) {
    printf(1, "file backed read only shared mapping on read only file test failed\n");
    exit();
  }
  close(fd);
}

// Mapping permissions test on private file backed mapping
void file_private_mapping_perm_test() {
  printf(1, "file backed private mapping permission test\n");
  int fd = open("README", O_RDONLY);
  if (fd == -1) {
    printf(1, "file backed private mapping permission test failed\n");
    exit();
  }
  // read only private mapping on read only opened file
  char *ret = mmap((void *)0, 200, PROT_READ, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed private mapping permission test failed\n");
    exit();
  }
  // write & read private mapping on read only opened file
  char *ret2 = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret2 == (void *)-1) {
    printf(1, "file backed private mapping permission test failed\n");
    exit();
  }
  for (int i = 0; i < 20; i++) {
    ret2[i] = 'a';
  }
  int res = munmap(ret, 200);
  if (res == -1) {
    printf(1, "file backed private mapping permission test failed\n");
    exit();
  }
  res = munmap(ret2, 200);
  if (res == -1) {
    printf(1, "file backed private mapping permission test failed\n");
    exit();
  }
  printf(1, "file backed private mapping permission test ok\n");
  close(fd);
}

// file backend mapping when size exceeds KERNBASE
void file_exceed_size_test(int fd) {
  printf(1, "file backed exceed mapping size test\n");
  int size = 600 * 1024 * 1024; // 600 MB
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, fd, 0);
  if (ret != (void *)-1) {
    printf(1, "file backed exceed mapping size test failed\n");
    munmap((void *)ret, size);
    exit();
  }
  printf(1, "file backed exceed mapping size test ok\n");
}

// file backed mapping count test when it exceeds 30
void file_exceed_count_test(int fd) {
  printf(1, "file backed exceed mapping count test\n");
  int size = 5096;
  int count = 50;
  uint arr[50];
  int i = 0;
  for (; i < count; i++) {
    void *ret = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, fd, 0);
    arr[i] = (uint)ret;
    if (ret == (void *)-1) {
      break;
    }
  }
  if (i == 30) {
    for (int j = 0; j < i; j++) {
      int ret = munmap((void *)arr[j], size);
      if (ret == -1) {
        printf(1, "file backed exceed mapping count test failed: at %d munmap\n", j);
        exit();
      }
    }
    printf(1, "file backed exceed mapping count test ok\n");
  } else {
    printf(1, "file backed exceed mapping count test failed: %d total mappings\n", i);
    exit();
  }
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

// <!! ------------------------------------------ Anonymous mappings test -------------------------------------------------- !!>

// Missing flags Test: Missing MAP_PRIVATE or MAP_SHARED in flags
void anon_missing_flags_test(void) {
  printf(1, "anonymous missing flags test\n");
  int size = 10000;
  // Missing MAP_PRIVATE or MAP_SHARED flag
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
void anon_private_test() {
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
void anon_shared_test() {
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

// Shared mapping test with multiple forks
void anon_shared_multi_fork_test() {
  printf(1, "anonymous shared mapping with multiple forks test\n");
  int size = 1000;
  char *ret = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Shared mapping
  if (ret == (void *)-1) {
    printf(1, "anonymous shared mapping with multiple forks test failed\n");
    exit();
  }
  char data[1000];
  for (int i = 0; i < size; i++) {
    data[i] = 'r';
  }
  int pid = fork();
  if (pid == -1) {
    printf(1, "anonymous shared mapping with multiple forks test failed: at fork\n");
    exit();
  }
  if (pid == 0) { // 1st fork Child Process
    for (int i = 0; i < size; i++) {
      ret[i] = 'r';
    }
    int pid2 = fork();
    if (pid2 == -1) {
      printf(1, "anonymous shared mapping with multiple forks test failed: at fork\n");
      exit();
    }
    if (pid2 == 0) { // 2nd fork Child Process
      if (my_strcmp(data, ret, size) != 0) {
        printf(1, "anonymous shared mapping with multiple forks test failed\n");
        exit();
      }
      int pid3 = fork();
      if (pid3 == -1) {
        printf(1, "anonymous shared mapping with multiple forks test failed: at fork\n");
        exit();
      }
      if (pid3 == 0) { // 3rd fork Child Process
        if (my_strcmp(data, ret, size) != 0) {
          printf(1, "anonymous shared mapping with multiple forks test failed\n");
        }
        exit();
      } else { // 3rd fork Parent Process
        wait();
        if (my_strcmp(data, ret, size) != 0) {
          printf(1, "anonymous shared mapping with multiple forks test failed\n");
        }
        exit();
      }
    } else { // 2nd fork Parent Process
      wait();
      if (my_strcmp(data, ret, size) != 0) {
        printf(1, "anonymous shared mapping with multiple forks test failed\n");
      }
      exit();
    }
    exit();
  } else { // 1st fork Parent process
    wait();
    if (my_strcmp(data, ret, size) != 0) {
      printf(1, "anonymous shared mapping with multiple forks test failed\n");
      exit();
    }
    int res = munmap((void *)ret, size);
    if (res == -1) {
      printf(1, "anonymous shared mapping with multiple forks test failed\n");
      exit();
    }
    printf(1, "anonymous shared mapping with multiple forks test ok\n");
  }
}

// Private mapping with fork
void anon_private_fork_test() {
  printf(1, "anonymous private mapping with fork test\n");
  char temp[200];
  for (int i = 0; i < 200; i++) {
    temp[i] = 'a';
  }
  int size = 200;
  char *ret = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // Shared mapping
  if (ret == (void *)-1) {
    printf(1, "anonymous private mapping with fork test failed\n");
    exit();
  }
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < size; i++) {
      ret[i] = 'a';
    }
    exit();
  } else {
    wait();
    if (my_strcmp(temp, ret, size) == 0) {
      printf(1, "anonymous private mapping with fork test failed\n");
      exit();
    }
    printf(1, "anonymous private mapping with fork test ok\n");
    munmap(ret, size);
  }
}

// When there is only read permission on mapping but user tries to write
void anon_write_on_ro_mapping_test() {
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
void anon_private_shared_fork_test() {
  printf(1, "anonymous private & shared mapping together with fork test\n");
  int size = 200;
  char data1[200], data2[200];
  for (int i = 0; i < size; i++) {
    data1[i] = 'a';
    data2[i] = 'r';
  }
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0); // Private mapping
  if (ret == (void *)-1) {
    printf(1, "anonymous private & shared mapping together with fork test failed\n");
    exit();
  }
  char *ret2 = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Shared mapping
  if (ret2 == (void *)-1) {
    printf(1, "anonymous private & shared mapping together with fork test failed\n");
    exit();
  }
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < size; i++) {
      ret[i] = 'a';
    }
    for (int i = 0; i < size; i++) {
      ret2[i] = 'r';
    }
    exit();
  } else {
    wait();
    // Private mapping
    if (my_strcmp(ret, data1, size) == 0) {
      printf(1, "anonymous private & shared mapping together with fork test failed\n");
      exit();
    }
    // Shared mapping
    if (my_strcmp(ret2, data2, size) != 0) {
      printf(1, "anonymous private & shared mapping together with fork test failed\n");
      exit();
    }
    int res = munmap(ret, size);
    if (res == -1) {
      printf(1, "anonymous private & shared mapping together with fork test failed\n");
      exit();
    }
    res = munmap(ret2, size);
    if (res == -1) {
      printf(1, "anonymous private & shared mapping together with fork test failed\n");
      exit();
    }
    printf(1, "anonymous private & shared mapping together with fork test ok\n");
  }
}

// mmap when the valid address is provided by user
void anon_given_addr_test() {
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
void anon_overlap_given_addr_test() {
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
void anon_intermediate_given_addr_test() {
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
void anon_intermediate_given_addr_not_possible_test() {
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
