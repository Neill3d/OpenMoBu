
/**	\file	device_facecap_hardware.cxx
*	Developed by Sergei <Neill3d> Solokhin 2019
*	e-mail to: neill3d@gmail.com
*	twitter: @Neill3d
*
* OpenMoBu github - https://github.com/Neill3d/OpenMoBu
*/


//--- Class declaration
#include "device_facecap_hardware.h"

#include <math.h>
#include <winsock2.h>
#include "tinyosc.h"

///////////////////////////////////////////////////////////////////////

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
	if (WSAStartup(MAKEWORD(2, 2), &wsadata))
	{
		//          GetErrorStr();
		Cleanup();
		return false;
	}
	return true;
}

void bzero(char *b, size_t length)
{
	memset(b, 0, length);
}

int CDevice_FaceCap_Hardware::StartServer(const int server_port)
{
	int lSocket;
	struct protoent* lP;
	struct sockaddr_in  lSin;

	lP = getprotobyname("tcp");

	lSocket = socket(AF_INET, SOCK_DGRAM, 0); // IPPROTO_UDP /*lP->p_proto*/);

	DWORD nonBlocking = 1;
	if (ioctlsocket(lSocket, FIONBIO, &nonBlocking) != 0)
	{
		if (m_Verbose)
		{
			printf("failed to set non-blocking socket\n");
		}
		closesocket(lSocket);
		return 0;
	}

	if (lSocket)
	{
		bzero((char *)&lSin, sizeof(lSin));

		lSin.sin_family = AF_INET;
		lSin.sin_port = htons(server_port);
		lSin.sin_addr.s_addr = INADDR_ANY;

		//Bind socket
		if (bind(lSocket, (struct sockaddr*)&lSin, sizeof(lSin)) < 0)
		{
			if (m_Verbose)
			{
				printf("failed to bind a socket\n");
			}
			closesocket(lSocket);
			return 0;
		}
	}

	return lSocket;
}

/************************************************
 *	Constructor.
 ************************************************/
CDevice_FaceCap_Hardware::CDevice_FaceCap_Hardware()
{
	mParent				= NULL;

	m_Verbose = false;
	
	mNetworkPort		= 9000;
	mStreaming			= true;
	
	mPosition[0]		= 0;
	mPosition[1]		= 0;
	mPosition[2]		= 0;

	mRotation[0]		= 0;
	mRotation[1]		= 0;
	mRotation[2]		= 0;

	Initialize();
}

/************************************************
 *	Destructor.
 ************************************************/
CDevice_FaceCap_Hardware::~CDevice_FaceCap_Hardware()
{
	Cleanup();
}

/************************************************
 *	Set the parent.
 ************************************************/
void CDevice_FaceCap_Hardware::SetParent(FBDevice* pParent)
{
	mParent = pParent;
}

/************************************************
 *	Open device communications.
 ************************************************/
