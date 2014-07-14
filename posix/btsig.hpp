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

	uint8_t			pktbuf[MAX_IO_BYTES];

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
		volatile unsigned	serviced : 1;	// True when the request has been serviced
		volatile unsigned	no_retry : 1;	// When true, there can be no retry 
		volatile unsigned	failed : 1;	// No reply received (and no retry)
	};

	volatile s_request	*txroot;	// Root of request queue
	volatile s_request	**txtail;	// Ptr to tail
	volatile s_request	*rxroot;	// Root of requests waiting a reply
	volatile s_request	**rxtail;	// Ptr to rx tail

	struct s_btsock {
		int			fd;		// Open file descriptor
		volatile uint32_t	rseqno;		// Sequ no that needs to be ACKed
		volatile uint32_t	wseqno;		// Sequ no for write that needs ACK
		volatile unsigned	ack_read : 1;	// When true, ACK prior read
		volatile unsigned	ack_write : 1;	// When true, ACK prior write
		volatile s_btsock	*next;
	};

	volatile s_btsock	*skroot;	// Root of open sockets list
	volatile s_btsock	**sktail;	// Ptr to tail

	void receiver();
	void transmitter();	
	void yield();
	void wait_ready();

	inline uint32_t _seqno()	{ return seqno++; }

	unsigned _request(Packet& pkt,time_t timeout,bool retry=true);

	s_btsock *_new_btsock(int fd);
	s_btsock *_locate_btsock(int fd,s_btsock ***lastp);
	bool _delete_btsock(int fd);

	int _socket(com_domain_e domain,sock_type_e type,int protocol);
	int _connect(int sock,unsigned port,const char *address);
	int _write(int sock,const void *buffer,unsigned bytes);
	int _read(int sock,void *buffer,unsigned bytes);
	int _close(int sock);

public:	BTSIG();

	unsigned get_interfaces();
	unsigned get_interface(unsigned ifno,const char **piface,protocols_e& protocol);

	static time_t time();

	static void recv(void *arg);
	static void xmit(void *arg);
	static void terminate();

	static int socket(com_domain_e domain,sock_type_e type,int protocol);
	static int connect(int sock,unsigned port,const char *address);
	static int write(int sock,const void *buffer,unsigned bytes);
	static int read(int sock,void *buffer,unsigned bytes);
	static int close(int sock);
};


extern BTSIG btsig;


#endif // BTSIG_HPP

// End btsig.hpp
