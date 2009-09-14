#ifndef __LIB_H__
#define __LIB_H__

#include <stdarg.h>
#include <stddef.h>

#ifdef HAVE_LIBC
#include <stdio.h>
#else
/* printing */
#define _p(_x) ((void *)(unsigned long)(_x))
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char * buf, size_t size, const char *fmt, ...);
int scnprintf(char * buf, size_t size, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char * buf, const char *fmt, ...);
int vsscanf(const char * buf, const char * fmt, va_list args);
int sscanf(const char * buf, const char * fmt, ...);
#endif

long simple_strtol(const char *cp,char **endp,unsigned int base);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
long long simple_strtoll(const char *cp,char **endp,unsigned int base);
unsigned long long simple_strtoull(const char *cp,char **endp,unsigned int base);

#define RAND_MIX 2654435769U

int rand(void);

#ifdef HAVE_LIBC
#include <string.h>
#else
/* string and memory manipulation */
int    memcmp(const void *cs, const void *ct, size_t count);
void  *memcpy(void *dest, const void *src, size_t count);
int    strncmp(const char *cs, const char *ct, size_t count);
int    strcmp(const char *cs, const char *ct);
char  *strcpy(char *dest, const char *src);
char  *strncpy(char *dest, const char *src, size_t count);
void  *memset(void *s,int c, size_t count);
size_t strnlen(const char *s, size_t count);
size_t strlen(const char *s);
char  *strchr(const char *s, int c);
char  *strrchr(const char *s, int c);
char  *strstr(const char *s1, const char *s2);
char * strcat(char * dest, const char * src);
#endif


#endif	/* __LIB_H__ */
