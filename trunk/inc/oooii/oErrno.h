/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// Wrapp the typical errno.h and include additional standard posix style errnos
// that aren't in all compilers as well as define a more robust last error that
// can include a more specific string description of the error.
#pragma once
#ifndef oErrno_h
#define oErrno_h

#include <stdarg.h>

// Converts the errno into a string version of the define
const char* oGetErrnoString(int _Errno);

// converts the errno into a more human-readable descriptive text
const char* oGetErrnoDesc(int _Errno);

// Sets a thread-specific value as the last error that occured via execution.
// The oooii lib pattern is a function will return true for success and false
// for failure. If the function returns false, the calling code should use 
// oGetLastError() to get more information. When writing functions, use these
// oSetLastError() functions to set that value.
bool oSetLastErrorV(int _Error, const char* _Format, va_list _Args);
bool oSetLastError(int _Error, const char* _Format, ...);

// Sets a default description (same as returned by oGetErrnoDesc()
inline bool oSetLastError(int _Error) { return oSetLastError(_Error, 0); }

// Returns the last error set by oSetLastError() or the system
int oGetLastError();
size_t oGetLastErrorCount();

// returns more descriptive text set by oSetLastError()
const char* oGetLastErrorDesc();

#ifdef _MSC_VER
#include <errno.h>

// copy-pasted from http://www.finalcog.com/c-error-codes-include-errno
// @oooii-tony: Values that conflict with MS's errno.h are prepended with 100.
// Revisit this when additional OS support can be tested.
//#define	EDEADLK		35	/* Resource deadlock would occur */
//#define	ENAMETOOLONG	36	/* File name too long */
//#define	ENOLCK		37	/* No record locks available */
//#define	ENOSYS		38	/* Function not implemented */
//#define	ENOTEMPTY	39	/* Directory not empty */
#define	ECHRNG			44		/* Channel number out of range */
#define	EL2NSYNC		45		/* Level 2 not synchronized */
#define	EL3HLT			46		/* Level 3 halted */
#define	EL3RST			47		/* Level 3 reset */
#define	ELNRNG			48		/* Link number out of range */
#define	EUNATCH			49		/* Protocol driver not attached */
#define	ENOCSI			50		/* No CSI structure available */
#define	EL2HLT			51		/* Level 2 halted */
#define	EBADE			52		/* Invalid exchange */
#define	EBADR			53		/* Invalid request descriptor */
#define	EXFULL			54		/* Exchange full */
#define	ENOANO			55		/* No anode */
#define	EBADRQC			56		/* Invalid request code */
#define	EBADSLT			57		/* Invalid slot */
#define	EDEADLOCK		EDEADLK
#define	EBFONT			59		/* Bad font file format */
#define	ENONET			64		/* Machine is not on the network */
#define	ENOPKG			65		/* Package not installed */
#define	EREMOTE			66		/* Object is remote */
#define	EADV			68		/* Advertise error */
#define	ESRMNT			69		/* Srmount error */
#define	ECOMM			70		/* Communication error on send */
#define	EMULTIHOP		72		/* Multihop attempted */
#define	EDOTDOT			73		/* RFS specific error */
#define	ENOTUNIQ		76		/* Name not unique on network */
#define	EBADFD			77		/* File descriptor in bad state */
#define	EREMCHG			78		/* Remote address changed */
#define	ELIBACC			79		/* Can not access a needed shared library */
#define	ELIBBAD			10080	/* Accessing a corrupted shared library */
#define	ELIBSCN			81		/* .lib section in a.out corrupted */
#define	ELIBMAX			82		/* Attempting to link in too many shared libraries */
#define	ELIBEXEC		83		/* Cannot exec a shared library directly */
//#define	EILSEQ		84		/* Illegal byte sequence */
#define	ERESTART		85		/* Interrupted system call should be restarted */
#define	ESTRPIPE		86		/* Streams pipe error */
#define	EUSERS			87		/* Too many users */
#define	ESOCKTNOSUPPORT	94		/* Socket type not supported */
#define	EPFNOSUPPORT	96		/* Protocol family not supported */
#define	ESHUTDOWN		100108	/* Cannot send after transport endpoint shutdown */
#define	ETOOMANYREFS	100109	/* Too many references: cannot splice */
#define	EHOSTDOWN		100112	/* Host is down */
#define	ESTALE			100116	/* Stale NFS file handle */
#define	EUCLEAN			100117	/* Structure needs cleaning */
#define	ENOTNAM			100118	/* Not a XENIX named type file */
#define	ENAVAIL			100119	/* No XENIX semaphores available */
#define	EISNAM			100120	/* Is a named type file */
#define	EREMOTEIO		100121	/* Remote I/O error */
#define	EDQUOT			100122	/* Quota exceeded */
#define	ENOMEDIUM		100123	/* No medium found */
#define	EMEDIUMTYPE		100124	/* Wrong medium type */
#define	ENOKEY			100126	/* Required key not available */
#define	EKEYEXPIRED		100127	/* Key has expired */
#define	EKEYREVOKED		100128	/* Key has been revoked */
#define	EKEYREJECTED	100129	/* Key was rejected by service */
 
#if(_MSC_VER < 1600)

#define	ELOOP			10040	/* Too many symbolic links encountered */
#define	EWOULDBLOCK		EAGAIN	/* Operation would block */
#define	ENOMSG			10042	/* No message of desired type */
#define	EIDRM			43		/* Identifier removed */
#define	ENOSTR			60		/* Device not a stream */
#define	ENODATA			61		/* No data available */
#define	ETIME			62		/* Timer expired */
#define	ENOSR			63		/* Out of streams resources */
#define	ENOLINK			67		/* Link has been severed */
#define	EPROTO			71		/* Protocol error */
#define	EBADMSG			74		/* Not a data message */
#define	EOVERFLOW		75		/* Value too large for defined data type */
#define	ENOTSOCK		88		/* Socket operation on non-socket */
#define	EDESTADDRREQ	89		/* Destination address required */
#define	EMSGSIZE		90		/* Message too long */
#define	EPROTOTYPE		91		/* Protocol wrong type for socket */
#define	ENOPROTOOPT		92		/* Protocol not available */
#define	EPROTONOSUPPORT	93		/* Protocol not supported */
#define	EOPNOTSUPP		10095	/* Operation not supported on transport endpoint */
#define	EAFNOSUPPORT	97		/* Address family not supported by protocol */
#define	EADDRINUSE		98		/* Address already in use */
#define	EADDRNOTAVAIL	99		/* Cannot assign requested address */
#define	ENETDOWN		100		/* Network is down */
#define	ENETUNREACH		101		/* Network is unreachable */
#define	ENETRESET		102		/* Network dropped connection because of reset */
#define	ECONNABORTED	103		/* Software caused connection abort */
#define	ECONNRESET		104		/* Connection reset by peer */
#define	ENOBUFS			105		/* No buffer space available */
#define	EISCONN			106		/* Transport endpoint is already connected */
#define	ENOTCONN		107		/* Transport endpoint is not connected */
#define	ETIMEDOUT		110		/* Connection timed out */
#define	ECONNREFUSED	111		/* Connection refused */
#define	EHOSTUNREACH	113		/* No route to host */
#define	EALREADY		114		/* Operation already in progress */
#define	EINPROGRESS		115		/* Operation now in progress */
#define	ECANCELED		125		/* Operation Canceled */

/* for robust mutexes */
#define	EOWNERDEAD		130		/* Owner died */
#define	ENOTRECOVERABLE	131		/* State not recoverable */

#endif

#else
	#error Unsupported platform (oErrno.h)
#endif
#endif