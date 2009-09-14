#include <stdint.h>
#include <errno.h>
#include <xen.h>

#include <io/xs_wire.h>

#include "barrier.h"
#include "console.h"
#include "store.h"
#include "lib.h"

extern char _text;

static struct xenstore_domain_interface *xs_intf;
static evtchn_port_t event_channel;
static int req_id = 0;


#define NOTIFY()							\
	do {								\
		struct evtchn_send event;				\
		event.port = event_channel;				\
		HYPERVISOR_event_channel_op (EVTCHNOP_send, &event);	\
	} while (0);


#define IGNORE(n)					\
	do {						\
		char buffer[XENSTORE_RING_SIZE];	\
		store_read_response (buffer, n);	\
	} while (0);



void store_init (start_info_t *start_info)
{
	console_write ("Initialize XenStore\r\n");
	xs_intf = (struct xenstore_domain_interface*)((machine_to_phys_mapping[start_info->store_mfn] << 12) + (unsigned long)&_text);
	event_channel = start_info->store_evtchn;
}


int store_write_request (char *msg, int len)
{
	int i;

	if (len > XENSTORE_RING_SIZE)
		return -1;

	for (i = xs_intf->req_prod; len > 0; i++, len--) {
		XENSTORE_RING_IDX idx;
		do {
			idx = i - xs_intf->req_cons;
			mb ();
		}
		while (idx >= XENSTORE_RING_SIZE);

		xs_intf->req[MASK_XENSTORE_IDX (i)] = *msg;
		msg++;
	}

	wmb ();

	xs_intf->req_prod = i;
	return 0;
}


int store_read_response (char *msg, int len)
{
	int i;

	for (i = xs_intf->rsp_cons; len > 0; i++, len--) {
		XENSTORE_RING_IDX idx;

		do {
			idx = xs_intf->rsp_prod - i;
			mb ();
		}
		while (idx == 0);

		*msg = xs_intf->rsp[MASK_XENSTORE_IDX (i)];
		msg++;
	}

	xs_intf->rsp_cons = i;
	return 0;
}


int store_write (char *key, char *value)
{
	int key_len = strlen (key);
	int val_len = strlen (value);
	struct xsd_sockmsg msg;

	msg.type = XS_WRITE;
	msg.req_id = req_id;
	msg.tx_id = 0;
	msg.len = 2 + key_len + val_len;

	store_write_request ((char*)&msg, sizeof (msg));
	store_write_request (key, key_len+1);
	store_write_request (value, val_len+1);
	NOTIFY ();

	store_read_response ((char*)&msg, sizeof (msg));

#if 0
	static char buf[256];
	console_write_int (msg.type, "reply type = %d, ");
	console_write_int (msg.len, "reply len = %d, msg = ");
	console_write (buf);
	console_write ("\r\n");
#else
	IGNORE (msg.len);
#endif

	if (msg.req_id != req_id++)
		return -1;
	return 0;
}



int store_read (char *key, char *val, int len)
{
	int key_len = strlen (key);
	struct xsd_sockmsg msg;

	msg.type = XS_READ;
	msg.req_id = req_id;
	msg.tx_id = 0;
	msg.len = 1 + key_len;

	store_write_request ((char*)&msg, sizeof (msg));
	store_write_request (key, key_len+1);
	NOTIFY ();

	store_read_response ((char*)&msg, sizeof (msg));

	if (msg.req_id != req_id++) {
		IGNORE (msg.len);
		return -1;
	}

	if (msg.len <= len) {
		store_read_response (val, msg.len);
		return 0;
	}

	store_read_response (val, len);
	IGNORE (msg.len - len);
	return -2;
}



int store_ls (char *key, char *val, int len)
{
	int key_len = strlen (key);
	struct xsd_sockmsg msg;

	msg.type = XS_DIRECTORY;
	msg.req_id = req_id;
	msg.tx_id = 0;
	msg.len = 1 + key_len;
	
	store_write_request ((char*)&msg, sizeof (msg));
	store_write_request (key, key_len+1);
	NOTIFY ();

	store_read_response ((char*)&msg, sizeof (msg));
	
	if (msg.req_id != req_id++) {
		IGNORE (msg.len);
		return -1;
	}

	if (msg.len <= len) {
		store_read_response (val, msg.len);
		return msg.len;
	}

	IGNORE (msg.len);
	return -2;
}
