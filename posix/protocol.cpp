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

	SlipSer::write(&rsp[np],1);
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

	pkt.reset();
	pkt << cmd << seqno << fd;
	pkt.close();
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

	pkt.rewind() >> cmd >> seqno >> sock >> port;
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

	pkt.reset() << cmd << seqno << er;
	pkt.close();
	send(pkt);
}

void
Protocol::close(Packet& pkt) {
	uint8_t cmd;
	uint32_t seqno;
	uint16_t sock;
	int16_t er = E_OK;
	int rc;

	pkt.rewind() >> cmd >> seqno >> sock;

	do	{
		rc = ::close(sock);
	} while ( rc == -1 && errno == EINTR );
		
	if ( rc == -1 ) {
		switch ( errno ) {
		case EBADF :
			er = E_EBADF;
			break;
		case EIO :
		default :
			er = E_EIO;
		}
	}

printf("Got close(sock=%d) returns %d\n",sock,er);

	pkt.reset() << cmd << seqno << er;
	pkt.close();
	send(pkt);
}

void
Protocol::read(Packet& pkt) {
	uint8_t cmd;
	uint32_t seqno;
	uint16_t sock, bytes;
	char iobuf[MAX_IO_BYTES];
	int16_t er = E_OK;
	int rc;

	pkt.rewind() >> cmd >> seqno >> sock >> bytes;

	if ( bytes <= MAX_IO_BYTES ) {
		do	{
			rc = ::read(sock,iobuf,bytes);
		} while ( rc == -1 && errno == EINTR );

		if ( rc == -1 ) {
			switch ( errno ) {
			case EBADF :
				er = E_EBADF;
				break;
			case EIO :
			default :
				er = E_EIO;
			}
			bytes = 0;
		} else	{
			bytes = rc;
		}
	} else	{
		er = E_EINVAL;
		bytes = 0;
	}

	pkt.reset() << cmd << seqno << er << bytes;
	if ( bytes > 0 )
		pkt.put(iobuf,bytes);

	pkt.close();
	send(pkt);
}

void
Protocol::write(Packet& pkt) {
	uint8_t cmd;
	uint32_t seqno;
	uint16_t sock, bytes;
	int16_t er = E_OK;
	const void *data = 0;
	int rc;

	pkt.rewind() >> cmd >> seqno >> sock >> bytes;
	data = pkt.point();

	if ( bytes <= MAX_IO_BYTES ) {
		do	{
			rc = ::write(sock,data,bytes);
		} while ( rc == -1 && errno == EINTR );
		
		if ( rc == -1 ) {
			switch ( errno ) {
			case EBADF :
				er = E_EBADF;
				break;
			case EIO :
			default :
				er = E_EIO;
			}
			bytes = 0;
		} else	{
			bytes = rc;
		}
	} else	{
		er = E_EINVAL;
		bytes = 0;
	}

printf("Got write(sock=%d) returns %d (bytes = %u)\n",sock,er,bytes);

	pkt.reset() << cmd << seqno << er << bytes;
	pkt.close();
	send(pkt);
}

void
Protocol::receiver() {

	listen();

	uint8_t buf[MAX_IO_BYTES+32];
	unsigned retlen = 0;

	for (;;) {
		retlen = protocol.SlipSer::read(buf,sizeof buf);
		if ( retlen < 1 )
			continue;

		Packet pkt(buf,sizeof buf);
		pkt.set_size(retlen);

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
		case C_Read :
			read(pkt);
			break;
		case C_Write :
			write(pkt);
			break;
		case C_Close :
			close(pkt);
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
	SlipSer::write(pkt.data(),pkt.size());
}

// End protocol.cpp
