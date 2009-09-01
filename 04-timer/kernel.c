#include <stdint.h>
#include <xen.h>
#include <sched.h>
#include "console.h"
#include "event.h"
#include "timer.h"

/* Some static space for the stack */
char stack[8192];


extern shared_info_t shared_info;
shared_info_t *HYPERVISOR_shared_info;


/* Main kernel entry point, called by trampoline */
void start_kernel (start_info_t *start_info)
{
	/* Map the shared info page */
	HYPERVISOR_update_va_mapping((unsigned long) &shared_info,
			__pte(start_info->shared_info | 7),
			UVMF_INVLPG);

	HYPERVISOR_shared_info = &shared_info;

	init_events ();
	console_init (start_info);
	init_timer ();
	console_write ("Just a hello!\r\n");

	while (1)
		HYPERVISOR_sched_op (SCHEDOP_block, 0);
}
