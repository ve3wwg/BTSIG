BTSIG
=====

BlueTooth Serial Internet Gateway (for Teensy-3.1)

The Teensy-3.1 device has 64k RAM and 256k of flash memory, and runs at
96 Mhz.  This makes the device a very capable AVR class device. At the
same time, a Bluetooth serial device with a TTL UART interface can be
attached to provide serial connectivity to a POSIX host (like Linux,
FreeBSD or Mac OS X). Serial Bluetooth devices are quite economical and
uncomplicated.

PURPOSE:
--------

The purpose of this project is to provide Internet connectivity to the
Teensy, involving the minimum of footprint in RAM and flash memory. The
"server" runs on the POSIX gateway host at the remote end of the 
Bluetooth serial link. Any memory required by the TCP/IP stack, remains
on the gateway host side (i.e. not on the Teensy!) 

The Teensy on the other hand, still enjoys connectivity through the
serial BT link. The BTSIG nucleus  is used to provide the illusion of
having a TCP/IP stack on the AVR class host. The job of this nucleus is
to coordinate the sending and receiving of packets to the gateway
server. The nucleus also provides error  recovery and retry, should a
serial link error occur or packets be lost at the server end. Currently
the nucleus consists of two coroutines: one handles transmission of
packets while the other handles receiving.

Client coroutines on the Teensy can make requests of the nucleus, like
the creation of a socket (with a socket(2) call). The nucleus forms the
necessary request packet, sends it and queues up the request for the
expected response. Upon reception of  the response, the request returns
control back to the calling client process. The caller blocks until the
request succeeds or fails. This keeps the Teensy calling code simple.

The nucleus supports multiple simultaneous coroutine calls. Each is
queued and handled as soon as the response for the request is received.
Since the communication link is serial, where byte sequencing is
guaranteed, each request  is received by the BTSIG gateway server in a
first come first served basis. The POSIX server design can include true
pre-emptive threading, which leads to the possibility that some requests
will receive replies ahead of others. The BTSIG nucleus is designed to
address this, servicing replies as they arrive.

WHY NOT BNEP?
-------------

One may ask, why not use or develop the BNEP protocol (Bluetooth Network
Encapsulation Protocol) on the Teensy?  Why develop something new?  The
short answer is that a BNEP implementation on the Teensy is likely to be
"fat" in its overall footprint. Secondly, BNEP provides an interface at
the Teensy end for a TCP/IP stack. The TCP/IP stack would then also add
to the overall resources needed.

Essentially, the BNEP approach amounts to this:

   TCP/IP <-> BNEP <----//----> BNEP <-> TCP/IP
   
The BNEP+TCP/IP stack is needed at both ends. Needing this in the AVR
class device is a true disadvantage.

The BTSIG approach shifts the resources required from the Teensy end to
the POSIX gateway server end (where ample resources exist). The
"interface" is largely accomodated at the gateway end. The Teensy end
only needs the management of the sending and receiving of requests to be
programmed for.

  TCP/IP <-> BTSIG_Gateway <----//----> BTSIG_Nucleus

Here we put the burden on the BTSIG Gateway, while keeping the BTSIG
Nucleus as small and simple as we dare.

IMPLEMENTATION:
---------------

This project brings Internet connectivity to the Teensy-3.x device
through the following software components:

    1. The Fibers package for Teensy coroutine support.
       (https://github.com/ve3wwg/teensy3_fibers)

    2. The SLIP+CRC8 package for providing packet capability 
       over a serial link:
       (https://github.com/ve3wwg/slip)

    3. This package (BTSIG)

To develop the protocol, a simulation is provided for POSIX systems, so
that both the Teensy nucleus and the gateway server components can be
tested prior to implementation on the actual hardware. The simulation
programs are xserver and xclient.

Since the Teensy nucleus runs on one CPU using coroutines provided by
the Fibers class, a POSIX implementation was needed for testing
purposes. A POSIX version of the Fibers class was developed to implement
coroutines in a portable way. The coroutine support was implemented on
top of pre-emptive pthreads, using mutexes. The mutexes force only one
thread to run at a given point in time (this  is a perverse way to use
pthreads, but it works). Context switches occur through the use of the
Fibers::yield() call, which manipulates the coroutine mutexes.

The Bluetooth serial connection is simulated by use of a AF_UNIX socket
between the xserver and xclient processes. The xserver program acts as a
test bed for the POSIX BTSIG gateway server components. The xclient
program tests the BTSIG nucleus that runs on the Teensy side.

--
