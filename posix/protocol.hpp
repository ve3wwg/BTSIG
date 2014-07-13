//////////////////////////////////////////////////////////////////////
// protocol.hpp -- Protocol definitions
// Date: Wed Jul  9 20:40:05 2014   (C) datablocks.net
///////////////////////////////////////////////////////////////////////

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "enums.hpp"
#include "iface.hpp"
#include "slipser.hpp"
#include "packet.hpp"

class Protocol : public SlipSer {
	Ifaces		iface;	// Interface information

	bool make_ipv4_addr(sockaddr_in& sockaddr,unsigned port,const char *addr);

	void enumerate();
	void socket(Packet& pkt);
	void connect(Packet& pkt);
	void read(Packet& pkt);
	void write(Packet& pkt);
	void close(Packet& pkt);

public:	Protocol();

	void receiver();

	void send(const Packet& pkt);

	static void recv();	
	static void xmit();
};


#endif // PROTOCOL_HPP

// End protocol.hpp
