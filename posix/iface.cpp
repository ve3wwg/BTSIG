//////////////////////////////////////////////////////////////////////
// iface.cpp -- Get Internet Interfaces
// Date: Tue Jul  8 20:14:33 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>

#include "iface.hpp"


//////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////

Ifaces::Ifaces() {
        struct ifaddrs *if_addrs = 0;
        struct ifaddrs *if_addr = 0; 
        void *tmp = 0;
        char buf[INET6_ADDRSTRLEN];
        
        if ( !getifaddrs(&if_addrs) ) {
                for ( if_addr = if_addrs; if_addr != 0; if_addr = if_addr->ifa_next ) {
			if ( !if_addr->ifa_addr )
				continue;		// May be null for Linux

                        if ( if_addr->ifa_addr->sa_family == AF_INET ) {
                                tmp = &((struct sockaddr_in *)if_addr->ifa_addr)->sin_addr;
                        } else  {
                                tmp = &((struct sockaddr_in6 *)if_addr->ifa_addr)->sin6_addr;
                        }
                         
                        inet_ntop(if_addr->ifa_addr->sa_family,tmp,buf,sizeof buf);
                        
                        if ( if_addr->ifa_addr->sa_family == AF_INET )
				interfaces[P_IPv4].push_back(buf);
                        else if ( if_addr->ifa_addr->sa_family == AF_INET6 )
				interfaces[P_IPv6].push_back(buf);
                }
                freeifaddrs(if_addrs);
                if_addrs = 0;
        }
}

unsigned
Ifaces::size(protocols_e p) const {

	auto it = interfaces.find(p);
	if ( it == interfaces.end() )
		return 0;	// None

	return static_cast<uint16_t>((it->second).size());
}

const std::string&
Ifaces::get(protocols_e p,unsigned x) const {

	auto it = interfaces.find(p);
	assert(it != interfaces.end());

	const ifvec_t& ifvec = it->second;
	assert(x < ifvec.size());
	return ifvec.at(x);
}

// End iface.cpp
