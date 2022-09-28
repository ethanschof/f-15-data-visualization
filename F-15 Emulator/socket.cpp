// implementation of the CSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "socket.h"
#include <iostream>
#include <cmath>

// include file ws2tcpip.h is needed because:
// struct ip_mreq and identifiers IP_DROP_MEMBERSHIP and IP_MULTICAST_LOOP, among others ...
#include <ws2tcpip.h>

#include "outstream.h"

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

int CSocket::cInstance = 0;
WSADATA CSocket::wsaData;


static OutStream slog(clog, false);
static OutStream serr(cerr, false);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocket::CSocket()
{
	m_bBroadcast=false;
	bLogEnable = false;
	sLogPrompt = "CSocket: ";

	Initialize();
	InitializeWSA(); // initialize Windows CSocket API
}

CSocket::CSocket(int type, int protocol, bool bLogEnable, string sLogPrompt)
{
	m_bBroadcast=false;
	CSocket::bLogEnable = bLogEnable;
	CSocket::sLogPrompt = sLogPrompt;
	slog.Activate(bLogEnable);
	serr.Activate(bLogEnable);
	slog.SetPrompt(sLogPrompt.c_str());
	serr.SetPrompt(sLogPrompt.c_str());

	Initialize();
	InitializeWSA(); // initialize Windows CSocket API

	hSocket = socket(AF_INET, type, protocol);

	if (hSocket == INVALID_SOCKET)
	{
		HandleError();
		status = BAD;
	}
	else
	{
		slog << sLogPrompt << "CSocket #" << (int)hSocket << " created successfully!" << endl;
		status = OK;
	}

}

void CSocket::Initialize()
{
	status = NOT_INITIALIZED;
	hSocket = NULL;
	addrLocal = 0;
	szLastErrorDescription = "No Error";
	iLastErrorCode = 0;
}

CSocket::~CSocket()
{
	Close();

	if (--cInstance == 0)
		WSACleanup();
}

void CSocket::InitializeWSA()
{
	if (cInstance++ == 0)
	{
		WORD wVersionRequested = MAKEWORD( 2, 2 );

		int err = WSAStartup( wVersionRequested, &wsaData );
		if ( err != 0 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			return;
		}
 
		/* Confirm that the WinSock DLL supports 2.2.*/
		/* Note that if the DLL supports versions greater    */
		/* than 2.2 in addition to 2.2, it will still return */
		/* 2.2 in wVersion since that is the version we      */
		/* requested.                                        */

		
		//if ( LOBYTE( wsaData.wVersion ) != 2 ||
		//	HIBYTE( wsaData.wVersion ) != 2 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
		//	WSACleanup( );
		//	return; 
		//}

	}
}

void CSocket::Attach(CSocket& socket)
{
	hSocket = socket.hSocket;
	addrLocal = socket.addrLocal;
	status = socket.status;
	fdset = socket.fdset;
	sa_src = socket.sa_src;
	sa_dest = socket.sa_dest;
	iLastErrorCode = socket.iLastErrorCode;
	szLastErrorDescription = socket.szLastErrorDescription;

	socket.Detach();
}

void CSocket::Detach()
{
	Initialize();
}

void CSocket::Disconnect()
{
	if (IsConnected())
	{
		shutdown(hSocket, 0); // disable receiving on socket
		shutdown(hSocket, 1); // disable sending on socket
		status = OK;
	}
}

void CSocket::Close()
{
	if (hSocket != NULL)
	{
		// not necessary!?, couldn't find SD_BOTH: shutdown(hSocket, SD_BOTH); // both send and receive har disabled on the socket
		shutdown(hSocket, 0); // disable receiving on socket
		shutdown(hSocket, 1); // disable sending on socket

		slog << "Close socket #" << (int)hSocket << endl;
		closesocket(hSocket);
	}

	Initialize();
}

bool CSocket::SetReuseAddress(bool b)
{
	int one;

	if (b)
		one = 1;
	else
		one = 0;

	if (Error(setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one))))
	{
		status = BAD;
		return false;
	}
	else
		return true;
}

bool CSocket::SetTcpNoDelay(bool b)
{
	// if b == false then disable the nagle algorithm, that is make small packets to be sent immediately

	int one;

	if (b)
		one = 1;
	else
		one = 0;

	if (Error(setsockopt(hSocket, SOL_SOCKET, TCP_NODELAY, (char *)&one, sizeof(one))))
	{
		status = BAD;
		return false;
	}
	else
		return true;
}

bool CSocket::SetSendBufSize(int iByteSize)
{
	if (Error(setsockopt(hSocket, SOL_SOCKET, SO_SNDBUF, (char *)&iByteSize, sizeof(iByteSize))))
	{
		status = BAD;
		return false;
	}
	else
		return true;
}

