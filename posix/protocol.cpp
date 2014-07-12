//////////////////////////////////////////////////////////////////////
// protocol.cpp -- Protocol module
// Date: Wed Jul  9 21:16:44 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "protocol.hpp"
#include "packet.hpp"
#include "enums.hpp"

static Protocol protocol;

Protocol::Protocol() {
}

bool
Protocol::make_ipv4_addr(sockaddr_in& sockaddr,unsigned port,const char *addr) {

	if ( !(port & 0xFFFF) )
		return false;

	memset(&sockaddr,0,sizeof sockaddr);
	if ( !inet_pton(AF_INET,addr,&sockaddr.sin_addr) ) {
		struct hostent *h = gethostbyname(addr);

		if ( !h )
			return false;		// Failed lookup

		if ( h->h_addrtype != AF_INET )
			return false;

		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr   = *(in_addr *)h->h_addr;
		sockaddr.sin_port   = htons(port);
		return true;
	}

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	return true;
}

void
Protocol::enumerate() {
	static const unsigned np = 2;
	static command_e rsp[np+1] = { R_Enumerate_IPv4, R_Enumerate_IPv6, R_Enumerate_End };
	static protocols_e proto[np] = { P_IPv4, P_IPv6 };
	char pbuf[128];
	unsigned ifno = 0;
	
	for ( unsigned px = 0; px < np; ++px ) {
		unsigned n = iface.size(proto[px]);

		for ( unsigned ux = 0; ux < n && ux < 256; ++ux ) {
			Packet p(pbuf,sizeof pbuf);
			const std::string& addr = iface.get(proto[px],ux);

			p << uint8_t(rsp[px]) << uint8_t(ifno++) << addr.c_str();
			send(p);
		}
	}

	write(&rsp[np],1);
}

void
Protocol::socket(Packet& pkt) {
	uint8_t cmd, b;
	uint32_t seqno;
	com_domain_e domain;
	sock_type_e type;
	int protocol;
	bool failed = false;

	pkt.rewind();
	pkt >> cmd >> seqno >> b;
	domain = static_cast<com_domain_e>(b);
	pkt >> b >> protocol;
	type = static_cast<sock_type_e>(b);
	
	int sock_domain, sock_type;

	switch ( domain ) {
	case CD_AF_INET :
		sock_domain = AF_INET;
		break;
	case CD_AF_INET6 :
		sock_domain = AF_INET6;
		break;
	default :
		failed = true;
	}

	switch ( type ) {
	case ST_SOCK_STREAM :
		sock_type = SOCK_STREAM;
		break;
	case ST_SOCK_DGRAM :
		sock_type = SOCK_DGRAM;
		break;
	default :
		failed = true;
	}		

	int fd = ::socket(sock_domain,sock_type,protocol);

printf(" got socket fd = %d;\n",fd);

	pkt.reset();
	pkt << cmd << seqno << fd;
	pkt.set_size(pkt.offset());
	assert(pkt.size() == 9);
	send(pkt);
}

void
Protocol::connect(Packet& pkt) {
	uint8_t cmd;
	uint32_t seqno;
	uint16_t sock, port;
	const char *addr = 0;
	int16_t er = E_OK;

	pkt.rewind();
	pkt >> cmd >> seqno >> sock >> port;
	addr = pkt.c_str();

	printf("Got connect(sock=%d,%u,'%s')\n",sock,port,addr);

	sockaddr_in skaddr;

	if ( !make_ipv4_addr(skaddr,port,addr) ) {
		er = E_EINVAL;
	} else	{
		int rc = ::connect(sock,(sockaddr *)&skaddr,sizeof skaddr);
		
		if ( rc == -1 ) {
			switch ( errno ) {
			case ECONNREFUSED :
				er = E_ECONNREFUSED;
				break;
			case EHOSTUNREACH :
				er = E_EHOSTUNREACH;
				break;
			default :
				er = E_EIO;
			}
		}
	}

	pkt.reset();
	pkt << cmd << seqno << er;

	pkt.set_size(pkt.offset());
	send(pkt);
}

void
Protocol::receiver() {

	listen();

	uint8_t buf[64];
	unsigned retlen = 0;

	for (;;) {
		retlen = protocol.read(buf,sizeof buf);
		if ( retlen < 1 )
			continue;

		Packet pkt(buf,retlen);
		uint8_t cmd;
		uint32_t seqno;

		pkt >> cmd;

		switch ( cmd ) {
		case C_Enumerate :
			enumerate();
			break;
		case C_Socket :
			socket(pkt);
			break;
		case C_Connect :
			connect(pkt);
			break;
		default :
			pkt >> seqno;
printf("Received request %02X seqno %u\n",(unsigned)cmd,seqno);
			continue;
		}
	}
}

void
Protocol::recv() {
	protocol.receiver();
}

void
Protocol::xmit() {
	;
}

void
Protocol::send(const Packet& pkt) {
	write(pkt.data(),pkt.size());
}

// End protocol.cpp
