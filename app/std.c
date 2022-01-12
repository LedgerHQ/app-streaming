#include <stdio.h>

struct stat;

/* https://interrupt.memfault.com/blog/boostrapping-libc-with-newlib */

extern int _end;

void *_sbrk(int incr) {
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&_end;
  }
  prev_heap = heap;

  heap += incr;

  return prev_heap;
}

int _close(int file) {
  return -1;
}

int _fstat(int file, struct stat *st) {
  return 0;
}

int _isatty(int file) {
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  return 0;
}

__attribute__((noreturn)) void _exit(int status) {
    asm(
        "li t0, 5\n"
        "add a0, %0, 0\n"
        "ecall\n"
        :: "r"(status) : "t0", "a0"
        );
    while (1);
}

void _kill(int pid, int sig) {
  return;
}

int _getpid(void) {
  return -1;
}

int _write(int file, char * ptr, int len) {
  return -1;
}

int _read(int file, char * ptr, int len) {
  return -1;
}
