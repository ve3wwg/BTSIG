//////////////////////////////////////////////////////////////////////
// iface.hpp -- Get information about Internet Interfaces
// Date: Tue Jul  8 20:13:25 2014   (C) Warren Gay ve3wwg
///////////////////////////////////////////////////////////////////////

#ifndef IFACE_HPP
#define IFACE_HPP

#include <memory.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>

#ifdef __linux__
#include <linux/netdevice.h>
#endif
#include <ifaddrs.h>

#include "enums.hpp"

#include <vector>
#include <unordered_map>
#include <string>


class Ifaces {
	typedef std::vector<std::string> ifvec_t;

	std::unordered_map<int,ifvec_t> interfaces;	// protocols_e -> ifset_t

public:	Ifaces();

	unsigned size(protocols_e p) const;
	const std::string& get(protocols_e p,unsigned x) const;
};

#endif // IFACE_HPP

// End iface.hpp
