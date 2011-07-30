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
#include <oooii/oErrno.h>
#include <oooii/oAssert.h>
#include <oooii/oSingleton.h>

struct oLastErrorContext : public oProcessThreadlocalSingleton<oLastErrorContext>
{
	oLastErrorContext()
		: Count(0)
	{
		*Desc = 0;
	}

	char Desc[2048];
	size_t Count;
};

bool oSetLastErrorV(int _Error, const char* _Format, va_list _Args)
{
	_set_errno(_Error);
	if (!_Format)
		_Format = oGetErrnoDesc(_Error);
	oASSERT(_Format, "A description must be used with oSetLastError for error code %d", _Error);
	return vsprintf_s(oLastErrorContext::Singleton()->Desc, _Format, _Args) >= 0;
}

bool oSetLastError(int _Error, const char* _Format, ...)
{
	++oLastErrorContext::Singleton()->Count;
	va_list args;
	va_start(args, _Format);
	return oSetLastErrorV(_Error, _Format, args);
}

size_t oGetLastErrorCount()
{
	return oLastErrorContext::Singleton()->Count;
}

int oGetLastError()
{
	int err = 0;
	_get_errno(&err);
	return err;
}

const char* oGetLastErrorDesc()
{
	const char* desc = oLastErrorContext::Singleton()->Desc;
	return *desc ? desc : oGetErrnoDesc(oGetLastError());
}

const char* oGetErrnoString(int _Errno)
{
	switch (_Errno)
	{
		case 0: return "OK";
		case E2BIG: return "E2BIG";
		case EACCES: return "EACCES";
		case EADDRINUSE: return "EADDRINUSE";
		case EADDRNOTAVAIL: return "EADDRNOTAVAIL";
		case EAFNOSUPPORT: return "EAFNOSUPPORT";
		case EAGAIN: return "EAGAIN";
		case EALREADY: return "EALREADY";
		case EBADF: return "EBADF";
		case EBADMSG: return "EBADMSG";
		case EBUSY: return "EBUSY";
		case ECANCELED: return "ECANCELED";
		case EEOF: return "EEOF";
		case ECHILD: return "ECHILD";
		case ECONNABORTED: return "ECONNABORTED";
		case ECONNREFUSED: return "ECONNREFUSED";
		case ECONNRESET: return "ECONNRESET";
		case EDEADLK: return "EDEADLK";
		case EDESTADDRREQ: return "EDESTADDRREQ";
		case EDOM: return "EDOM";
		case EDQUOT: return "EDQUOT";
		case EEXIST: return "EEXIST";
		case EFAULT: return "EFAULT";
		case EFBIG: return "EFBIG";
		case EHOSTUNREACH: return "EHOSTUNREACH";
		case EIDRM: return "EIDRM";
		case EILSEQ: return "EILSEQ";
		case EINPROGRESS: return "EINPROGRESS";
		case EINTR: return "EINTR";
		case EINVAL: return "EINVAL";
		case EIO: return "EIO";
		case EISCONN: return "EISCONN";
		case EISDIR: return "EISDIR";
		case ELOOP: return "ELOOP";
		case EMFILE: return "EMFILE";
		case EMLINK: return "EMLINK";
		case EMSGSIZE: return "EMSGSIZE";
		case EMULTIHOP: return "EMULTIHOP";
		case ENAMETOOLONG: return "ENAMETOOLONG";
		case ENETDOWN: return "ENETDOWN";
		case ENETRESET: return "ENETRESET";
		case ENETUNREACH: return "ENETUNREACH";
		case ENFILE: return "ENFILE";
		case ENOBUFS: return "ENOBUFS";
		case ENODATA: return "ENODATA";
		case ENODEV: return "ENODEV";
		case ENOENT: return "ENOENT";
		case ENOEXEC: return "ENOEXEC";
		case ENOLCK: return "ENOLCK";
		case ENOLINK: return "ENOLINK";
		case ENOMEM: return "ENOMEM";
		case ENOMSG: return "ENOMSG";
		case ENOPROTOOPT: return "ENOPROTOOPT";
		case ENOSPC: return "ENOSPC";
		case ENOSR: return "ENOSR";
		case ENOSTR: return "ENOSTR";
		case ENOSYS: return "ENOSYS";
		case ENOTCONN: return "ENOTCONN";
		case ENOTDIR: return "ENOTDIR";
		case ENOTEMPTY: return "ENOTEMPTY";
		case ENOTSOCK: return "ENOTSOCK";
		case EOPNOTSUPP: return "EOPNOTSUPP";
		case ENOTTY: return "ENOTTY";
		case ENXIO: return "ENXIO";
		case EOVERFLOW: return "EOVERFLOW";
		case EPERM: return "EPERM";
		case EPIPE: return "EPIPE";
		case EPROTO: return "EPROTO";
		case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
		case EPROTOTYPE: return "EPROTOTYPE";
		case ERANGE: return "ERANGE";
		case EROFS: return "EROFS";
		case ESPIPE: return "ESPIPE";
		case ESRCH: return "ESRCH";
		case ESTALE: return "ESTALE";
		case ETIME: return "ETIME";
		case ETIMEDOUT: return "ETIMEDOUT";
		case EXDEV: return "EXDEV";
		case ECHRNG: return "ECHRNG";
		case EL2NSYNC: return "EL2NSYNC";
		case EL3HLT: return "EL3HLT";
		case EL3RST: return "EL3RST";
		case ELNRNG: return "ELNRNG";
		case EUNATCH: return "EUNATCH";
		case ENOCSI: return "ENOCSI";
		case EL2HLT: return "EL2HLT";
		case EBADE: return "EBADE";
		case EBADR: return "EBADR";
		case EXFULL: return "EXFULL";
		case ENOANO: return "ENOANO";
		case EBADRQC: return "EBADRQC";
		case EBADSLT: return "EBADSLT";
		case EBFONT: return "EBFONT";
		case ENONET: return "ENONET";
		case ENOPKG: return "ENOPKG";
		case EREMOTE: return "EREMOTE";
		case EADV: return "EADV";
		case ESRMNT: return "ESRMNT";
		case ECOMM: return "ECOMM";
		case EDOTDOT: return "EDOTDOT";
		case ENOTUNIQ: return "ENOTUNIQ";
		case EBADFD: return "EBADFD";
		case EREMCHG: return "EREMCHG";
		case ELIBACC: return "ELIBACC";
		case ELIBBAD: return "ELIBBAD";
		case ELIBSCN: return "ELIBSCN";
		case ELIBMAX: return "ELIBMAX";
		case ELIBEXEC: return "ELIBEXEC";
		case ERESTART: return "ERESTART";
		case ESTRPIPE: return "ESTRPIPE";
		case EUSERS: return "EUSERS";
		case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
		case EPFNOSUPPORT: return "EPFNOSUPPORT";
		case ESHUTDOWN: return "ESHUTDOWN";
		case ETOOMANYREFS: return "ETOOMANYREFS";
		case EHOSTDOWN: return "EHOSTDOWN";
		case EUCLEAN: return "EUCLEAN";
		case ENOTNAM: return "ENOTNAM";
		case ENAVAIL: return "ENAVAIL";
		case EISNAM: return "EISNAM";
		case EREMOTEIO: return "EREMOTEIO";
		case ENOMEDIUM: return "ENOMEDIUM";
		case EMEDIUMTYPE: return "EMEDIUMTYPE";
		case ENOKEY: return "ENOKEY";
		case EKEYEXPIRED: return "EKEYEXPIRED";
		case EKEYREVOKED: return "EKEYREVOKED";
		case EKEYREJECTED: return "EKEYREJECTED";
		case EOWNERDEAD: return "EOWNERDEAD";
		case ENOTRECOVERABLE: return "ENOTRECOVERABLE";
		default: break;
	}

	static oTHREADLOCAL char buf[64];
	sprintf_s(buf, "Unhandled errno_t %u", _Errno);
	return buf;
}

