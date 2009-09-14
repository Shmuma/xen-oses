#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <xen.h>
#include <hypercall-x86_32.h>
#include <io/console.h>

void console_init (start_info_t *start_info);
int  console_write (const char *msg);
int  console_read(char * buffer, int n);
int  console_write_int (int value, char *format);
int  printk (char *format, ...);

#endif
