#include <stdint.h>
#include <xen.h>
#include <event_channel.h>
#include <io/console.h>

#include <barrier.h>

#include "console.h"


struct xencons_interface *console;
static evtchn_port_t console_evt;
extern char _text;



int console_init (start_info_t *start)
{
	console = (struct xencons_interface*)
		((machine_to_phys_mapping[start->console.domU.mfn] << 12) + (unsigned long)&_text);
	console_evt = start->console.domU.evtchn;
	return 0;
}



int console_write (const char *msg)
{
	struct evtchn_send event;
	int length = 0;

	event.port = console_evt;

	while (*msg) {
		XENCONS_RING_IDX data;

		while (console->out_prod - console->out_cons >= sizeof (console->out)) {
			HYPERVISOR_event_channel_op (EVTCHNOP_send, &event);
			mb ();
		}

		int ring_index = MASK_XENCONS_IDX (console->out_prod, console->out);

		if (*msg == '\n') {
			console->out[ring_index] = '\r';
			console->out_prod++;
			ring_index = MASK_XENCONS_IDX (console->out_prod, console->out);
		}
		console->out[ring_index] = *msg;

		wmb ();
		console->out_prod++;

		length++;
		msg++;
	}

	HYPERVISOR_event_channel_op (EVTCHNOP_send, &event);
	return length;
}


void console_flush (void)
{
	while (console->out_cons < console->out_prod) {
		HYPERVISOR_sched_op (SCHEDOP_yield, 0);
		mb ();
	}
}
