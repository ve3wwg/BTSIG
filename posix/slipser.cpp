//////////////////////////////////////////////////////////////////////
// slipser.cpp -- Simulated Bluetooth Serial with SLIP
// Date: Sat Jul  5 18:49:56 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/poll.h>

#include "slipser.hpp"


SlipSer::SlipSer() : SLIP(SlipSer::readb,SlipSer::writeb,(void *)this,SlipSer::flush) {
	sock = -1;
	enable_crc8(true);
}

SlipSer::~SlipSer() {
	close(sock);
	sock = -1;
	if ( is_listen )
		unlink("./sock");
}

void
SlipSer::listen() {
	int rc;
	sockaddr_un addr, from;
	socklen_t from_len;

	sock = socket(AF_UNIX,SOCK_STREAM,0);
	assert(sock >= 0);

	memset(&addr,0,sizeof addr);
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path,"./sock",sizeof(addr.sun_path) - 1);

	unlink("./sock");

	rc = bind(sock,(const sockaddr *)(&addr),sizeof addr);
	assert(!rc);

	is_listen = true;

	rc = ::listen(sock,1);
	assert(!rc);

	from_len = sizeof from;
	rc = ::accept(sock,(sockaddr *)(&from),&from_len);
	assert(rc >= 0);

	::close(sock);

	sock = rc;
}

void
SlipSer::connect() {

	int rc;
	sockaddr_un addr;
	
	sock = socket(AF_UNIX,SOCK_STREAM,0);
	assert(sock >= 0);

	memset(&addr,0,sizeof addr);
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path,"./sock",sizeof(addr.sun_path) - 1);

	rc = ::connect(sock,(const sockaddr *)(&addr),sizeof addr);
	assert(!rc);
}

void
SlipSer::write(const void *data,unsigned n) {
	SLIP::write(data,n);
}

unsigned
SlipSer::read(void *data,unsigned n) {
	SLIP::Status s;
	unsigned retlen = 0;

	do	{
		s = SLIP::read(data,n,retlen);
	} while ( s != SLIP::ES_Ok );

	return retlen;
}

void
SlipSer::poll_read() {
	struct pollfd polls;
	int rc;

	do	{
		do	{
			polls.fd = sock;
			polls.events = POLLIN | POLLPRI;
			polls.revents = 0;

			rc = ::poll(&polls,1,1);
		} while ( rc == -1 && errno == EINTR );
		if ( rc < 1 )
			yield();
	} while ( !rc || !(polls.revents & POLLIN) );
}

void
SlipSer::poll_write() {
	struct pollfd polls;
	int rc;

	polls.fd = sock;
	polls.events = POLLOUT;

	do	{
		do	{
			rc = poll(&polls,1,0);
		} while ( rc == -1 && errno == EINTR );
		if ( rc < 1 )
			yield();
	} while ( !rc || !(polls.revents & POLLOUT) );
}

uint8_t
SlipSer::readb(void *arg) {
	SlipSer *obj = (SlipSer *)arg;
	uint8_t b;
	int rc;
	
	do	{
		obj->poll_read();
		rc = ::read(obj->sock,&b,1);
	} while ( rc == -1 && errno == EINTR );

	if ( !rc ) {
		puts("*** Socket disconnect ***");
		exit(0);
	}

	assert(rc == 1);

	return b;
}

void
SlipSer::writeb(uint8_t b,void *arg) {
	SlipSer *obj = (SlipSer *)arg;
	int rc;

	do	{
		obj->poll_write();
		rc = ::write(obj->sock,&b,1);
	} while ( rc == -1 && errno == EINTR );
}

void
SlipSer::flush(void *arg) {
	SlipSer *obj = (SlipSer *)arg;

	obj->yield();
}

// End slipser.cpp
