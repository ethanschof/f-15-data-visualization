// interface for the CSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKET_H__62B09CBF_2DAB_11D5_AEAD_00500450FDE0__INCLUDED_)
#define AFX_SOCKET_H__62B09CBF_2DAB_11D5_AEAD_00500450FDE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WINSOCKAPI_ 
  #define _WINSOCKAPI_   // Prevent inclusion of winsock.h in windows.h
#endif

//#include <windows.h>
// include file winsock2.h is needed because:
// Types SOCKET, WSADATA and structs sockaddr_in, fd_set among others ...
#include <winsock2.h>

#include <string>

/*
Typical usages:

  CSocket sock1(SOCK_DGRAM, IPPROTO_UDP); // create socket object sock1
  sock1.BindLocal(NULL, 123); // make socket be binded to local address, defined by name NULL (that is the default interface) and port number 123
  sock1.BindRemote("123.234.56.45");

  CSocket sock2(SOCK_DGRAM, IPPROTO_UDP); // create socket object sock2
  sock2.BindLocal("123.234.56.45", 10001); // make socket be binded to local address (interface) 123.234.56.45 and port number 10001
  sock2.ReceiveOnMulticastGroup("225.6.7.8"); // also make the socket be binded to the multicast address 225.6.7.8


  // To use TCP/IP

  // on server:
  CSocket serverSocket(SOCK_STREAM, IPPROTO_TCP);
  serverSocket.BindLocal(<interface, IP-address>, <port>); // socket where this server will listen

  #define ever ;;
  for(ever)
  {
	CSocket s; // unattached socket
	serverSocket.Listen(s); // here we will hang until some connects to this server
	if (s.IsConnected())
		... start to send and receive on socket s, perhaps start a thread here ...
	...
  }

  // on client:
  CSocket clientSocket(SOCK_STREAM, IPPROTO_TCP);
  clientSocket.BindRemote(<remote IP-address>, <port>);
  if (clientSocket.IsConnected())
		... start to send and receive on socket s, perhaps start a thread here ...

*/

class CSocket  
{
public:
	enum Status {NOT_INITIALIZED=0, BAD=1, OK=2, CONNECTED=4}; // CONNECTED used just for TCP/IP

	CSocket();
	CSocket(int type /*=SOCK_DGRAM|SOCK_STREAM*/,
		   int protocol=IPPROTO_UDP /* IPPROTO_TCP */,
		   bool bLogEnable = false,
		   std::string sLogPrompt = "");
	virtual ~CSocket();

	void Close();

	// bind this socket to the local host and port, returns true if successful
	// use NULL as first parameter if the default interface should be binded
	bool BindLocal(const char* szHostnameOrIPAddr, unsigned short portnr);
	// bind this socket to the remote host and port, returns true if successful
	bool BindRemote(const char* szRemoteHostnameOrIPAddr, unsigned short portnr);

	// listens to incoming Connects and returns a socket for established connects
	// socket to be used must be a SOCK_STREAM, IPPROTO_TCP one
	bool Listen(CSocket& socket);

	// connect socket to a server, specified by the BindRemote method
	bool Connect();

	// disconnect the socket
	void Disconnect();

	bool IsMulticastIpAddress(const char* szIPAddr);

	// enable receive of multicasting. Bind must be called first. returns true if successful
	bool ReceiveOnMulticastGroup(const char* szMulticastGroupnameOrIPAddr);
	bool StopReceiveOnMulticastGroup(const char* szMulticastGroupnameOrIPAddr);

	bool SetReuseAddress(bool b);
	bool SetTcpNoDelay(bool b);
	bool SetSendBufSize(int iByteSize);
	bool SetRcvBufSize(int iByteSize);

	bool IsOk()
	{
		return (status & (OK | CONNECTED)) != 0;
	}
	bool IsConnected()
	{
		return (status & CONNECTED) != 0;
	}
	Status GetStatus() {return status;}

	bool Send(const void* pData, int cLenBytes); // send a data to socket

	// Receive data on socket. Call is blocking if no data is available.
	// Returns true upon successfull receivment.
	// Returns false if an error or if a connected socket is gracefully closed by the peer.
	bool Receive(void* pData, int& cLenBytes); // receive data on socket, this call is blocking if no data is current available

	// returns true if data is waiting to be retrieved by Receive
	// if no in-parameter is given the call will block
	// otherwise it will be a used as the time-out value in seconds
	bool DataIsWaiting();
	bool DataIsWaiting(float rTmeOut);

	static const char* GetVendorInfo() {return wsaData.lpVendorInfo;}
	static int GetMaxNrOfSocketsPossible() {return wsaData.iMaxSockets;}
	static const char* GetSocketDescription() {return wsaData.szDescription;}
	static const char* GetSocketSystemStatus() {return wsaData.szSystemStatus;}

	int GetLastErrorCode() {return iLastErrorCode;}
	const char* GetLastErrorDescription() {return szLastErrorDescription;}

	unsigned long GetLocalIPAddress();
	unsigned short GetLocalPortNr();
	unsigned long GetIPAddrOfLastReceivedSource() {return ntohl(sa_src.sin_addr.S_un.S_addr);}
	unsigned short GetPortNrOfLastReceivedSource() {return ntohs(sa_src.sin_port);}

	SOCKET GetSocket() {return hSocket;}

	// attach to another Sockets "socket" (hSocket)
	void Attach(CSocket& socket);
	// detach from the socket referenced by the local hSocket handle
	void Detach();

	// The idea with DisableMulticastLoopback is to stop the short circuit of messages going out from the
	// local interface to be sent back to it. This method may not work in some versions of Windows.
	bool DisableMulticastLoopback(); // disable that outgoing multicast packets are sent back to the sender

	// Socket logging to stdout, stdlog and stderr control
	void Logging(bool bEnable);
	void SetLogPrompt(std::string& sLogPrompt);

protected:
	void Initialize();
	void InitializeWSA();

	bool Error(int iRetCode);
	void HandleError();

	static int cInstance;
	Status status;

	unsigned long addrLocal; // local bound address in network byte order

	fd_set fdset;

	struct sockaddr_in sa_dest; // remote bound address
	struct sockaddr_in sa_src;  // source address
	static WSADATA wsaData;
	SOCKET hSocket;
	int iLastErrorCode;
	char* szLastErrorDescription;
	std::string sLogPrompt;

	bool bLogEnable;
	bool m_bBroadcast;
};

#endif // !defined(AFX_SOCKET_H__62B09CBF_2DAB_11D5_AEAD_00500450FDE0__INCLUDED_)
