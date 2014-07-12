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

public:	Protocol();

	void receiver();
	void enumerate();
	void socket(Packet& pkt);

	void send(const Packet& pkt);

	static void recv();	
	static void xmit();
};


#endif // PROTOCOL_HPP

// End protocol.hpp
