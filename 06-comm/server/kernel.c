#include <stdint.h>
#include <xen.h>
#include <sched.h>

#include "lib.h"
#include "console.h"
#include "event.h"
#include "store.h"
#include "server.h"

/* Some static space for the stack */
char stack[8192];


extern shared_info_t shared_info;
shared_info_t *HYPERVISOR_shared_info;



/* Main kernel entry point, called by trampoline */
void start_kernel (start_info_t *start_info)
{
	sched_shutdown_t op;
	int res;

	/* Map the shared info page */
	HYPERVISOR_update_va_mapping((unsigned long) &shared_info,
			__pte(start_info->shared_info | 7),
			UVMF_INVLPG);

	HYPERVISOR_shared_info = &shared_info;

	init_events ();
	console_init (start_info);
	console_write ("XenStore demo kernel starting...\r\n");

	store_init (start_info);

	printk ("pt_base = 0x%p\r\n", start_info->pt_base);
	printk ("nr_pages = %d\r\n", start_info->nr_pages);

	res = server_init (start_info);
	printk ("server_init () = %d\r\n", res);

	while (1)
		HYPERVISOR_sched_op (SCHEDOP_block, 0);
}
