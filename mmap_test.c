#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "mmap.h"
char *filename = "PASSWD";

// File backend mappings test (18 tests)
void file_private_test();                              // File backed private mapping
void file_shared_test();                               // File backed shared mapping
void file_invalid_fd_test();                           // Invalid fds provided to file backed mapping
void file_invalid_flags_test();                        // Invalid flags provided to file backend mapping
void file_writeable_shared_mapping_on_ro_file_test();  // writeable shared mapping on file opened in read only mode
void file_ro_shared_mapping_on_ro_file_test();         // read only shared mapping on file opened in read only mode
void file_exceed_size_test();                          // File backed mapping size exceeds KERNBASE
void file_exceed_count_test();                         // File backend mapping count exceeds mmap array limit
void file_private_mapping_perm_test();                 // Mapping permissions test on private file backed mapping
void file_pagecache_coherency_test();                  // Check if pagecache is updated after file write
void file_private_with_fork_test();                    // Private file mapping with fork test
void file_shared_with_fork_test();                     // Shared file mapping with fork test
void file_mapping_with_offset_test();                  // Private file mapping with some offset test
void file_given_addr_test();                           // File backed Mapping with valid user provided address test
void file_invalid_addr_test();                         // File backed Mapping with invalid user provided address test
void file_overlap_given_addr_test();                   // File backed Mapping with user provided address overlapping with existing mapping test
void file_intermediate_given_addr_test();              // File backed Mapping with user provided address can be fit between two existing mappings test
void file_intermediate_given_addr_not_possible_test(); // File backed Mapping with user provided address that cannot be fit between two existing mappings test

// Anonymous tests (14 tests)
void anon_private_test();                              // private anonymous mapping test
void anon_shared_test();                               // Shared anonymous mapping test
void anon_private_fork_test();                         // Private anonymous mapping with fork test
void anon_shared_multi_fork_test();                    // Shared mapping with multiple forks test
void anon_exceed_size_test();                          // Mapping exceeds KERNBASE due to size test
void anon_exceed_count_test();                         // Mapping count exceeds 30 (mmap array limit) test
void anon_private_shared_fork_test();                  // Private & Shared anonymous mapping together with fork test
void anon_missing_flags_test();                        // Invalid flags to mmap test
void anon_given_addr_test();                           // Anonymous Mapping with valid user provided address test
void anon_invalid_addr_test();                         // Anonymous Mapping with invalid user provided address test
void anon_overlap_given_addr_test();                   // Anonymous Mapping with user provided address overlapping with existing mapping test
void anon_intermediate_given_addr_test();              // Anonymous Mapping with user provided address can be fit between two existing mappings test
void anon_intermediate_given_addr_not_possible_test(); // Anonymous Mapping with user provided address that cannot be fit between two existing mappings test
void anon_write_on_ro_mapping_test();                  // Trying to write read only mapping test

// Other Mmap tests
void munmap_partial_size_test();      // When user unmaps the mapping partially
void mmap_write_on_ro_mapping_test(); // Write test on read only mapping
void mmap_none_permission_test();     // None permission on mapping test
void mmap_valid_map_fixed_test();     // MAP_FIXED flag testing with valid address
void mmap_invalid_map_fixed_test();   // MAP_FIXED flag testing with invalid addresses

void file_tests() {
  file_invalid_fd_test();
  file_invalid_flags_test();
  file_writeable_shared_mapping_on_ro_file_test();
  file_ro_shared_mapping_on_ro_file_test();
  file_private_mapping_perm_test();
  file_exceed_size_test();
  file_exceed_count_test();
  file_private_test();
  file_shared_test();
  file_pagecache_coherency_test();
  file_private_with_fork_test();
  file_shared_with_fork_test();
  file_mapping_with_offset_test();
  file_given_addr_test();
  file_invalid_addr_test();
  file_overlap_given_addr_test();
  file_intermediate_given_addr_test();
  file_intermediate_given_addr_not_possible_test();
}

void anonymous_tests(void) {
  anon_private_test();
  anon_shared_test();
  anon_private_fork_test();
  anon_shared_multi_fork_test();
  anon_private_shared_fork_test();
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
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 1;
    }
  }
  return 0;
}

