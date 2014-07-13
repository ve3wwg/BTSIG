//////////////////////////////////////////////////////////////////////
// btsig.cpp -- Implementation of BTSIG Class (Teensy Stack)
// Date: Wed Jul  9 22:33:55 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "btsig.hpp"
#include "packet.hpp"

BTSIG btsig;			// Single instance object

//////////////////////////////////////////////////////////////////////
// Construct the BTSIG object
//////////////////////////////////////////////////////////////////////

BTSIG::BTSIG() {
	f_enum = false;
	ifroot = 0;
	ifnext = &ifroot;
	interfaces = 0;
	seqno = 0;

	txroot = 0;
	txtail = &txroot;
	rxroot = 0;
	rxtail = &rxroot;

	rx_fiber = fibers.create(BTSIG::recv,0,8192);
	tx_fiber = fibers.create(BTSIG::xmit,0,8192);

	btsig.SlipSer::connect();
}

//////////////////////////////////////////////////////////////////////
// Join with BTSIG for termination (incomplete)
//////////////////////////////////////////////////////////////////////

void
BTSIG::terminate() {
	btsig.fibers.join(btsig.rx_fiber);
	btsig.fibers.join(btsig.tx_fiber);
}

//////////////////////////////////////////////////////////////////////
// Receiving fiber
//////////////////////////////////////////////////////////////////////

void
BTSIG::receiver() {
	uint8_t buf[64];
	unsigned rlen;
	command_e rsp;

	while ( !f_enum ) {
		rlen = SlipSer::read(buf,sizeof buf);
		Packet pkt(buf,rlen);
		uint8_t b;

		pkt >> b;
		rsp = static_cast<command_e>(b);

		switch ( rsp ) {
		case R_Enumerate_IPv4 :
			{
				uint8_t ifno;
				const char *addr;

				pkt >> ifno;
				addr = pkt.c_str();

				s_iface *newif = new s_iface;

				newif->protocol = P_IPv4;
				newif->addr     = strdup(addr);
				newif->ifno     = ifno;
				newif->next     = 0;

				*ifnext = newif;
				ifnext = &newif->next;
				++interfaces;
			}
			break;
		case R_Enumerate_IPv6 :
			{
				uint8_t ifno;
				const char *addr;

				pkt >> ifno;
				addr = pkt.c_str();

				s_iface *newif = new s_iface;

				newif->protocol = P_IPv6;
				newif->addr     = strdup(addr);
				newif->ifno     = ifno;
				newif->next     = 0;

				*ifnext = newif;
				ifnext = &newif->next;
				++interfaces;
			}
			break;
		case R_Enumerate_End :
			f_enum = true;
			break;
		default :
			;
		}
	}

	for (;;) {
		rlen = SlipSer::read(buf,sizeof buf);
		Packet pkt(buf,rlen);
		uint8_t b;
		uint32_t seqno;

		pkt >> b >> seqno;

printf("Receiver response %02X seqno %u\n",b,seqno);

		volatile s_request * volatile *lastp = &rxroot;
		volatile s_request *req = rxroot;

		// Match the request by seqno
		for ( ; req; lastp = &req->next, req = req->next )
			if ( req->seqno == seqno )
				break;
		
		if ( !req || req->seqno != seqno )
			continue;			// Spurious reply

		// Process the request
		*lastp = req->next;			// Take this request off of the list
		req->next = 0;

		req->pkt->copy(pkt);			// Copy response packet data into caller's buffer
		req->serviced = true;			// Mark this request as serviced

printf("Receiver response %02X seqno %u SERVICED.\n",b,seqno);
	}
}

//////////////////////////////////////////////////////////////////////
// Transmitting fiber
//////////////////////////////////////////////////////////////////////