const char* oGetErrnoDesc(errno_t _Errno)
{
	switch (_Errno)
	{
		case 0: return "Success";
		case E2BIG: return "Argument list too long";
		case EACCES: return "Permission denied";
		case EADDRINUSE: return "Address in use";
		case EADDRNOTAVAIL: return "Address not available";
		case EAFNOSUPPORT: return "Address family not supported";
		case EAGAIN: return "Operation would block, or resource unavailable, try again";
		case EALREADY: return "Connection already in progress";
		case EBADF: return "Bad file descriptor";
		case EBADMSG: return "Bad message";
		case EBUSY: return "Device or resource busy";
		case ECANCELED: return "Operation canceled";
		case EEOF: return "End of file";
		case ECHILD: return "No child processes";
		case ECONNABORTED: return "Connection aborted";
		case ECONNREFUSED: return "Connection refused";
		case ECONNRESET: return "Connection reset";
		case EDEADLK: return "Resource deadlock would occur";
		case EDESTADDRREQ: return "Destination address required";
		case EDOM: return "Mathematics argument out of domain of function";
		case EDQUOT: return "Reserved";
		case EEXIST: return "File exists";
		case EFAULT: return "Bad address";
		case EFBIG: return "File too large";
		case EHOSTUNREACH: return "Host is unreachable";
		case EIDRM: return "Identifier removed";
		case EILSEQ: return "Illegal byte sequence";
		case EINPROGRESS: return "Operation in progress";
		case EINTR: return "Interrupted function";
		case EINVAL: return "Invalid argument";
		case EIO: return "I/O error";
		case EISCONN: return "Socket is connected";
		case EISDIR: return "Is a directory";
		case ELOOP: return "Too many levels of symbolic links";
		case EMFILE: return "Too many open files";
		case EMLINK: return "Too many links";
		case EMSGSIZE: return "Message too large";
		case EMULTIHOP: return "Reserved";
		case ENAMETOOLONG: return "Filename too long";
		case ENETDOWN: return "Network is down";
		case ENETRESET: return "Connection aborted by network";
		case ENETUNREACH: return "Network unreachable";
		case ENFILE: return "Too many files open in system";
		case ENOBUFS: return "No buffer space available";
		case ENODATA: return "No message is available on the STREAM head read queue";
		case ENODEV: return "No such device";
		case ENOENT: return "No such file or directory";
		case ENOEXEC: return "Executable file format error";
		case ENOLCK: return "No locks available";
		case ENOLINK: return "Reserved";
		case ENOMEM: return "Not enough space";
		case ENOMSG: return "No message of the desired type";
		case ENOPROTOOPT: return "Protocol not available";
		case ENOSPC: return "No space left on device";
		case ENOSR: return "No STREAM resources";
		case ENOSTR: return "Not a STREAM";
		case ENOSYS: return "Function not supported";
		case ENOTCONN: return "The socket is not connected";
		case ENOTDIR: return "Not a directory";
		case ENOTEMPTY: return "Directory not empty";
		case ENOTSOCK: return "Not a socket";
		case EOPNOTSUPP: return "Operation not supported on transport endpoint";
		case ENOTTY: return "Inappropriate I/O control operation";
		case ENXIO: return "No such device or address";
		case EOVERFLOW: return "Value too large to be stored in data type";
		case EPERM: return "Operation not permitted";
		case EPIPE: return "Broken pipe";
		case EPROTO: return "Protocol error";
		case EPROTONOSUPPORT: return "Protocol not supported";
		case EPROTOTYPE: return "Protocol wrong type for socket";
		case ERANGE: return "Result too large";
		case EROFS: return "Read-only file system";
		case ESPIPE: return "Invalid seek";
		case ESRCH: return "No such process";
		case ESTALE: return "Reserved";
		case ETIME: return "Stream ioctl() timeout";
		case ETIMEDOUT: return "Connection timed out";
		case EXDEV: return "Cross-device link";
		case ECHRNG: return "Channel number out of range";
		case EL2NSYNC: return "Level 2 not synchronized";
		case EL3HLT: return "Level 3 halted";
		case EL3RST: return "Level 3 reset";
		case ELNRNG: return "Link number out of range";
		case EUNATCH: return "Protocol driver not attached";
		case ENOCSI: return "No CSI structure available";
		case EL2HLT: return "Level 2 halted";
		case EBADE: return "Invalid exchange";
		case EBADR: return "Invalid request descriptor";
		case EXFULL: return "Exchange full";
		case ENOANO: return "No anode";
		case EBADRQC: return "Invalid request code";
		case EBADSLT: return "Invalid slot";
		case EBFONT: return "Bad font file format";
		case ENONET: return "Machine is not on the network";
		case ENOPKG: return "Package not installed";
		case EREMOTE: return "Object is remote";
		case EADV: return "Advertise error";
		case ESRMNT: return "Srmount error";
		case ECOMM: return "Communication error on send";
		case EDOTDOT: return "RFS specific error";
		case ENOTUNIQ: return "Name not unique on network";
		case EBADFD: return "File descriptor in bad state";
		case EREMCHG: return "Remote address changed";
		case ELIBACC: return "Can not access a needed shared library";
		case ELIBBAD: return "Accessing a corrupted shared library";
		case ELIBSCN: return ".lib section in a.out corrupted";
		case ELIBMAX: return "Attempting to link in too many shared libraries";
		case ELIBEXEC: return "Cannot exec a shared library directly";
		case ERESTART: return "Interrupted system call should be restarted";
		case ESTRPIPE: return "Streams pipe error";
		case EUSERS: return "Too many users";
		case ESOCKTNOSUPPORT: return "Socket type not supported";
		case EPFNOSUPPORT: return "Protocol family not supported";
		case ESHUTDOWN: return "Cannot send after transport endpoint shutdown";
		case ETOOMANYREFS: return "Too many references: cannot splice";
		case EHOSTDOWN: return "Host is down";
		case EUCLEAN: return "Structure needs cleaning";
		case ENOTNAM: return "Not a XENIX named type file";
		case ENAVAIL: return "No XENIX semaphores available";
		case EISNAM: return "Is a named type file";
		case EREMOTEIO: return "Remote I/O error";
		case ENOMEDIUM: return "No medium found";
		case EMEDIUMTYPE: return "Wrong medium type";
		case ENOKEY: return "Required key not available";
		case EKEYEXPIRED: return "Key has expired";
		case EKEYREVOKED: return "Key has been revoked";
		case EKEYREJECTED: return "Key was rejected by service";
		case EOWNERDEAD: return "Owner died";
		case ENOTRECOVERABLE: return "State not recoverable";
		default: break;
	}

	static oTHREADLOCAL char buf[64];
	sprintf_s(buf, "Undescribed errno_t %u", _Errno);
	return buf;
}