int main(int args, char *argv[]) {
  file_tests();
  anonymous_tests();
  munmap_partial_size_test();
  mmap_write_on_ro_mapping_test();
  mmap_none_permission_test();
  mmap_valid_map_fixed_test();
  mmap_invalid_map_fixed_test();
  exit();
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
void file_invalid_flags_test() {
  printf(1, "file backed mapping invalid flags test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed mapping invalid flags test failed: at open\n");
    exit();
  }
  int size = 100;
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, 0, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed mapping invalid flags test ok\n");
    close(fd);
    return;
  }
  printf(1, "file backed mapping invalid flags test failed\n");
  exit();
}

// When file has only read only permission but mapping is shared with Write permission
void file_writeable_shared_mapping_on_ro_file_test() {
  printf(1, "file backed writeable shared mapping on read only file test\n");
  int fd = open(filename, O_RDONLY);
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
  int fd = open(filename, O_RDONLY);
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
  int fd = open(filename, O_RDONLY);
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
void file_exceed_size_test() {
  printf(1, "file backed exceed mapping size test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed exceed mapping size test failed: at open\n");
    exit();
  }
  int size = 600 * 1024 * 1024; // 600 MB
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, fd, 0);
  if (ret != (void *)-1) {
    printf(1, "file backed exceed mapping size test failed\n");
    munmap((void *)ret, size);
    exit();
  }
  close(fd);
  printf(1, "file backed exceed mapping size test ok\n");
}

// file backed mapping count test when it exceeds 30
void file_exceed_count_test() {
  printf(1, "file backed exceed mapping count test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed exceed mapping count test failed\n");
    exit();
  }
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
    close(fd);
  } else {
    printf(1, "file backed exceed mapping count test failed: %d total mappings\n", i);
    exit();
  }
}

// Simple private file backed mapping test
void file_private_test() {
  printf(1, "file backed private mapping test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed private mapping test failed: at open\n");
    exit();
  }
  int size = 1000;
  char buf[1000];
  int n = read(fd, buf, size);
  if (n != size) {
    printf(1, "file backed private mapping test failed: at read\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed private mapping test failed\n");
    exit();
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed private mapping test failed\n");
    exit();
  }
  for (int i = 0; i < 40; i++) {
    ret[i] = 'p';
    buf[i] = 'p';
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed private mapping test failed\n");
    exit();
  }
  int res = munmap((void *)ret, size);
  if (res == -1) {
    printf(1, "file backed private mapping test failed\n");
    exit();
  }
  int fd2 = open(filename, O_RDONLY);
  char buf2[1000];
  // Read from the file again and check if it is not equal to mapping data as mapping is private
  n = read(fd2, buf2, size);
  if (n != size) {
    printf(1, "file backed private mapping test failed: at read\n");
    exit();
  }
  if (my_strcmp(buf, buf2, size) == 0) {
    printf(1, "file backed private mapping test failed\n");
    exit();
  }
  close(fd2);
  close(fd);
  printf(1, "file backed private mapping test ok\n");
}

// Simple private file backed mapping test
void file_mapping_with_offset_test() {
  printf(1, "file backed mapping with offset test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed mapping with offset test failed: at open\n");
    exit();
  }
  int size = 1000;
  char buf[1000];
  int n = read(fd, buf, 200); // Move to offset 200
  if (n != 200) {
    printf(1, "file backed mapping with offset test failed: at read\n");
    exit();
  }
  if (read(fd, buf, size) != size) {
    printf(1, "file backed mapping with offset test failed: at read\n");
    exit();
  }
  // Offset is 200
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 200);
  if (ret == (void *)-1) {
    printf(1, "file backed mapping with offset test failed\n");
    exit();
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed mapping with offset test failed\n");
    exit();
  }
  for (int i = 0; i < 40; i++) {
    ret[i] = 'p';
    buf[i] = 'p';
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed mapping with offset test failed\n");
    exit();
  }
  int res = munmap((void *)ret, size);
  if (res == -1) {
    printf(1, "file backed mapping with offset test failed\n");
    exit();
  }
  close(fd);
  printf(1, "file backed mapping with offset test ok\n");
}