bool CSocket::SetRcvBufSize(int iByteSize)
{
	if (Error(setsockopt(hSocket, SOL_SOCKET, SO_RCVBUF, (char *)&iByteSize, sizeof(iByteSize))))
	{
		status = BAD;
		return false;
	}
	else
		return true;
}

// bind this socket to the local host and port
bool CSocket::BindLocal(const char* szHostnameOrIPAddr, unsigned short portnr)
{
	if (!IsOk())
	{
		return false;
	}
	char originalHostnameOrIPAddr[256];
	memset(originalHostnameOrIPAddr, 0, sizeof(szHostnameOrIPAddr));
	if (szHostnameOrIPAddr != NULL)
	{
		strcpy(originalHostnameOrIPAddr, szHostnameOrIPAddr);
	}
	unsigned long BroadcastAddress;//=inet_addr(szHostnameOrIPAddr);
	bool multicastAddress = IsMulticastIpAddress(szHostnameOrIPAddr);

	if (szHostnameOrIPAddr == NULL || multicastAddress) // if hostname or address not given or multicast address, take the default
	{
		addrLocal = htonl(INADDR_ANY); // the default interface
		szHostnameOrIPAddr = "default";
	}
	else if(!strcmp(szHostnameOrIPAddr, "255.255.255.255")){
	  szHostnameOrIPAddr = NULL;
	  m_bBroadcast=true;
	} 
	else
	{
		addrLocal = inet_addr(szHostnameOrIPAddr); // first assume the address is in a IPv4 internet protocol dotted address

		if (addrLocal == INADDR_NONE) // check if the convertion went alright
		{ // noop, presumably the address is a hostname, do a name look-up instead
			 struct hostent *hp = gethostbyname(szHostnameOrIPAddr);
			 if (hp == NULL)
			 {
				 status = BAD;
				 return false;
			 }

			 addrLocal = *((unsigned long*)hp->h_addr_list[0]);
		}
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	if(m_bBroadcast){
	  BOOL	  fBroadcast = TRUE;
	  sin.sin_port = htons(0);
	  sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	  if(setsockopt(hSocket, SOL_SOCKET, SO_BROADCAST, (CHAR *)&fBroadcast, sizeof(BOOL)) == SOCKET_ERROR){
		// Return failure
		return false;
	  }

	  INTERFACE_INFO ifinf[32];
	  memset(ifinf, 0, sizeof(ifinf));
	  DWORD cbIfinf;
	  if(SOCKET_ERROR==WSAIoctl(hSocket, SIO_GET_INTERFACE_LIST, NULL, 0, ifinf, sizeof(ifinf), &cbIfinf, NULL, NULL)){
		int error = WSAGetLastError();
		int a=0;
	  }
	  unsigned long netmask=0;
	  for (int n = 0; n < 1; ++n) {
		  BroadcastAddress=ntohl(ifinf[n].iiAddress.AddressIn.sin_addr.s_addr); /* if IPv4 */
		  netmask=ntohl(ifinf[n].iiNetmask.AddressIn.sin_addr.s_addr);
	  }
	  sa_dest.sin_family = AF_INET;
	  sa_dest.sin_port = htons(portnr);
	  sa_dest.sin_addr.s_addr = htonl((u_long)BroadcastAddress | (~netmask));
	}
	else{
	  sin.sin_port = htons(portnr);
	  sin.sin_addr.S_un.S_addr = addrLocal;
	}
	u_long  BlockMode =1;
	  
	// set socket to nonblocking mode
	if(ioctlsocket (hSocket, FIONBIO , (u_long FAR*) &BlockMode) == SOCKET_ERROR ){
	  return TRUE;
	}

	if (multicastAddress)
	{
		if (!ReceiveOnMulticastGroup(originalHostnameOrIPAddr))
		{
			status = BAD;
			return false;
		}
	}

	if (Error(bind(hSocket, (struct sockaddr *)&sin, sizeof(sin))))
	{
		status = BAD;
		return false;
	}


	slog << "Local bind made on socket #" << (int)hSocket << " on " << szHostnameOrIPAddr << " interface and port #" << portnr << endl;

	return true;
}

// bind this socket to the remote host and port
bool CSocket::BindRemote(const char* szRemoteHostnameOrIPAddr, unsigned short portnr)
{
	unsigned long addr = inet_addr(szRemoteHostnameOrIPAddr); // first assume the address is in a IPv4 internet protocol dotted address

	if (addr == INADDR_NONE) // check if the convertion went alright
	{ // noop, presumably the address is a hostname, do a name look-up instead
		 struct hostent *hp = gethostbyname(szRemoteHostnameOrIPAddr);
		 if (hp == NULL) return false;
		 addr = *((unsigned long*)hp->h_addr_list[0]);
	}
	sa_dest.sin_family = AF_INET;
	sa_dest.sin_port = htons(portnr);
    sa_dest.sin_addr.S_un.S_addr = addr;
	
	slog << "Remote bind made on socket #" << (int)hSocket << " to host " << szRemoteHostnameOrIPAddr << " and port #" << portnr << endl;
	m_bBroadcast=false;
  	return true;
}

bool CSocket::IsMulticastIpAddress(const char* szIPAddr)
{
	if (szIPAddr == NULL)
	{
		return false;
	}

	if (strlen(szIPAddr) == 0)
	{
		return false;
	}

	if (IN_MULTICAST(ntohl(inet_addr(szIPAddr))))
	{
		return true;
	}

	return false;
}

bool CSocket::ReceiveOnMulticastGroup(const char* szMulticastGroupnameOrIPAddr)
{
	if (!IsOk()) return false;
		
	struct ip_mreq mreq;
	// mreq is the ip_mreqstructure
	/*	{
		struct in_addr imr_multiaddr;  //The multicast group to join 
		struct in_addr imr_interface;  //The interface to join on
		}
	*/

	unsigned long addr = inet_addr(szMulticastGroupnameOrIPAddr); // first assume the address is in a IPv4 internet protocol dotted address

	if (addr == INADDR_NONE) // check if the convertion went alright
	{ // noop, presumably the address is a hostname, do a name look-up instead
		 struct hostent *hp = gethostbyname(szMulticastGroupnameOrIPAddr);
		 if (hp == NULL) return false;
		 addr = *((unsigned long*)hp->h_addr_list[0]);
	}

	mreq.imr_multiaddr.s_addr = addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY); // use the default interface

	bool fError = Error(setsockopt(hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)));

	if (!fError) slog << "Enabled receiving multicasts on socket #" << (int)hSocket << " for multicast group " << szMulticastGroupnameOrIPAddr << endl;
	return !fError; // if not error then success
}

