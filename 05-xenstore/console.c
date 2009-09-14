#include <stdint.h>
#include <stdarg.h>
#include <xen.h>
#include <sched.h>
#include <barrier.h>

#include "console.h"
#include "event.h"
#include "lib.h"

static evtchn_port_t event_channel;
extern char _text;
static struct xencons_interface *console;


void handle_input (evtchn_port_t port, struct pt_regs *regs)
{
	XENCONS_RING_IDX cons = console->in_cons;
	XENCONS_RING_IDX prod = console->in_prod;
	int length = prod - cons;
	if (length > 0)
	{
		char buffer[10];
		console_read(buffer, ++length);
		console_write(buffer);
	}
}


void console_init (start_info_t *si)
{
	console = (struct xencons_interface*)((machine_to_phys_mapping[si->console.domU.mfn] << 12) +
					      (unsigned long)&_text);
	event_channel = si->console.domU.evtchn;
	register_event (event_channel, handle_input);
}



int console_write (const char *msg)
{
	struct evtchn_send event;
	int len = 0;

	event.port = event_channel;

	while (*msg) {
		XENCONS_RING_IDX idx;

		do {
			idx = console->out_prod - console->out_cons;
			HYPERVISOR_event_channel_op (EVTCHNOP_send, &event);
			mb ();
		} while (idx >= sizeof (console->out));

		int ring_index = MASK_XENCONS_IDX (console->out_prod, console->out);

		console->out[ring_index] = *msg;
		wmb ();

		console->out_prod++;
		len++;

		msg++;
	}

	HYPERVISOR_event_channel_op (EVTCHNOP_send, &event);
	return len;
}


/* Read up to length characters from the console into buffer */
int console_read(char * buffer, int n)
{
	int length = 0;
	while(n > length  && console->in_cons < console->in_prod)
	{
		/* Copy the character */
		int ring_index = MASK_XENCONS_IDX(console->in_cons, console->in);
		*buffer = console->in[ring_index];
		buffer++;
		length++;
		console->in_cons++;
		wmb();
	}
	/* NULL-terminate the string */
	*buffer = '\0';
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


int console_write_int (int value, char *format)
{
	return printk (format, value);
}


int  printk (char *format, ...)
{
	static char buf[1024];
	va_list va;
	int res;

	va_start (va, format);
	vsnprintf (buf, sizeof (buf), format, va);
	va_end (va);
	return console_write (buf);
}
