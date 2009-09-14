#include <stdint.h>
#include <xen.h>
#include <sched.h>
#include <barrier.h>
#include <io/ring.h>
#include <x86_mm.h>

#include "server.h"
#include "lib.h"
#include "console.h"
#include "comm.h"


DEFINE_RING_TYPES (comm, struct comm_request, struct comm_response);

static comm_front_ring_t front_ring;
static comm_back_ring_t back_ring;



int server_init (start_info_t *si)
{
/* 	xen_pfn_t page = virt_to_pfn (si->pt_base) + si->nr_pt_frames + si->nr_pages-1; */
/* 	char *addr; */

/* 	addr = (void*)((machine_to_phys_mapping[page] << 12) + */
/* 		       (unsigned long)&_text); */

/* 	printk ("console mfn = 0x%x\r\n", si->console.domU.mfn); */
/* 	printk ("pfn = 0x%x, addr = 0x%p\r\n", page, addr); */
/* 	*addr = 'f'; */
/* 	printk ("Value: '%c'\r\n", *addr); */

	/* create requests ring */
	/* create requests channel */
	/* publish channel info in xenstore */
}
