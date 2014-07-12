//////////////////////////////////////////////////////////////////////
// xserver.cpp -- Test Bed for POSIX BTSIG Server
// Date: Sat Jul  5 19:17:41 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "protocol.hpp"

int
main(int argc,char **argv) {
	Ifaces ifaces;

	printf("%u IPv4 Interfaces\n",ifaces.size(P_IPv4));

	for ( unsigned ux = 0; ux < ifaces.size(P_IPv4); ++ux ) {
		const std::string& ipv4 = ifaces.get(P_IPv4,ux);
			
		printf("  ipv4  %s\n",ipv4.c_str());
	}

	printf("%u IPv6 Interfaces\n",ifaces.size(P_IPv6));

	for ( unsigned ux = 0; ux < ifaces.size(P_IPv6); ++ux ) {
		const std::string& ipv6 = ifaces.get(P_IPv6,ux);
			
		printf("  ipv6  %s\n",ipv6.c_str());
	}

	puts("xserver listening:");

	Protocol::recv();

	puts("xserver ended.");
	return 0;
}

// End xserver.cpp