bool CSocket::StopReceiveOnMulticastGroup(const char* szMulticastGroupnameOrIPAddr)
{
	if (!IsOk()) return false;

	struct ip_mreq mreq;
	// mreq is the ip_mreqstructure
	/*	{
		struct in_addr imr_multiaddr;  //The multicast group to join 
		struct in_addr imr_interface;  //The interface to join on
		}
	*/

	unsigned long addr = inet_addr(szMulticastGroupnameOrIPAddr); // first assume the address is in a IPv4 internet protocol dotted address

	if (addr == INADDR_NONE) // check if the convertion went alright
	{ // noop, presumably the address is a hostname, do a name look-up instead
		 struct hostent *hp = gethostbyname(szMulticastGroupnameOrIPAddr);
		 if (hp == NULL) return false;
		 addr = *((unsigned long*)hp->h_addr_list[0]);
	}

	mreq.imr_multiaddr.s_addr = addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY); // use the default interface

	bool fError = Error(setsockopt(hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)));

	if (!fError) slog << "Disabled receiving multicasts on socket #" << (int)hSocket << " for multicast group " << szMulticastGroupnameOrIPAddr << endl;
	return !fError; // if not error then success
}

// The idea with this method is to stop the short circuit of messages going out from the
// local interface to be sent back to it. This method may not work in some versions of Windows.
bool CSocket::DisableMulticastLoopback()
{
	BOOL status = FALSE;
	int iSize = sizeof(status);
	if (!Error(getsockopt(hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&status, &iSize)))
		return !Error(setsockopt(hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&status, sizeof(status)));
	else
		return false;
}

bool CSocket::Send(const void* pData, int cLenBytes)
{
	if (!IsOk()) return false;

	if (sendto(hSocket, (const char*)pData, cLenBytes, 0, (struct sockaddr *)&sa_dest, sizeof(sa_dest)) == SOCKET_ERROR) return false;

	return true;
}

bool CSocket::Receive(void* pData, int& cLenBytes)
{
	if (!IsOk())
	{
		return false;
	}

	int iRcvLen = sizeof(sa_src);
	cLenBytes = recvfrom(hSocket, (char*)pData, cLenBytes, 0, (struct sockaddr *)&sa_src, &iRcvLen);

	if (cLenBytes == 0 && ((status & CONNECTED) != 0))
		return false; // socket gracefully disconnected by peer
	if (cLenBytes == SOCKET_ERROR)
	{
		HandleError();
		return false;
	}

	return true;
}

bool CSocket::DataIsWaiting()
{
	fdset.fd_count = 1;
	fdset.fd_array[0] = hSocket;

	int cSockets = select(0, &fdset, NULL, NULL, NULL);

	switch (cSockets)
	{
	case 0:
		return false;
		break;
	case 1:
		return true;
		break;
	case SOCKET_ERROR:
		HandleError();
		return false;
		break;
	}

	return false;
}