void
BTSIG::transmitter() {
	static uint8_t cmd_enumerate[] = { C_Enumerate };
	time_t req_sent = 0;

	while ( !btsig.f_enum ) {
		if ( !req_sent || (time() - req_sent) > 3 ) {
			SlipSer::write(cmd_enumerate,sizeof cmd_enumerate);
			req_sent = time();
		}
	}

	puts("xmit ready..");

	for (;;) {
		if ( txroot != 0 ) {
			volatile s_request *req = txroot;	// First request
			txroot = req->next;			// Remove from tx queue
			*rxtail = req;				// Add to receiver queue (in case we get fast reply)
			SlipSer::write(req->pkt->data(),req->pkt->size()); // Write packet out
printf("Request transmitted.\n");
		} else if ( rxroot != 0 ) {
			time_t now = time();				// Get current time
			volatile s_request * volatile *lastp = &rxroot;	// Tail pointer
			volatile s_request *req = rxroot;		// Get head of receiving queue

			for ( ; req != 0; lastp = &req->next, req = req->next ) {
				if ( req->reqtime + req->timeout <= now ) {
					// This request has timed out:
					req->seqno = _seqno();				// Assign new sequence #
					req->reqtime = now;
					req->pkt->seek(1u);
					*req->pkt << uint32_t(req->seqno);		// Rewrite the sequence #
printf("Retransmitting request %02X as new seqno %u\n",req->cmd,req->seqno);
					SlipSer::write(req->pkt->data(),req->pkt->size());	// Resend request with new seqno
				}
			}
			yield();
		} else	{
			yield();			// Nothing to do
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Internal: Wait until enumeration completes
//////////////////////////////////////////////////////////////////////

void
BTSIG::wait_ready() {
	while ( !f_enum )
		yield();
}

//////////////////////////////////////////////////////////////////////
// Get # of network interfaces
//////////////////////////////////////////////////////////////////////

unsigned
BTSIG::get_interfaces() {
	wait_ready();
	return interfaces;
}

//////////////////////////////////////////////////////////////////////
// Obtain address of a particular interface (and its protocol)
//////////////////////////////////////////////////////////////////////

unsigned
BTSIG::get_interface(unsigned ifno,const char **piface,protocols_e& protocol) {
	unsigned ino = 0;

	*piface = 0;
	protocol = P_None;

	wait_ready();

	if ( ifno >= interfaces )
		return ~0;		// Invalid

	for ( s_iface *p = ifroot; p != 0; p = p->next, ++ino ) {
		if ( ino == ifno ) {
			*piface = p->addr;
			protocol = p->protocol;
			return ino;
		}
	}

	return ~0;			// Invalid
}

//////////////////////////////////////////////////////////////////////
// Yield control to another Fiber
//////////////////////////////////////////////////////////////////////

void
BTSIG::yield() {
	fibers.yield();
}

//////////////////////////////////////////////////////////////////////
// Return the platform's sense of Unix time
//////////////////////////////////////////////////////////////////////

time_t
BTSIG::time() {
#ifdef SIMULATE
	return ::time(0);
#else
#endif
}

//////////////////////////////////////////////////////////////////////
// Static method to start receiving Fiber
//////////////////////////////////////////////////////////////////////

void
BTSIG::recv(void *arg) {
	btsig.receiver();
}

//////////////////////////////////////////////////////////////////////
// Static method to start transmitting Fiber
//////////////////////////////////////////////////////////////////////

void
BTSIG::xmit(void *arg) {
	btsig.transmitter();
}

//////////////////////////////////////////////////////////////////////
// Queue a request on half of the client. This routine blocks until
// the request is marked as "serviced"
//////////////////////////////////////////////////////////////////////

unsigned
BTSIG::_request(Packet& pkt,time_t timeout) {
	uint8_t b;
	uint32_t seqno;
	s_request *req = new s_request;

	pkt.set_size(pkt.offset());		// Set size of the formatted packet
	pkt.rewind();				// Rewind for reading packet header

	pkt >> b >> seqno;			// Get cmd and seqno
	req->seqno = seqno;
	req->pkt = &pkt;			// Save ref to caller's pkt and buffer
	req->cmd = command_e(b);		// Command type
	req->serviced = false;			// Not serviced yet
	req->next = 0;				// No next pointer
	req->reqtime = time();			// Time of this request
	req->timeout = timeout;			// When it times out

	*txtail = req;				// Put request on the queue

printf("Request %02X seqno %u requested\n",b,seqno);

	while ( !req->serviced )
		yield();			// Yield until request is serviced

printf("Request answered: size = %u\n",pkt.size());

	free(req);

	return pkt.size();
}

//////////////////////////////////////////////////////////////////////
// 				A P I
//////////////////////////////////////////////////////////////////////

int
BTSIG::_socket(com_domain_e domain,sock_type_e type,int protocol) {

	switch ( domain ) {
	case CD_AF_INET :
		break;
	case CD_AF_INET6 :
	default :
		return -E_EINVAL;
	}

	switch ( type ) {
	case ST_SOCK_STREAM :
		break;
	case ST_SOCK_DGRAM :
	default :
		return -E_EINVAL;
	}

	char buf[64];
	Packet pkt(buf,sizeof buf);	

	pkt 	<< uint8_t(C_Socket) << _seqno()
		<< uint8_t(domain) << uint8_t(type) << int32_t(protocol);

	if ( !_request(pkt,8) )
		return -E_EIO;			// Request failed
	
	int sock = -1;

	pkt.seek(5);
	pkt >> sock;

	return sock;
}

int
BTSIG::_connect(int sock,unsigned port,const char *address) {
	char buf[128];
	Packet pkt(buf,sizeof buf);	

	pkt << uint8_t(C_Connect) << _seqno() << int16_t(sock) << int16_t(port) << address;

	if ( !_request(pkt,8) )
		return -E_EIO;			// Request failed
	
	uint16_t urc;

	pkt.seek(5);
	pkt >> urc;

	return -int(urc);			// Returns zero when successful
}

int
BTSIG::_close(int sock) {
	char buf[32];
	Packet pkt(buf,sizeof buf);

	pkt << uint8_t(C_Close) << _seqno() << int16_t(sock);
	if ( !_request(pkt,8) )
		return -E_EIO;

	uint16_t urc;
	pkt.seek(5);
	pkt >> urc;

	return -int(urc);
}

int
BTSIG::_write(int sock,const void *buffer,unsigned bytes) {

	if ( bytes > MAX_IO_BYTES )
		return -E_EINVAL;

	char buf[bytes+32];
	Packet pkt(buf,bytes+32);

	pkt << uint8_t(C_Write) << _seqno() << int16_t(sock) << int16_t(bytes);
	pkt.put(buffer,bytes);

	if ( !_request(pkt,10) )
		return -E_EIO;

	uint16_t urc, wrbytes;
	pkt.seek(5);
	pkt >> urc >> wrbytes;

	return !urc ? wrbytes : -int(urc);
}

int
BTSIG::_read(int sock,void *buffer,unsigned bytes) {

	if ( bytes > MAX_IO_BYTES )
		return -E_EINVAL;

	char buf[bytes+32];
	Packet pkt(buf,bytes+32);

printf("Requesting read of %u bytes\n",bytes);
	pkt << uint8_t(C_Read) << _seqno() << int16_t(sock) << uint16_t(bytes);

	if ( !_request(pkt,10) )
		return -E_EIO;

	uint16_t urc, rdlen;
	pkt.seek(5);
	pkt >> urc;

printf("Read rc = %u..\n",urc);

	if ( urc != 0 )
		return -int(urc);	// Error returned

	pkt >> rdlen;
	assert(rdlen <= bytes);

printf("Read %u bytes!\n",rdlen);

	if ( rdlen > 0 )
		pkt.get(buffer,rdlen);

	return int(rdlen);		// # of bytes returned
}

int
BTSIG::socket(com_domain_e domain,sock_type_e type,int protocol) {
	return btsig._socket(domain,type,protocol);
}

int
BTSIG::connect(int sock,unsigned port,const char *address) {
	return btsig._connect(sock,port,address);
}

int
BTSIG::close(int sock) {
	return btsig._close(sock);
}

int
BTSIG::write(int sock,const void *buffer,unsigned bytes) {
	return btsig._write(sock,buffer,bytes);
}

int
BTSIG::read(int sock,void *buffer,unsigned bytes) {
	return btsig._read(sock,buffer,bytes);
}

// End btsig.cpp
