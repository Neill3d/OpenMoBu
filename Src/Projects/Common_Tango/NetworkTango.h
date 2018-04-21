
#ifndef _NETWORK_TANGO_H_
#define _NETWORK_TANGO_H_

// NetworkTango.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "NetworkUtils.h"
#include <stdint.h>

namespace Network
{

#define PACKET_MAGIC_1      't'
#define PACKET_MAGIC_2      'r'
#define PACKET_MAGIC_3      'k'

#define PACKET_REASON_REGISTER	3
#define PACKET_REASON_FEEDBACK	4
#define PACKET_REASON_CAMERA	5
#define PACKET_REASON_CAMERA_INFO 6
#define PACKET_REASON_TAKE_INFO 7
#define PACKET_REASON_CONTROL   1
#define PACKET_REASON_COMMAND   2
#define PACKET_REASON_IMAGE     10
#define PACKET_REASON_IMAGE_TILE 11

#define    PACKET_COMMAND_SYNC_TAKES    1        // command to receive a takes list
#define PACKET_COMMAND_LENS            2        // command to change a value of lens

#define PACKET_COMMAND_PLAY            3
#define PACKET_COMMAND_STOP            4
#define PACKET_COMMAND_REWIND        5
#define PACKET_COMMAND_LOOP            6
#define    PACKET_COMMAND_RECORD        7        // we should specify if we want to create a new take or override existing
#define PACKET_COMMAND_LIVE				20
#define PACKET_COMMAND_CHAT            8        // just send a log message between tablet and mobu
#define PACKET_COMMAND_SPEED_NDX    9        // change a playback speed
#define PACKET_COMMAND_TAKE_NDX        10        // change a current take
#define PACKET_COMMAND_TAKE_NEW        11        // create a new take or (duplicate, delete)
#define PACKET_COMMAND_FRAME_START    12
#define PACKET_COMMAND_FRAME_STOP    13
#define PACKET_COMMAND_FRAME_CURR    14

#define PACKET_COMMAND_MOVE_SCALE    15        // just logging about movement changes

#define PACKET_COMMAND_SYNC_CAMERAS    16        // sync camera names in the scene
#define PACKET_COMMAND_CAMERA_DATA    17        // query a ndx camera data

#define PACKET_COMMAND_SYNC_OBJECTS    18        // sync scene objects names in the scene

#define PACKET_COMMAND_SCENE_NAME    19        // query a scene name (fbx filename)

#define PACKET_COMMAND_SCREEN_POS	21
#define PACKET_COMMAND_SCREEN_SIZE	22

#define PACKET_COMMAND_CAMERALIST_REQ	23
#define PACKET_COMMAND_CAMERA_SWITCH    24
#define PACKET_COMMAND_CAMERA_ALIGN 	25
#define PACKET_COMMAND_CAMERA_PARENT	26
#define PACKET_COMMAND_CAMERA_NEW       27

#define PACKET_COMMAND_TAKE_REM		28

#define MAX_NUMBER_OF_TILES     256
#define DEFAULT_TILE_SIZE		1200
    // 65536
#define MAX_IMAGE_SIZE          131072

// header - 8 bytes
    union CHeader {

		uint8_t bytes[8];

		struct {
			uint8_t magic[3];     // check if a packet is control packet
			uint8_t reason;
			float timestamp;    // timestamp id to check that packet is recent
		};
    };


    struct CImageHeader
    {
        float		aspect;			// original image aspect

        unsigned short width;
        unsigned short height;
		// 8
        int     dataSize;    // bytes, negative value for compressed, possitive for uncompressed
		int		compressed;
        int		internalFormat;	// compressed format
		// 8
        //int     dataFormat; // GL_UNSIGNED_BYTE by default

        //unsigned short numTiles;    // number of tiles for that image
        //unsigned int    tileSize;   // size for each tile (1k by default)
    };

    struct CImageTileHeader
    {
        unsigned int    tileSize; // current tile size
        unsigned int    tileOffset;

        unsigned short tileIndex; // if this packet is just a peace of main image
        unsigned short tileCount; // total number of tiles in that timestamp
    };

// sync all states (24)
    struct CSyncControl
    {
        int startFrame;
        int stopFrame;
        int currFrame;
		// 12
        unsigned char take;    // current take
        unsigned char takeCount;

        unsigned char speed;    // current speed

        unsigned char recordMode;
        unsigned char playMode;
        unsigned char loopMode;

		unsigned char	liveMode;
		unsigned char	temp;
		// 8
        float lens;
		// 4
    };

	struct CCommand {
		// send a command to mobu
		unsigned char command;
		int value;
	};

	struct CDeviceData {

		float translation[3];
		float orientation[4];    // quaternion
		// 28
		float lens;
		float moveMult;        // internal multiplication state
		float fly;
		// 12
		unsigned char triggers[6];
		// 8
		unsigned char unused[2];
	};

	struct CStaticName
	{
		char    raw[32];
	};

	//
	struct CTakeInfo {
		unsigned char	id;
		unsigned char	totalCount;

		unsigned char	temp[2];

		CStaticName		name;
	};


	// 106
	struct CCameraInfo {
		// 56
		double translation[3];
		double orientation[4];

		// 16
		float lens;
		float moveMult;
		float nearPlane;
		float farPlane;

		// 2
		unsigned char 	id;
		unsigned char	totalCount;

		// 32
		CStaticName		name;
	};

	//////////////////////////////////////////////////////////

	struct CPacketImage
	{
		CHeader			header;
		CImageHeader	imageheader;
	};

	struct CPacketImageTile
	{
		CHeader				header;
		CImageTileHeader	tileheader;
	};

	struct CPacketControl
	{
		CHeader			header;
		CSyncControl	body;
	};

    struct CPacketCommand {

        CHeader			header;
		CCommand		body;
        
    };

    struct CPacketDevice {
        CHeader			header;
        CDeviceData     body;
    };

	struct CPacketCameraInfo {
		CHeader 	header;
		CCameraInfo 	body;
	};

	struct CPacketTakeInfo {
		CHeader		header;
		CTakeInfo	body;
	};

	void FillBufferWithHeader(unsigned char *mBuffer, unsigned char reason, const float timestamp);
	bool CheckMagicNumber(unsigned char *mBuffer);

}

#endif // _NETWORK_TANGO_H_