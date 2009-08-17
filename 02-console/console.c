#include <stdint.h>
#include <xen.h>

#include <event_channel.h>
#include <io/console.h>

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


