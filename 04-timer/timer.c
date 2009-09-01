#include <stdint.h>
#include <xen.h>

#include "console.h"
#include "event.h"
#include "timer.h"


void timer_handler (evtchn_port_t port, struct pt_regs *regs)
{
	console_write ("Timer event\r\n");
}


void init_timer ()
{
	evtchn_bind_virq_t op;

	console_write ("Initialize periodical timer handler\r\n");
	
	op.virq = VIRQ_TIMER;
	op.vcpu = 0;
	
	if (HYPERVISOR_event_channel_op (EVTCHNOP_bind_virq, &op) != 0)
		console_write ("Error bind virq\r\n");
	else
		register_event (op.port, timer_handler);
}
