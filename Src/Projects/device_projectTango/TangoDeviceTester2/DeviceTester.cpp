// TCP/IP server : test for ordeviceoptical
//

#include <stdio.h>

#define WS_VERSION_REQUIRED 0x0101

#include <WinSock2.h>
#include <windows.h>

#include "StreamThread.h"

#include <thread>


/////////////////////////////////////////////////////////////////////////
//

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


int main(int argc, char* argv[])
{
	printf("Server ...\n");

	Initialize();
   
	std::thread	lthread(Network::MainServerFunc);
	lthread.join();

	Cleanup();
	return 0;
}

