#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/mman.h>

typedef void (*userfunc_t)(void);

static void usage(const char *program_name)
{
  puts("~~ asmloader -- loads and executes x86 machine code");
  printf("Usage: %s <binfile>\n", program_name);
  puts("<binfile> -- filename with raw compiled x86 machine code.");
}

static int get_file_size(FILE *f)
{
  int savepos = ftell(f);

  if (savepos == -1) {

    fprintf(stderr,
            "Cant get file size with error %i\n",
            errno
    );

    return -1;
  }

  int size = fseek(f, 0, SEEK_END);

  if (size == -1) {

    fprintf(stderr,
            "fseek failed with error %i. "
            "(However, just before ftell was successfull)\n",
            errno
    );

    return -1;
  }


  size = ftell(f);

  if (fseek(f, savepos, SEEK_SET) == -1) {

    fprintf(stderr,
            "fseek failed when was restoring previous position with error %i. "
            "(However, just before ftell was successfull)\n",
            errno
    );

    return -1;
  }

  return size;
}

inline static void __attribute__((always_inline))
store_regs()
{
  asm ( "push %%rax\n\t"
        "push %%rbx\n\t"
        "push %%rcx\n\t"
        "push %%rdx\n\t"
        "push %%rsi\n\t"
        "push %%rdi\n\t"
        "push %%rbp\n\t"
        "push %%rsp\n\t"
        "push %%r8\n\t"
        "push %%r9\n\t"
        "push %%r10\n\t"
        "push %%r11\n\t"
        "push %%r12\n\t"
        "push %%r13\n\t"
        "push %%r14\n\t"
        "push %%r15\n\t"
        ::);
}

inline static void __attribute__((always_inline))
restore_regs()
{
  asm ( "pop %%r15\n\t"
        "pop %%r14\n\t"
        "pop %%r13\n\t"
        "pop %%r12\n\t"
        "pop %%r11\n\t"
        "pop %%r10\n\t"
        "pop %%r9\n\t"
        "pop %%r8\n\t"
        "pop %%rsp\n\t"
        "pop %%rbp\n\t"
        "pop %%rdi\n\t"
        "pop %%rsi\n\t"
        "pop %%rdx\n\t"
        "pop %%rcx\n\t"
        "pop %%rbx\n\t"
        "pop %%rax\n\t"
        ::);
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    usage(argv[0]);
    return -1;
  }

  FILE *binfile = fopen(argv[1], "rb");

  if (!binfile) {
    printf("Cannot open file %s\n", argv[1]);
    return -2;
  }

  int file_size = get_file_size(binfile);

  if (binfile < 0) {
    puts("Failed when tried obtaining file size.");
    return -3;
  }

  int pagesize = sysconf(_SC_PAGE_SIZE);

  if (pagesize == -1) {
    puts("Getting system page size failed.");
    return -4;
  }

  char *mem_binary = memalign(pagesize, file_size);

  if (mem_binary == NULL) {
    puts("Failed to allocate mem_binary for binary."); 
    return -5;
  }

  if (mprotect(mem_binary, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
    puts("Failed to change alloced page attribute.");
    return -6;
  }

  puts("Successfully prepared page for user binary.");

  int bytes_written = fread(mem_binary, 1, file_size, binfile);
  if (bytes_written != file_size) {
    printf("Writing user binary failed! Supposed to write %i bytes, but %i bytes were actually written.",
           file_size,
           bytes_written);
    return -7;
  }

  printf("Written %i bytes.\n", bytes_written);

  userfunc_t f = (userfunc_t)(mem_binary);

  store_regs();
  f();
  restore_regs();
}
