
#ifndef _NETWORK_UTILS_H_
#define _NETWORK_UTILS_H_

// NetworkUtils.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//
// platform detection

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif

namespace Network {

#define MAX_UDP_BUFFER_SIZE		1500

////////////////////////////////////////////////////////////////////////////////
// Address

    class Address {
    public:

        Address();

		Address(const Address &_other)
				: address(_other.address)
				, port(_other.port)
		{}

        Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d,
                unsigned short port);

        Address(unsigned int address, unsigned short port);

        unsigned int GetAddress() const;

        unsigned char GetA() const;

        unsigned char GetB() const;

        unsigned char GetC() const;

        unsigned char GetD() const;

        unsigned short GetPort() const;

        bool operator==(const Address &other) const;

        bool operator!=(const Address &other) const;

        void Set(unsigned int address, unsigned short port);
		void Set(unsigned char a, unsigned char b, unsigned char c, unsigned char d,
			unsigned short port);

		void SetAddress(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
		void SetAddress(const char *str);
		void SetPortOnly(unsigned short port);

        sockaddr_in GetSocketAddress() const;

    private:

        unsigned int address;
        unsigned short port;
    };

////////////////////////////////////////////////////////////////////////////////
// Socket
    class Socket {
    public:
		//! a constructor
        Socket();
		//! a destructor
        ~Socket();

        bool Open(unsigned short port, bool blocking);

        void Close();

        bool IsOpen() const;

        bool Send(const Address &destination, const void *data, int size);

        int Receive(Address &sender, void *data, int size);

    private:
#if PLATFORM == PLATFORM_WINDOWS
        SOCKET	handle{ 0 };
#else
        int handle{ 0 };
#endif
    };

}

#endif