// Shared file backed mapping test
void file_shared_test() {
  printf(1, "file backed shared mapping test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed shared mapping test failed: at open\n");
    exit();
  }
  int size = 1000;
  char buf[1000];
  int n = read(fd, buf, size);
  if (n != size) {
    printf(1, "file backed shared mapping test failed: at read\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed shared mapping test failed\n");
    exit();
  }
  // Check if both entries have same data
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed shared mapping test failed\n");
    exit();
  }
  for (int i = 0; i < 40; i++) {
    ret[i] = 'a';
    buf[i] = 'a';
  }
  // Check if both mappings are same after edit
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed shared mapping test failed\n");
    exit();
  }
  int res = munmap((void *)ret, size);
  if (res == -1) {
    printf(1, "file backed shared mapping test failed\n");
    exit();
  }
  int fd2 = open(filename, O_RDONLY);
  char buf2[1000];
  // Read from the file again and check if it is equal to mapping data
  n = read(fd2, buf2, size);
  if (n != size) {
    printf(1, "file backed shared mapping test failed: at read\n");
    exit();
  }
  if (my_strcmp(buf, buf2, size) != 0) {
    printf(1, "file backed shared mapping test failed\n");
    exit();
  }
  close(fd2);
  close(fd);
  printf(1, "file backed shared mapping test ok\n");
}

// Check if pagecache is updated after write to file
void file_pagecache_coherency_test() {
  printf(1, "file backed mapping pagecache coherency test\n");
  int size = 100;
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed mapping pagecache coherency test failed: at open\n");
    exit();
  }
  char buf[100];
  int n = read(fd, buf, size);
  if (n != size) {
    printf(1, "file backed mapping pagecache coherency test failed: at read\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed mapping pagecache coherency test failed\n");
    exit();
  }
  if (my_strcmp(buf, ret, size) != 0) {
    printf(1, "file backed mapping pagecache coherency test failed\n");
    exit();
  }
  char a[100];
  for (int i = 0; i < 100; i++)
    a[i] = 'a';
  // Write some data to file at offset equal to size
  n = write(fd, a, size);
  if (n != size) {
    printf(1, "file backed mapping pagecache coherency test failed: at filewrite\n");
    exit();
  }
  close(fd);
  int fd2 = open(filename, O_RDWR); // Again open file just to seek to start
  if (fd2 == -1) {
    printf(1, "file backed mapping pagecache coherency test failed: at open\n");
    exit();
  }
  n = read(fd2, buf, size); // For the lseek
  if (n != size) {
    printf(1, "file backed mapping pagecache coherency test failed: at read\n");
    exit();
  }
  n = read(fd2, buf, size); // Read from the file at offset size
  if (n != size) {
    printf(1, "file backed mapping pagecache coherency test failed: at read\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd2, 100);
  if (ret2 == (void *)-1) {
    printf(1, "file backed mapping pagecache coherency test failed: at mmap\n");
    exit();
  }
  if (my_strcmp(ret2, buf, size) != 0) {
    printf(1, "file backed mapping pagecache coherency test failed\n");
    exit();
  }
  int res = munmap(ret, size);
  if (res == -1) {
    printf(1, "file backed mapping pagecache coherency test failed\n");
    exit();
  }
  res = munmap(ret2, size);
  if (res == -1) {
    printf(1, "file backed mapping pagecache coherency test failed\n");
    exit();
  }
  printf(1, "file backed mapping pagecache coherency test ok\n");
  close(fd2);
}

// private file backed mapping with fork test
void file_private_with_fork_test() {
  printf(1, "file backed private mapping with fork test\n");
  int size = 200;
  char buf[200];
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed private mapping with fork test failed\n");
    exit();
  }
  if (read(fd, buf, size) != size) {
    printf(1, "file backed private mapping with fork test failed\n");
    exit();
  }
  char *ret = mmap((void *)0, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < 50; i++) {
      ret[i] = 'n';
    }
    // The mapping should not be same as we have edited the data
    if (my_strcmp(ret, buf, size) == 0) {
      printf(1, "file backed private mapping with fork test failed\n");
      exit();
    }
    exit();
  } else {
    wait();
    // As it is private mapping therefore it should be same as read
    if (my_strcmp(ret, buf, size) != 0) {
      printf(1, "file backed private mapping with fork test failed\n");
      exit();
    }
    int res = munmap(ret, size);
    if (res == -1) {
      printf(1, "file backed private mapping with fork test failed\n");
      exit();
    }
    printf(1, "file backed private mapping with fork test ok\n");
  }
}

