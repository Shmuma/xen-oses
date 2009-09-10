#ifndef __STORE_H__
#include <stdint.h>
#include <xen.h>

void store_init (start_info_t *start_info);
int store_write (char *key, char *val);
int store_read  (char *key, char *val, int len);
int store_ls  (char *key, char *val, int len);

#endif	/* __STORE_H__ */
