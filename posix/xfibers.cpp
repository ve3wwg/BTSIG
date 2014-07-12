//////////////////////////////////////////////////////////////////////
// xfibers.cpp -- Test the Framework (main program)
// Date: Sat Jul  5 16:03:05 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "fibers.hpp"

static Fibers<16> fibers;

static void
foo(void *arg) {

	puts("foo() started..");

	for ( int x=0; x < 8; ++x ) {
		printf("foo calling yield(), x = %d\n",x);
		fibers.yield();
	}
	puts("foo() returning..");
}

static void
bar(void *arg) {

	puts("bar() started..");

	for ( int x=0; x < 4; ++x ) {
		printf("bar calling yield(), x = %d\n",x);
		fibers.yield();
	}
	puts("bar() returning..");
}

int
main(int argc,char **argv) {
	puts("xfibers started:");

	for ( int x=0; x<4; ++x ) {
		fibers.yield();
		puts("alone main.yield()");
	}

	unsigned foox = fibers.create(foo,0,8192);
	assert(foox > 0);
	puts("foo running..");

	unsigned barx = fibers.create(bar,0,8192);
	assert(barx > foox);
	puts("bar running..");

	for ( int x=0; x<8; ++x ) {
		fibers.yield();
		printf("main calling yield(), x = %d\n",x);
	}

	printf("main joining with foo() foox=%u\n",foox);
	fibers.join(foox);
	fibers.join(barx);

	puts("xfibers ended.");
	return 0;
}

// End xfibers.cpp