// shared file backed mapping with fork test
void file_shared_with_fork_test() {
  printf(1, "file backed shared mapping with fork test\n");
  int size = 200;
  char buf[200];
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed shared apping with fork test failed\n");
    exit();
  }
  if (read(fd, buf, size) != size) {
    printf(1, "file backed shared mapping with fork test failed\n");
    exit();
  }
  char *ret2 = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  int pid = fork();
  if (pid == 0) {
    for (int i = 0; i < 50; i++) {
      ret2[i] = 'o';
    }
    if (my_strcmp(ret2, buf, size) == 0) {
      printf(1, "file backed shared mapping with fork test failed\n");
      exit();
    }
    exit();
  } else {
    wait();
    // The data written in child process should persist here
    if (my_strcmp(ret2, buf, size) == 0) {
      printf(1, "file backed shared mapping with fork test failed\n");
      exit();
    }
    int res = munmap(ret2, size);
    if (res == -1) {
      printf(1, "file backed shared mapping with fork test failed\n");
      exit();
    }
    close(fd);
    // Check if data is written into file
    int fd2 = open(filename, O_RDWR);
    for (int i = 0; i < 50; i++) {
      buf[i] = 'o';
    }
    char buf2[200];
    if (read(fd2, buf2, size) != size) {
      printf(1, "file backed shared mapping with fork test failed\n");
      exit();
    }
    if (my_strcmp(buf2, buf, size) != 0) {
      printf(1, "file backed shared mapping with fork test failed\n");
      exit();
    }
    printf(1, "file backed shared mapping with fork test ok\n");
    close(fd2);
  }
}

// file backed mmap when the valid address is provided by user
void file_given_addr_test() {
  printf(1, "file backed valid provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed shared apping with fork test failed\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0x60001000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed valid provided address test failed: at mmap\n");
    exit();
  }
  int res = munmap(ret, 200);
  if (res == -1) {
    printf(1, "file backed valid provided address test failed: at munmap\n");
    exit();
  }
  close(fd);
  printf(1, "file backed valid provided address test ok\n");
}

// file backed mmap when provided address is less than MMAPBASE
void file_invalid_addr_test(void) {
  printf(1, "file backed invalid provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed invalid provided address test failed\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0x50001000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret != (void *)-1) {
    printf(1, "file backed invalid provided address test failed\n");
    munmap(ret, 200);
    exit();
  }
  printf(1, "file backed invalid provided address test ok\n");
  close(fd);
}

// test when the address is provided by user and it overlaps with existing address
void file_overlap_given_addr_test() {
  printf(1, "file backed overlapping provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed overlapping provided address test failed\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0x60001000, 10000, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed overlapping provided address test failed: at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0x60001000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret2 == (void *)-1 || ret2 == (void *)0x60001000) {
    printf(1, "file backed overlapping provided address test failed: at second mmap\n");
    exit();
  }
  int res = munmap(ret, 10000);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at first munmap\n");
    exit();
  }
  res = munmap(ret2, 200);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at second munmap\n");
    exit();
  }
  printf(1, "file backed overlapping provided address test ok\n");
  close(fd);
}

// test when the file backed mapping is possible between two mappings
void file_intermediate_given_addr_test() {
  printf(1, "file backed intermediate provided address test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed intermediate provided address test failed\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0, 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed intermediate provided address test failed: failed at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0x60003000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret2 == (void *)-1) {
    printf(1, "file backed intermediate provided address test failed: failed at second mmap\n");
    munmap(ret, 1000);
    exit();
  }
  char *ret3 = (char *)mmap((void *)0x60000100, 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret3 != (void *)0x60001000) {
    printf(1, "file backed intermediate provided address test failed: failed at third mmap\n");
    munmap(ret, 1000);
    munmap(ret2, 200);
    exit();
  }
  int res = munmap(ret, 1000);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at first munmap\n");
    munmap(ret2, 200);
    munmap(ret3, 1000);
    exit();
  }
  res = munmap(ret2, 200);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at second munmap\n");
    munmap(ret3, 1000);
    exit();
  }
  res = munmap(ret3, 1000);
  if (res == -1) {
    printf(1, "file backed overlapping provided address test failed: at third munmap\n");
    exit();
  }
  close(fd);
  printf(1, "file backed intermediate provided address test ok\n");
}

