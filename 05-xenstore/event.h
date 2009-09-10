#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdint.h>
#include <xen.h>
#include <event_channel.h>


struct pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
};

typedef void (*evtchn_handler_t) (evtchn_port_t, struct pt_regs *);


void init_events ();
void register_event (evtchn_port_t port, evtchn_handler_t handler);

#endif