bool CSocket::DataIsWaiting(float rTmeOut)
{
	static struct timeval timeoutAfter = {0, 0};

	timeoutAfter.tv_sec = long(fabsf(rTmeOut));
	timeoutAfter.tv_usec = long(fmodf(rTmeOut, 1.0f) * 1e6);

	fdset.fd_count = 1;
	fdset.fd_array[0] = hSocket;

	int cSockets = select(0, &fdset, NULL, NULL, &timeoutAfter);

	switch (cSockets)
	{
	case 0:
		return false;
		break;
	case 1:
		return true;
		break;
	case SOCKET_ERROR:
		HandleError();
		return false;
		break;
	}

	return false;
}

void CSocket::HandleError()
{
	#define CASE(_x) case _x: \
						serr << "CSocket Error: " << #_x << endl; \
						szLastErrorDescription = #_x; \
						break;

	switch (iLastErrorCode = WSAGetLastError())
	{
		CASE(WSANOTINITIALISED)
		CASE(WSAENETDOWN)
		CASE(WSAEINTR)
		CASE(WSAEISCONN)
		CASE(WSAEFAULT)
		CASE(WSAEAFNOSUPPORT)
		CASE(WSAEINPROGRESS)
		CASE(WSAEMFILE)
		CASE(WSAENOBUFS)
		CASE(WSAESOCKTNOSUPPORT)
		CASE(WSAEPROTONOSUPPORT)
		CASE(WSAEBADF)
		CASE(WSAEINVAL)
		CASE(WSAEPROTOTYPE)
		CASE(WSAENETRESET)
		CASE(WSAENOPROTOOPT)
		CASE(WSAENOTCONN)
		CASE(WSAEOPNOTSUPP)
		CASE(WSAESHUTDOWN)
		CASE(WSAENOTSOCK)
		CASE(WSAEMSGSIZE)
		CASE(WSAEWOULDBLOCK)
		CASE(WSAETIMEDOUT)
		CASE(WSAECONNRESET)
		default:
			szLastErrorDescription = "Unknown";
			iLastErrorCode = -1;
			break;
	}
}

bool CSocket::Error(int iRetCode)
{
	if (iRetCode == 0)
	{
		szLastErrorDescription = "No Error";
		iLastErrorCode = 0;
		return false;
	}

	HandleError();

	return true;
}

unsigned long CSocket::GetLocalIPAddress()
{
	if (hSocket == NULL) return 0;

	struct sockaddr_in sa_local;

	int iSize = sizeof(sa_local);
	if (getsockname(hSocket, (struct sockaddr*)&sa_local, &iSize) == SOCKET_ERROR)
	{
		HandleError();
		return 0;
	}
	else
		return ntohl(sa_local.sin_addr.S_un.S_addr);
}

unsigned short CSocket::GetLocalPortNr()
{
	if (hSocket == NULL) return 0;

	struct sockaddr_in sa_local;

	int iSize = sizeof(sa_local);
	if (getsockname(hSocket, (struct sockaddr*)&sa_local, &iSize) == SOCKET_ERROR)
	{
		HandleError();
		return 0;
	}
	else
		return ntohs(sa_local.sin_port);
}

bool CSocket::Connect()
{
	if (IsConnected())
	{
		serr << "Trying to connect socket that is already connected" << endl;
		return false; // already connected
	}

	int iSize = sizeof(sa_dest);
	if (connect(hSocket, (struct sockaddr *)&sa_dest, iSize) == 0)
		status = CONNECTED;
	else
	{
		HandleError();
		status = BAD;
	}

	return status == CONNECTED;
}

bool CSocket::Listen(CSocket& socket)
{
	if (!IsOk()) return false;

	if (socket.IsConnected())
	{
		serr << "CSocket used for listning already connected!" << endl;
		return false;
	}

	if (listen(hSocket, SOMAXCONN) == 0)
	{
		struct sockaddr_in sa;
		int saSize = sizeof(sa);
		SOCKET s = accept(hSocket, (struct sockaddr*)&sa, &saSize);

		if (s == INVALID_SOCKET)
		{
			HandleError();
			socket.status = BAD;
			return false;
		}

		socket.hSocket = s;
		socket.status = CONNECTED;
		return true;
	}
	else
	{
		HandleError();
		socket.status = BAD;
		return false;
	}
}

void CSocket::Logging(bool bEnable)
{
	bLogEnable = bEnable;
	slog.Activate(bLogEnable);
	serr.Activate(bLogEnable);
}

void CSocket::SetLogPrompt(std::string& sLogPrompt)
{
	CSocket::sLogPrompt = sLogPrompt;
	slog.SetPrompt(sLogPrompt.c_str());
	serr.SetPrompt(sLogPrompt.c_str());
}