// mmap when the mapping is not possible between two mappings
void file_intermediate_given_addr_not_possible_test() {
  printf(1, "file backed intermediate provided address not possible test\n");
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    printf(1, "file backed intermediate provided address not possible test\n");
    exit();
  }
  char *ret = (char *)mmap((void *)0, 1000, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret == (void *)-1) {
    printf(1, "file backed intermediate provided address not possible test failed: failed at first mmap\n");
    exit();
  }
  char *ret2 = (char *)mmap((void *)0x60003000, 200, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret2 == (void *)-1) {
    printf(1, "file backed intermediate provided address not possible test failed: failed at second mmap\n");
    exit();
  }
  char *ret3 = (char *)mmap((void *)0x60000100, 10000, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (ret3 == (void *)0x60000100) {
    printf(1, "file backed intermediate provided address not possible test failed: failed at third mmap\n");
    exit();
  }
  munmap(ret, 1000);
  munmap(ret2, 200);
  munmap(ret3, 10000);
  close(fd);
  printf(1, "file backed intermediate provided address not possible test ok\n");
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

// <!! -------------------------------------------------------------- Other MMAP Tests ---------------------------------------- !!>

// When there is only read permission on mapping but user tries to write
void mmap_write_on_ro_mapping_test() {
  printf(1, "write on read only mapping test\n");
  int pid = fork();
  if (pid == -1) {
    printf(1, "write on read only mapping failed in fork\n");
    exit();
  }
  if (pid == 0) {
    int size = 10000;
    int *ret = (int *)mmap((void *)0, size, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret == (void *)-1) {
      printf(1, "write on read only mapping test failed\n");
      exit();
    }
    for (int i = 0; i < size / 4; i++) {
      ret[i] = i;
    }
    // If the memory access allowed then test should failed
    printf(1, "write on read only mapping test failed\n");
    exit();
  } else {
    wait();
    printf(1, "write on read only mapping test ok\n");
  }
}

// Munmap only partial size of the mapping test
void munmap_partial_size_test() {
  printf(1, "munmap only partial size test\n");
  int size = 10000;
  int *ret = (int *)mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "munmap only partial size test failed\n");
    exit();
  }
  for (int i = 0; i < size / 4; i++) {
    ret[i] = i;
  }
  // Munmap only first page
  int res = munmap((void *)ret, 5);
  if (res == -1) {
    printf(1, "munmap only partial size test failed\n");
    exit();
  }
  // Check if next page is still accessible or not
  for (int i = 1024; i < size / 4; i++) {
    ret[i] = i * 2;
  }
  res = munmap((void *)ret, size);
  if (res == -1) {
    printf(1, "munmap only partial size test failed\n");
    exit();
  }
  printf(1, "munmap only partial size test ok\n");
}

// None permission on mapping test
void mmap_none_permission_test() {
  printf(1, "none permission on mapping test\n");
  int pid = fork();
  if (pid == -1) {
    printf(1, "none permission on mapping test failed\n");
    exit();
  }
  if (pid == 0) {
    int size = 10000;
    char *ret = (char *)mmap((void *)0x70003000, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (ret == (void *)-1) {
      printf(1, "none permission on mapping test failed\n");
      exit();
    }
    printf(1, "%s\n", ret);
    for (int i = 0; i < size / 4; i++) {
      ret[i] = i;
    }
    // If the memory access allowed then test should failed
    printf(1, "none permission on mapping test failed\n");
    exit();
  } else {
    wait();
    printf(1, "none permission on mapping test ok\n");
  }
}

// To test MAP_FIXED flag with valid address
void mmap_valid_map_fixed_test() {
  printf(1, "mmap valid address map fixed flag test\n");
  char *ret = mmap((void *)0x60001000, 200, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret == (void *)-1) {
    printf(1, "mmap valid address map fixed flag test failed\n");
    exit();
  }
  int res = munmap(ret, 200);
  if (res == -1) {
    printf(1, "mmap valid address map fixed flag test failed\n");
    exit();
  }
  printf(1, "mmap valid address map fixed flag test ok\n");
}

// To test MAP_FIXED flag with invalid addresses
void mmap_invalid_map_fixed_test() {
  printf(1, "mmap invalid address map fixed flag test\n");
  // When the address is less than MMAPBASE
  char *ret = mmap((void *)0x50001000, 200, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret != (void *)-1) {
    printf(1, "mmap invalid address map fixed flag test failed\n");
    exit();
  }
  // When the address is not page aligned
  char *ret2 = mmap((void *)0x60000100, 200, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret2 != (void *)-1) {
    printf(1, "mmap invalid address map fixed flag test failed\n");
    exit();
  }
  // Mapping is not possible because other mapping already exists at provided address
  char *ret3 = mmap((void *)0x60000000, 200, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret3 == (void *)-1) {
    printf(1, "mmap invalid address map fixed flag test failed\n");
    exit();
  }
  char *ret4 = mmap((void *)0x60000000, 200, PROT_WRITE | PROT_READ, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret4 != (void *)-1) {
    printf(1, "mmap invalid address map fixed flag test failed\n");
    exit();
  }
  munmap(ret3, 200);
  printf(1, "mmap invalid address map fixed flag test ok\n");
}
