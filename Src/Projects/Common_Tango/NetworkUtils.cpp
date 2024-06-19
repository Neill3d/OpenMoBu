
// NetworkUtils.cc
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "NetworkUtils.h"

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	#include <android/log.h>

	#include <sys/socket.h>
	#include <sys/un.h>

	#include <unistd.h>

#define LOGI(...) \
	__android_log_print(ANDROID_LOG_INFO, "tango_jni_example", __VA_ARGS__)
#define LOGE(...) \
	__android_log_print(ANDROID_LOG_ERROR, "tango_jni_example", __VA_ARGS__)
#else

#define LOGI	printf
#define LOGE	printf

#endif

#include <stdio.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>



#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "ws2_32.lib" )
#endif

// WORK with sockets on windows

bool InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
	return true;
#endif
}

void ShutdownSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

namespace Network {

/////////////////////////////////////////////////////////////////////
// Address

	Address::Address() {
		address = 0;
		port = 0;
	}

	Address::Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d,
					 unsigned short _port) {
		address = (a << 24) | (b << 16) | (c << 8) | d;
		port = _port;
	}

	Address::Address(unsigned int _address, unsigned short _port)
			: address(_address), port(_port) {
	}

	unsigned int Address::GetAddress() const {
		return address;
	}

	unsigned char Address::GetD() const {
		return (address & 0xFF);
	}

	unsigned char Address::GetC() const {
		return (address >> 8) & 0xFF;
	}

	unsigned char Address::GetB() const {
		return (address >> 16) & 0xFF;
	}

	unsigned char Address::GetA() const {
		return (address >> 24) & 0xFF;
	}

	unsigned short Address::GetPort() const {
		return port;
	}

	bool Address::operator==(const Address &other) const {
		return (port == other.port && address == other.address);
	}

	bool Address::operator!=(const Address &other) const {
		return (port != other.port || address != other.address);
	}

	sockaddr_in Address::GetSocketAddress() const {
		sockaddr_in socket_address;
		socket_address.sin_family = AF_INET;
		socket_address.sin_addr.s_addr = htonl(address);
		socket_address.sin_port = htons(port);

		return socket_address;
	}

	void Address::Set(unsigned int _address, unsigned short _port) {
		address = _address;
		port = _port;
	}

	void Address::Set(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short _port) {
		address = (a << 24) | (b << 16) | (c << 8) | d;
		port = _port;
	}

	void Address::SetAddress(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
	{
		address = (a << 24) | (b << 16) | (c << 8) | d;
	}

	void Address::SetPortOnly(unsigned short _port)
	{
		port = _port;
	}

	void Address::SetAddress(const char *str)
	{
		int a, b, c, d;
#if PLATFORM == PLATFORM_WINDOWS
		int count = sscanf_s(str, "%d.%d.%d.%d", &a, &b, &c, &d);
#else
		int count = sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
#endif
		if (4 == count)
		{
			SetAddress((unsigned char)a, (unsigned char)b, (unsigned char)c, (unsigned char)d);
		}
	}

/////////////////////////////////////////////////////////////////////
// Socket

	Socket::Socket() 
	{}

	Socket::~Socket() {
		Close();
	}

	bool Socket::Open(unsigned short port, bool blocking) {
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

#if PLATFORM == PLATFORM_WINDOWS

        if (handle == INVALID_SOCKET)
        {
            printf("failed to create socket\n");
            return false;
        }
#else
		if (handle <= 0) {
			printf("failed to create socket\n");
			return false;
		}
#endif


		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons((unsigned short) port);

		if (bind(handle, (const sockaddr *) &address, sizeof(sockaddr_in)) < 0) {
			Close();
			printf("failed to bind socket\n");
			return false;
		}
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		socklen_t len;
		int trysize, gotsize, err;
		len = sizeof(int);
		trysize = 1048576+32768;
		do {
			trysize -= 32768;
			setsockopt(handle,SOL_SOCKET,SO_SNDBUF,(char*)&trysize,len);
			err = getsockopt(handle,SOL_SOCKET,SO_SNDBUF,(char*)&gotsize, &len);
			if (err < 0) { perror("getsockopt"); break; }
		} while (gotsize < trysize);
		printf("Size set to %d\n",gotsize);

        len = sizeof(int);
        trysize = 1048576+32768;
        do {
            trysize -= 32768;
            setsockopt(handle,SOL_SOCKET,SO_RCVBUF,(char*)&trysize,len);
            err = getsockopt(handle,SOL_SOCKET,SO_RCVBUF,(char*)&gotsize, &len);
            if (err < 0) { perror("getsockopt"); break; }
        } while (gotsize < trysize);
        printf("Size set to %d\n",gotsize);
#endif
		if (false == blocking) {
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

			int nonBlocking = 1;
			if (fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
				printf("failed to set non-blocking socket\n");
				return false;
			}

#elif PLATFORM == PLATFORM_WINDOWS

            DWORD nonBlocking = 1;
            if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0)
            {
                Close();
                printf("failed to set non-blocking socket\n");
                return false;
            }

#endif
		}

		return true;
	}

	bool Socket::IsOpen() const {
		return (handle > 0);
	}

	void Socket::Close() {
		if (handle > 0) {
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
			close(handle);
#elif PLATFORM == PLATFORM_WINDOWS
			closesocket(handle);
#endif

			handle = 0;
		}
	}

	bool Socket::Send(const Address &destination, const void *data, int size) {
		sockaddr_in socket_address = destination.GetSocketAddress();
		
		int sent_bytes = sendto(handle, (const char *) data, size,
								0, (sockaddr *) &socket_address, sizeof(sockaddr_in));
		//LOGI("socket sent - %d", sent_bytes);
		if (sent_bytes != size) {
			LOGE("failed to send packet, error code %d", errno);

			return false;
		}

		return true;
	}

	int Socket::Receive(Address &sender, void *data, int size) {
		if (0 == handle)
			return 0;

#if PLATFORM == PLATFORM_WINDOWS
		typedef int socklen_t;
#endif

		sockaddr_in from;
		socklen_t fromLength = sizeof(from);

		int received_bytes = recvfrom(handle, (char *) data, size,
									  0, (sockaddr *) &from, &fromLength);

		if (received_bytes <= 0) {
			//LOGE("Error recvfrom(...) %d (%s) ", errno, strerror(errno));
			//printf("failed to receive packet, error - %d\n", errno);

#if PLATFORM == PLATFORM_WINDOWS
			int err = WSAGetLastError();
			//printf("failed to receive packet, error %d\n", err);
#endif

			return 0;
		}


		unsigned int from_address = ntohl(from.sin_addr.s_addr);
		unsigned short from_port = ntohs(from.sin_port);

		sender.Set(from_address, from_port);

		return received_bytes;
	}

}