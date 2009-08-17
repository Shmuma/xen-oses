#include <stdint.h>
#include <xen.h>
#include <event_channel.h>
#include <io/console.h>

#include <barrier.h>

#include "console.h"


struct xencons_interface *console;
static evtchn_port_t console_evt;
extern char _text;



/* Initialise the console */
int console_init(start_info_t * start)
{
	console = (struct xencons_interface*)
		((machine_to_phys_mapping[start->console.domU.mfn] << 12)
		 +
		((unsigned long)&_text));
	console_evt = start->console.domU.evtchn;
	/* TODO: Set up the event channel */
	return 0;
}

/* Write a NULL-terminated string */
int console_write(char * message)
{
	struct evtchn_send event;
	event.port = console_evt;
	int length = 0;
	while(*message != '\0')
	{
		/* Wait for the back end to clear enough space in the buffer */
		XENCONS_RING_IDX data;
		do
		{
			data = console->out_prod - console->out_cons;
			HYPERVISOR_event_channel_op(EVTCHNOP_send, &event);
			mb();
		} while (data >= sizeof(console->out));
		/* Copy the byte */
		int ring_index = MASK_XENCONS_IDX(console->out_prod, console->out);
		console->out[ring_index] = *message;
		/* Ensure that the data really is in the ring before continuing */
		wmb();
		/* Increment input and output pointers */
		console->out_prod++;
		length++;
		message++;
	}
	HYPERVISOR_event_channel_op(EVTCHNOP_send, &event);
	return length;
}

/* Block while data is in the out buffer */
void console_flush(void)
{
	/* While there is data in the out channel */
	while(console->out_cons < console->out_prod)
	{
		/* Let other processes run */
		HYPERVISOR_sched_op(SCHEDOP_yield, 0);
		mb();
	}
}



/* int console_init (start_info_t *start) */
/* { */
/* 	console = (struct xencons_interface*) */
/* 		((machine_to_phys_mapping[start->console.domU.mfn] << 12) + (unsigned long)&_text); */
/* 	console_evt = start->console.domU.evtchn; */
/* 	return 0; */
/* } */



/* int console_write (const char *msg) */
/* { */
/* 	struct evtchn_send event; */
/* 	int length = 0; */

/* 	event.port = console_evt; */

/* 	while (*msg) { */
/* 		XENCONS_RING_IDX data; */

/* 		do { */
/* 			data = console->out_prod - console->out_cons; */
/* 			HYPERVISOR_event_channel_op (EVTCHNOP_send, &event); */
/* 			mb (); */
/* 		} */
/* 		while (data >= sizeof (console->out)); */

/* 		int ring_index = MASK_XENCONS_IDX (console->out_prod, console->out); */
/* 		console->out[ring_index] = *msg; */

/* 		wmb (); */
/* 		console->out_prod++; */

/* 		length++; */
/* 		msg++; */
/* 	} */

/* 	HYPERVISOR_event_channel_op (EVTCHNOP_send, &event); */
/* 	return length; */
/* } */
