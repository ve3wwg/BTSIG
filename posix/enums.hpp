//////////////////////////////////////////////////////////////////////
// enums.hpp -- Enumerated constants
// Date: Wed Jul  9 20:40:05 2014   (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#ifndef ENUMS_HPP
#define ENUMS_HPP

#ifdef SIMULATE
#include <time.h>
#else
typedef unsigned long time_t;
#endif

enum protocols_e {
	P_None = 0,		// Also could mean "any"
	P_IPv4 = 1,		// IPv4
	P_IPv6			// IPv6
};

enum command_e {
	C_None = 0,			// Reserved
	C_Enumerate,			// Enumerate interfaces
	C_Socket,			// socket(2)
	R_Enumerate_IPv4 = 0x81,	// IPv4 Interface enumerate response
	R_Enumerate_IPv6,		// IPv6 Interface enumerate response
	R_Enumerate_End,		// End of enumeration
};

enum com_domain_e {
	CD_AF_INET = 8,			// IPv4
	CD_AF_INET6,			// IPv6
};

enum sock_type_e {
	ST_SOCK_STREAM = 20,		// Provides sequenced, reliable, two-way, connection-based byte streams.
	ST_SOCK_DGRAM,			// Supports datagrams
};

//////////////////////////////////////////////////////////////////////
// Generic errno values
//////////////////////////////////////////////////////////////////////

#define	E_EPERM		 1	/* Operation not permitted */
#define	E_ENOENT	 2	/* No such file or directory */
#define	E_ESRCH		 3	/* No such process */
#define	E_EINTR		 4	/* Interrupted system call */
#define	E_EIO		 5	/* I/O error */
#define	E_ENXIO		 6	/* No such device or address */
#define	E_E2BIG		 7	/* Argument list too long */
#define	E_ENOEXEC	 8	/* Exec format error */
#define	E_EBADF		 9	/* Bad file number */
#define	E_ECHILD	10	/* No child processes */
#define	E_EAGAIN	11	/* Try again */
#define	E_ENOMEM	12	/* Out of memory */
#define	E_EACCES	13	/* Permission denied */
#define	E_EFAULT	14	/* Bad address */
#define	E_ENOTBLK	15	/* Block device required */
#define	E_EBUSY		16	/* Device or resource busy */
#define	E_EEXIST	17	/* File exists */
#define	E_EXDEV		18	/* Cross-device link */
#define	E_ENODEV	19	/* No such device */
#define	E_ENOTDIR	20	/* Not a directory */
#define	E_EISDIR	21	/* Is a directory */
#define	E_EINVAL	22	/* Invalid argument */
#define	E_ENFILE	23	/* File table overflow */
#define	E_EMFILE	24	/* Too many open files */
#define	E_ENOTTY	25	/* Not a typewriter */
#define	E_ETXTBSY	26	/* Text file busy */
#define	E_EFBIG		27	/* File too large */
#define	E_ENOSPC	28	/* No space left on device */
#define	E_ESPIPE	29	/* Illegal seek */
#define	E_EROFS		30	/* Read-only file system */
#define	E_EMLINK	31	/* Too many links */
#define	E_EPIPE		32	/* Broken pipe */
#define	E_EDOM		33	/* Math argument out of domain of func */
#define	E_ERANGE	34	/* Math result not representable */

#endif // ENUMS_HPP

// End enums.hpp
