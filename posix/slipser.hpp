//////////////////////////////////////////////////////////////////////
// slipser.hpp -- Simulated Bluetooth Serial with SLIP
// Date: Sat Jul  5 18:47:58 2014   (C) Warren Gay ve3wwg
///////////////////////////////////////////////////////////////////////

#ifndef SLIPSER_HPP
#define SLIPSER_HPP

#include "slip.hpp"


class SlipSer : public SLIP {
	int		sock;		// Socket
	bool		is_listen;	// true if this is the listen socket

public:	SlipSer();
	~SlipSer();

	void listen();
	void connect();

	void poll_read();
	void poll_write();

	unsigned read(void *data,unsigned n);
	void write(const void *data,unsigned n);

	virtual void yield() { };

	static uint8_t readb(void *arg);
	static void writeb(uint8_t b,void *arg);
	static void flush(void *arg);
};


#endif // SLIPSER_HPP

// End slipser.hpp