bool CDevice_FaceCap_Hardware::Open()
{
	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool CDevice_FaceCap_Hardware::Close()
{
	StopStream();
	return true;
}


/************************************************
 *	Poll device.
 ************************************************/
bool CDevice_FaceCap_Hardware::PollData()
{
	return true;
}


/************************************************
 *	Fetch a data packet from the device.
 ************************************************/

bool CDevice_FaceCap_Hardware::ProcessMessage(tosc_message *osc)
{
	bool status = false;

	if (m_Verbose)
	{
		printf("[%i bytes] %s %s\n",
			osc->len, // the number of bytes in the OSC message
			tosc_getAddress(osc), // the OSC address string, e.g. "/button1"
			tosc_getFormat(osc)); // the OSC format string, e.g. "f"
	}
	
	const char* address = tosc_getAddress(osc);
	const char* format = tosc_getFormat(osc);

	if (strcmpi(address, "/HT") == 0 && strcmpi(format, "fff") == 0)
	{
		// head translation
		const float x = tosc_getNextFloat(osc);
		const float y = tosc_getNextFloat(osc);
		const float z = tosc_getNextFloat(osc);

		mPosition[0] = static_cast<double>(x);
		mPosition[1] = static_cast<double>(y);
		mPosition[2] = static_cast<double>(z);

		status = true;
	}
	else if (strcmpi(address, "/HR") == 0 && strcmpi(format, "fff") == 0)
	{
		// head rotation
		const float x = tosc_getNextFloat(osc);
		const float y = tosc_getNextFloat(osc);
		const float z = tosc_getNextFloat(osc);

		mRotation[0] = static_cast<double>(x);
		mRotation[1] = static_cast<double>(y);
		mRotation[2] = static_cast<double>(z);

		status = true;
	}
	else if (strcmpi(address, "/ELR") == 0 && strcmpi(format, "ff") == 0)
	{
		// left eye rotation
		const float yaw = tosc_getNextFloat(osc);
		const float pitch = tosc_getNextFloat(osc);

		m_LeftEye[0] = static_cast<double>(yaw);
		m_LeftEye[1] = static_cast<double>(pitch);

		status = true;
	}
	else if (strcmpi(address, "/ERR") == 0 && strcmpi(format, "ff") == 0)
	{
		// right eye rotation
		const float yaw = tosc_getNextFloat(osc);
		const float pitch = tosc_getNextFloat(osc);

		m_RightEye[0] = static_cast<double>(yaw);
		m_RightEye[1] = static_cast<double>(pitch);

		status = true;
	}
	else if (strcmpi(address, "/W") == 0 && strcmpi(format, "if") == 0)
	{
		// blendshape
		const int index = tosc_getNextInt32(osc);
		const float value = tosc_getNextFloat(osc);

		if (index >= 0 && index < static_cast<int>(EHardwareBlendshapes::count))
		{
			m_BlendShapes[index] = static_cast<double>(value);
			status = true;
		}
	}

	return status;
}

int CDevice_FaceCap_Hardware::FetchData()
{
	int number_of_packets = 0;

	if (mSocket)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(mSocket, &readSet);
		struct timeval timeout = { 1, 0 }; // select times out after 1 second
		if (select(mSocket + 1, &readSet, NULL, NULL, &timeout) > 0) {
			sockaddr_in	lClientAddr;
			int		lSize;

			lSize = sizeof(lClientAddr);
			bzero((char *)&lClientAddr, sizeof(lClientAddr));

			int bytes_received = 1;

			while (bytes_received > 0)
			{
				memset(mBuffer, 0, sizeof(char) * 2048);

				bytes_received = recvfrom(mSocket, mBuffer, 2048, 0, (struct sockaddr*) &lClientAddr, &lSize);
				if (m_Verbose)
				{
					FBTrace("bytes received - %d\n", bytes_received);
				}

				if (bytes_received > 0)
				{
					if (tosc_isBundle(mBuffer)) {
						tosc_bundle bundle;
						tosc_parseBundle(&bundle, mBuffer, bytes_received);
						//const uint64_t timetag = tosc_getTimetag(&bundle);
						tosc_message osc;
						while (tosc_getNextMessage(&bundle, &osc)) {
							if (ProcessMessage(&osc))
							{
								number_of_packets = 1;
							}
							else if (m_Verbose)
							{
								FBTrace("[ERROR] incoming message is not recognized \n");
							}
						}
					}
					else {
						tosc_message osc;
						tosc_parseMessage(&osc, mBuffer, bytes_received);

						if (ProcessMessage(&osc))
						{
							number_of_packets = 1;
						}
						else if (m_Verbose)
						{
							FBTrace("[ERROR] incoming message is not recognized \n");
						}
					}
				}
			}
		}
	}

	return number_of_packets;
}


/************************************************
 *	Get the device setup information.
 ************************************************/
bool CDevice_FaceCap_Hardware::GetSetupInfo()
{
	return true;
}


/************************************************
 *	Start the device streaming mode.
 ************************************************/
bool CDevice_FaceCap_Hardware::StartStream()
{
	mSocket = StartServer(mNetworkPort);
	if (mSocket)
	{
		FBTrace("mSocket - %d\n", mSocket);
	}
	
	return mSocket != 0;
}


/************************************************
 *	Stop the device streaming mode.
 ************************************************/
bool CDevice_FaceCap_Hardware::StopStream()
{
	if (mSocket) closesocket(mSocket);
	mSocket = 0;

	return true;
}



/************************************************
 *	Get the current position.
 ************************************************/
void CDevice_FaceCap_Hardware::GetPosition(double* pPos)
{
	pPos[0] = mPosition[0];
	pPos[1] = mPosition[1];
	pPos[2] = mPosition[2];
}


/************************************************
 *	Get the current rotation.
 ************************************************/
void CDevice_FaceCap_Hardware::GetRotation(double* pRot)
{
	pRot[0] = mRotation[0];
	pRot[1] = mRotation[1];
	pRot[2] = mRotation[2];
}

void CDevice_FaceCap_Hardware::GetLeftEyeRotation(double* rotation)
{
	rotation[0] = m_LeftEye[0];
	rotation[1] = m_LeftEye[1];
}
void CDevice_FaceCap_Hardware::GetRightEyeRotation(double* rotation)
{
	rotation[0] = m_RightEye[0];
	rotation[1] = m_RightEye[1];
}

const int CDevice_FaceCap_Hardware::GetNumberOfBlendshapes() const
{
	return static_cast<int>(EHardwareBlendshapes::count);
}
const double CDevice_FaceCap_Hardware::GetBlendshapeValue(const int index)
{
	return m_BlendShapes[index];
}

/************************************************
 *	Communications type.
 ************************************************/
void CDevice_FaceCap_Hardware::SetCommunicationType(FBCommType pType)
{
	mParent->CommType = pType;
}
int CDevice_FaceCap_Hardware::GetCommunicationType()
{
	return mParent->CommType;
}
