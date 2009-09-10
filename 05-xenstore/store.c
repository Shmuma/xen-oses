#include <stdint.h>
#include <xen.h>

#include "console.h"
#include "store.h"

extern char _text;



void store_init (start_info_t *start_info)
{
	console_write ("Initialize XenStore\r\n");
}
