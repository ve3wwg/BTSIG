//////////////////////////////////////////////////////////////////////
// xpacket.cpp -- Test bed for Packet class
// Date: Sun Jul 13 14:23:10 2014
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "packet.hpp"

int
main() {
	char buf[64];

	Packet p1(buf,sizeof buf);

	std::string tstr = "Hello net!";

	p1 << 23 << 98 << "abc" << (short)29 << tstr << ';' << 23.5 << static_cast<uint32_t>(6009u) << uint16_t(1000) << uint32_t(12905)
		<< int16_t(-1234) << int32_t(-54321);
	p1.close();

	int a, b;
	short c;
	char s1[32], c2;
	double d;
	uint32_t u, f;
	uint16_t e;
	int16_t g;
	int32_t h;

	std::string rstr;

	p1 >> a >> b;

	{
		const char *s = p1.c_str();
		assert(!strcmp(s,"abc"));
	}

	p1 >> s1 >> c >> rstr >> c2 >> d >> u >> e >> f >> g >> h;

	assert(a == 23);
	assert(b == 98);
	assert(!strcmp(s1,"abc"));
	assert(c == 29);
	assert(tstr == "Hello net!");
	assert(c2 == ';');
	assert(d == 23.5);
	assert(u == 6009u);
	assert(e == 1000u);
	assert(f == 12905u);
	assert(g == -1234);
	assert(h == -54321);

	return 0;
}

// End xpacket.cpp
