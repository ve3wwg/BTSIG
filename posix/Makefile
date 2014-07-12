# TFFFT Makefile
# Warren Gay ve3wwg
# Sat Jul  5 07:58:06 2014

include Makefile.conf

CXX_FILES 	:= $(wildcard *.cpp)
OBJS 		:= $(CXX_FILES:.cpp=.o) 

all:	xclient xserver

xclient: xclient.o mutex.o slipser.o btsig.o slip/slip.o packet.o
	$(CXX) xclient.o mutex.o slipser.o btsig.o slip/slip.o packet.o -o xclient $(LDFLAGS)

xserver: xserver.o mutex.o slipser.o iface.o protocol.o slip/slip.o packet.o 
	$(CXX) xserver.o mutex.o slipser.o iface.o protocol.o slip/slip.o packet.o -o xserver $(LDFLAGS)

pkt:	packet.o
	$(CXX) packet.o -o pkt

slip/slip.o:
	$(MAKE) -$(MAKEFLAGS) -C slip -f Makefile.posix

btsigserv: slipser.hpp
xclient: slipser.hpp
packet.o: packet.hpp

xfibers: $(OBJS)
	$(CXX) $(OBJS) -o xfibers $(LDFLAGS)

xfibers: fibers.hpp

clean:
	rm -f *.o

distclean: clean
	rm -f errs.t
	rm -f sock xserver xclient pkt *.core core.*
	$(MAKE) -C slip -f Makefile.posix distclean

clobber: distclean

backup: distclean
	(cd .. && tar czvf TFFFT.tar.gz TFFFT)	

# End Makefile
