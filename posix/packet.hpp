//////////////////////////////////////////////////////////////////////
// packet.hpp -- Packet Data Marshaller Class
// Date: Fri Jul 11 19:23:59 2014   (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#ifndef PACKET_HPP
#define PACKET_HPP

#include <stdint.h>

#ifdef USE_STDSTR
#include <string>
#endif

#ifdef __DBL_DIG__
#define USE_DBL_DIG __DBL_DIG__
#else
#define USE_DBL_DIG 15
#endif

class Packet {
	uint8_t		*buffer;		// Pointer to buffer
	unsigned	buflen;			// Buffer's active length for reading, allocated length for output
	unsigned	maxlen;			// Physical size of the buffer
	unsigned	_offset;		// Offset into the buffer

public:	Packet(void *buffer,size_t buflen);

	void rewind();				// Rewind offset, without changing buflen
	void reset();				// Rewind offset, set buflen = maxlen

	inline const void *data() const		{ return buffer; }
	inline unsigned offset() const 		{ return _offset; }
	inline unsigned size() const		{ return buflen; }

	inline void set_size(unsigned sz) 	{ buflen = sz; }

	void seek(unsigned new_offset);

	bool copy(const Packet& pkt);		// Copy from pkt to this object (return true if not truncated)

	Packet& operator<<(uint8_t uch);
	Packet& operator<<(char ch);
	Packet& operator<<(const char *str);
	Packet& operator<<(uint16_t u);
	Packet& operator<<(uint32_t u);
	Packet& operator<<(int16_t i);
	Packet& operator<<(int32_t i);
	Packet& operator<<(double f);

	Packet& operator>>(uint8_t &uch);
	Packet& operator>>(char &ch);
	Packet& operator>>(char *str);
	Packet& operator>>(uint16_t& u);
	Packet& operator>>(uint32_t& u);
	Packet& operator>>(int16_t& i);
	Packet& operator>>(int32_t& i);
	Packet& operator>>(double& f);

#ifdef USE_STDSTR
	Packet& operator<<(const std::string& str);
	Packet& operator>>(std::string& str);
#endif

	const char *c_str();
};

#endif // PACKET_HPP

// End packet.hpp
