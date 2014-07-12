//////////////////////////////////////////////////////////////////////
// btsig.hpp -- BlueTooth Serial Internet Gateway (Teensy Stack)
// Date: Wed Jul  9 22:31:19 2014   (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#ifndef BTSIG_HPP
#define BTSIG_HPP

#include "enums.hpp"
#include "fibers.hpp"
#include "slipser.hpp"
#include "packet.hpp"

class BTSIG : public SlipSer {
	Fibers<16>		fibers;		// Collection of co-routines
	unsigned		rx_fiber;	// Receiving fiber #
	unsigned		tx_fiber;	// Transmitting fiber #

	volatile bool		f_enum;		// True after interfaces have been enumerated
	volatile uint32_t	seqno;		// Outgoing sequence numbers

	struct s_iface {
		protocols_e	protocol;	// Interface protocol
		const char	*addr;		// Interface address string
		s_iface		*next;		// Ptr to next if any
		uint8_t		ifno;		// Interface #
	};

	uint16_t		interfaces;	// Count of interfaces available
	s_iface			*ifroot;	// Ptr to root interface
	s_iface			**ifnext;	// Points to ifroot->next

	struct s_request {
		volatile time_t		reqtime;	// Request time
		time_t			timeout;	// Timeout time in seconds
		Packet			*pkt;		// Packet buffer (I/O)
		command_e		cmd;		// Command ID
		volatile uint32_t	seqno;		// Sequence no of this request
		volatile s_request	*next;		// Next request
		volatile bool		serviced;	// True when the request has been serviced
	};

	volatile s_request	*txroot;	// Root of request queue
	volatile s_request	**txtail;	// Ptr to tail
	volatile s_request	*rxroot;	// Root of requests waiting a reply
	volatile s_request	**rxtail;	// Ptr to rx tail

	void receiver();
	void transmitter();	
	void yield();
	void wait_ready();

	inline uint32_t _seqno()	{ return seqno++; }

	unsigned _request(Packet& pkt,time_t timeout);

	int _socket(com_domain_e domain,sock_type_e type,int protocol);
	int _connect(int sock,unsigned port,const char *address);

public:	BTSIG();

	unsigned get_interfaces();
	unsigned get_interface(unsigned ifno,const char **piface,protocols_e& protocol);

	static time_t time();

	static void recv(void *arg);
	static void xmit(void *arg);
	static void terminate();

	static int socket(com_domain_e domain,sock_type_e type,int protocol);
	static int connect(int sock,unsigned port,const char *address);
};


extern BTSIG btsig;


#endif // BTSIG_HPP

// End btsig.hpp
