//////////////////////////////////////////////////////////////////////
// xclient.cpp -- Simulated Bluetooth Client
// Date: Sat Jul  5 19:02:28 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "btsig.hpp"

int
main(int argc,char **argv) {
	unsigned ifaces = 0;

	puts("xclient started:");
	
	ifaces = btsig.get_interfaces();

	printf("There are %u interfaces:\n",ifaces);

	for ( unsigned ux=0; ux < ifaces; ++ux ) {
		protocols_e protocol = P_None;
		const char *addr = 0;

		if ( btsig.get_interface(ux,&addr,protocol) == ~0u )
			break;

		printf(" iface # %u : %s protocol %d\n",
			ux,
			addr,
			protocol);
	}

	int sock = BTSIG::socket(CD_AF_INET,ST_SOCK_STREAM,0);
	printf("Got sock = %d back from socket()\n",sock);

	if ( sock >= 0 ) {
		int rc;

		rc = BTSIG::connect(sock,80,"google.com");
		printf("Got rc = %d from connect() request\n",rc);

		rc = BTSIG::write(sock,"GET /\r\n",7);
		printf("Got rc = %d from write()\n",rc);

		char buf[1000];
		rc = BTSIG::read(sock,buf,sizeof buf);
		printf("Got rc = %d from read()\n",rc);

		if ( rc > 0 )
			printf("  buf[]='%80.80s...';\n",buf);

		rc = BTSIG::close(sock);
		printf("BTSIG::close(%d) returned %d;\n",sock,rc);
	}

	BTSIG::terminate();

	puts("xclient ended.");
	return 0;
}

// End xclient.cpp

