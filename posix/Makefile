# TFFFT Makefile
# Warren Gay ve3wwg
# Sat Jul  5 07:58:06 2014

include Makefile.conf

CXX_FILES 	:= $(wildcard *.cpp)
OBJS 		:= $(CXX_FILES:.cpp=.o) 

all:	slip.o xclient xserver

xclient: xclient.o mutex.o slipser.o btsig.o slip.o packet.o
	$(CXX) xclient.o mutex.o slipser.o btsig.o slip.o packet.o -o xclient $(LDFLAGS)

xserver: xserver.o mutex.o slipser.o iface.o protocol.o slip.o packet.o 
	$(CXX) xserver.o mutex.o slipser.o iface.o protocol.o slip.o packet.o -o xserver $(LDFLAGS)

pkt:	packet.o
	$(CXX) packet.o -o pkt

slip.cpp: ../common/slip/slip.cpp
	ln -s ../common/slip/slip.cpp slip.cpp

slip.hpp: ../common/slip/slip.hpp
	ln -s ../common/slip/slip.hpp slip.hpp

crc8.cpp: ../common/slip/crc8.cpp
	ln -s ../common/slip/crc8.cpp crc8.cpp

slip.o:	slip.cpp slip.hpp crc8.cpp

btsigserv: slipser.hpp
xclient: slipser.hpp
packet.o: packet.hpp

xfibers: $(OBJS)
	$(CXX) $(OBJS) -o xfibers $(LDFLAGS)

xfibers: fibers.hpp

clean:
	rm -f *.o

distclean: clean
	rm -f errs.t slip.cpp slip.hpp crc8.cpp
	rm -f sock xserver xclient pkt *.core core.*

clobber: distclean

backup: distclean
	(cd .. && tar czvf TFFFT.tar.gz TFFFT)	

# End Makefile
