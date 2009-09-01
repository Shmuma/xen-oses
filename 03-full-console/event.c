#include <stdint.h>
#include <xen.h>
#include <hypercall-x86_32.h>
#include <barrier.h>

#include "event.h"
#include "console.h"

#define NUM_CHANNELS 1024

static evtchn_handler_t handlers[NUM_CHANNELS];
extern volatile shared_info_t shared_info;

#define SET_BIT(bit,field) __asm__ __volatile__ ("lock btsl %1,%0":"=m"(field):"Ir"(bit):"memory" );
#define CLEAR_BIT(field, bit) __asm__ __volatile__ ("lock btrl %1,%0":"=m" ((field)):"Ir"((bit)):"memory")


void hypervisor_callback ();
void failsafe_callback ();


void EVT_IGN(evtchn_port_t port, struct pt_regs * regs) {};



void init_events ()
{
	HYPERVISOR_set_callbacks (FLAT_KERNEL_CS, (unsigned long)hypervisor_callback,
				  FLAT_KERNEL_CS, (unsigned long)failsafe_callback);

	/* Set all handlers to ignore, and mask them */
	for (unsigned int i = 0; i < NUM_CHANNELS; i++)
	{
		handlers[i] = EVT_IGN;
		SET_BIT (i, shared_info. evtchn_mask[0]);
	}

	/* Allow upcalls. */
	shared_info.vcpu_info[0].evtchn_upcall_mask = 0;
}



/* Register an event handler and unmask the port */
void register_event (evtchn_port_t port, evtchn_handler_t handler)
{
	handlers[port] = handler;
	CLEAR_BIT (shared_info.evtchn_mask, port);
}



static inline unsigned long int first_bit(unsigned long int word)
{
	__asm__("bsfl %1,%0"
		:"=r" (word)
		:"rm" (word));
	return word;
}


static inline unsigned long int xchg(unsigned long int * old, unsigned long int value)
{
	__asm__("xchgl %0,%1"
		:"=r" (value)
		:"m" (*old), "0"(value)
		:"memory");
	return value;
}



void do_hypervisor_callback (struct pt_regs *regs)
{
	unsigned int pending_selector;
	unsigned int next_event_offset;
	vcpu_info_t *vcpu = &shared_info.vcpu_info[0];

	/* Make sure we don't lose the edge on new events... */
	vcpu->evtchn_upcall_pending = 0;

	/* Set the pending selector to 0 and get the old value atomically */
	pending_selector = xchg(&vcpu->evtchn_pending_sel, 0);

	while (pending_selector != 0)
	{
		/* Get the first bit of the selector and clear it */
		next_event_offset = first_bit (pending_selector);
		pending_selector &= ~(1 << next_event_offset);
		unsigned int event;

		/* While there are events pending on unmasked channels */
		while(( event =
		(shared_info.evtchn_pending[pending_selector] 
		 & 
		~shared_info.evtchn_mask[pending_selector]))
		 != 0)
		{
			/* Find the first waiting event */
			unsigned int event_offset = first_bit(event);

			/* Combine the two offsets to get the port */
			evtchn_port_t port = (pending_selector << 5) + event_offset;
			/* Handle the event */
			handlers[port](port, regs);
			/* Clear the pending flag */
			CLEAR_BIT(shared_info.evtchn_pending[0], event_offset);
		}
	}
}
