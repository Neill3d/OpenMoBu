// TCP/IP server : test for ordeviceoptical
//

#include <stdio.h>

#define WS_VERSION_REQUIRED 0x0101
#include <winsock.h>
#include <windows.h>

#include "DeviceBuffer.h"

bool Cleanup()
{
    if (WSACleanup()) 
	{
//         GetErrorStr();
        WSACleanup();
        
        return false;
    }
    return true;
}
bool Initialize()
{
    WSADATA wsadata;
    if (WSAStartup(WS_VERSION_REQUIRED, &wsadata)) 
	{
//          GetErrorStr();
        Cleanup();
        return false;
    }
    return true;
}

void bzero(char *b, size_t length)
{
    memset( b,0,length );
}

int StartServer(int pPort)
{
    int lSocket;
    struct protoent* lP;
    struct sockaddr_in  lSin;

    Initialize();

    lP = getprotobyname("tcp");
    
    lSocket = socket(AF_INET, SOCK_STREAM, lP->p_proto);
    
    if (lSocket)
    {
        bzero((char *)&lSin, sizeof(lSin));

		lSin.sin_family = AF_INET;
		lSin.sin_port = htons(pPort);
		lSin.sin_addr.s_addr = 0L;

		//Bind socket
		if( bind(lSocket, (struct sockaddr*)&lSin, sizeof(lSin)) < 0 )
		{
			return 0;
		}

		if( listen(lSocket, 1) < 0 )
		{
			return 0;
		}
	}

	return lSocket;
}

nsTime GetNanoSeconds()
{
	static double dmUnits = 0.0;

	LARGE_INTEGER	t;

	if(QueryPerformanceCounter(&t)) 
	{
		double	count;
		count = (double) t.QuadPart;
		if (!dmUnits) 
		{
			QueryPerformanceFrequency(&t);
			dmUnits = 1000000000.0 / (double) t.QuadPart;
		}
		return (unsigned __int64) (count * dmUnits);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	printf("Server ...\n");

    int Soc=0;
    while (1)
    {
		static double ServerStartedOn = 0.0;
        if (!Soc) 
        {
			Soc = StartServer( PORTNUMBER );
			if (Soc) 
			{
				printf("Server started on port %d\n",PORTNUMBER);
			}
		} else
		{
			printf("Waiting for connection\n");
			sockaddr_in	lClientAddr;
			int		lSize;
			int		lSocket;

			lSize = sizeof(lClientAddr);
	        bzero((char *)&lClientAddr, sizeof(lClientAddr));

			lSocket = accept(Soc, (struct sockaddr*)&lClientAddr, &lSize);
			if( lSocket >= 0 ) 
			{
				sockaddr_in	lAddr;
				sDataBuffer mDataBuffer;

		        bzero((char *)&lAddr, sizeof(lAddr));
				if( getsockname(lSocket, (struct sockaddr*)&lAddr, &lSize) < 0 )
				{
					return -1;
				}
				printf("Connection established\n");
				ServerStartedOn = (double)GetNanoSeconds();

				int iResult, iSendResult;

				for (;;)
				{
					double timestamp = ((double)GetNanoSeconds() - ServerStartedOn) / 1000000000.0;

					// receive data from a client (mobu sync state, image data, camera list, take list)
					iResult = recv(lSocket, (char*)&mDataBuffer, sizeof(mDataBuffer), 0);

					if (iResult > 0) 
					{
						// we have received iResult bytes
						// TODO: process data
					}
					else if (iResult == 0)
					{
						// closing a connection
					}
					else {
						printf("recv failed: %d\n", WSAGetLastError());
						break;
					}

					mDataBuffer.Simulate(timestamp);

					// write back last device sync state, commands, feedback
					iSendResult = send(lSocket, (char*)&mDataBuffer, sizeof(mDataBuffer), 0);

					if (iSendResult == SOCKET_ERROR)
					{
						printf("send failed: %d\n", WSAGetLastError());
						break;
					}

					Sleep( 1000/SIM_FPS );
				}
			
				shutdown(lSocket, 2);
				closesocket(lSocket);

				printf("Connection closed, connection time = %f ms\n",(GetNanoSeconds()-ServerStartedOn)/1000000.0);

			}
		}
	}
	
    if (Soc) 
    {
        closesocket( Soc );
    }
	Cleanup();
	return 0;
}

