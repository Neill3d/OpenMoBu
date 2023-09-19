
// postprocessing_helper.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postEffectUtils.h"
#include "rg_etc1\rg_etc1.h"

#pragma warning(push)
#pragma warning(disable:4265)
#include <mutex>
#pragma warning(pop)

#include <thread>

using namespace PostProcessingEffects;

// extract 4x4 RGBA block from source image
void CodecETC1_ExtractBlockRGBA(const unsigned char *src, int x, int y, int w, int h, int pitch, unsigned char *block)
{
	static const int map[] = { 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 2, 0, 0, 1, 2, 3 };
	int bx, by, bw, bh;

	bw = (w - x < 4) ? (w-x) : 4;
	bh = (h - y < 4) ? (h-y) : 4;
	for (int i = 0; i < 4; ++i)
	{
		by = map[(bh - 1) * 4 + i] + y;
		for (int j = 0; j < 4; ++j)
		{
			bx = map[(bw - 1) * 4 + j] + x;
			block[(i * 4 * 4) + (j * 4) + 0] = src[(by * pitch) + (bx * 4) + 0];
			block[(i * 4 * 4) + (j * 4) + 1] = src[(by * pitch) + (bx * 4) + 1];
			block[(i * 4 * 4) + (j * 4) + 2] = src[(by * pitch) + (bx * 4) + 2];
			block[(i * 4 * 4) + (j * 4) + 3] = src[(by * pitch) + (bx * 4) + 3];
		}
	}
}

struct blockInfo
{
	int tilex;
	int tiley;
	int tilew;
	int tileh;

	int imagewidth;
	int imageheight;
	int pitch;
	int temp;

	unsigned char *imagedata;
	unsigned char *stream;
};

void compute_block(int tilex,
					int tiley,
					int tilew,
					int tileh,

					int imagewidth,
					int imageheight,
					int pitch,
					int temp,

					unsigned char *imagedata,
					unsigned char *stream)
{
	unsigned int block[16];

	rg_etc1::etc1_pack_params options;
	options.m_quality = (rg_etc1::etc1_quality) 0;

	for (int y = tiley; y < tileh; y++)
	{
		for (int x = tilex; x < tilew; x++)
		{
			// extract block 
			CodecETC1_ExtractBlockRGBA(imagedata, x * 4, y * 4, imagewidth, imageheight, pitch, (unsigned char*)block);
			// pack block
			rg_etc1::pack_etc1_block(stream, block, options);
			stream += 8;
		}
	}
}

// compress texture

// run in 4 threads

static const int num_threads = 16;

//std::thread t[num_threads];

size_t CompressImageBegin(unsigned char *stream, int imagewidth, int imageheight,
	unsigned char *imagedata, int pitch, int quality)
{
	// try a parallel execution

	rg_etc1::pack_etc1_block_init();

	//
	// launch a group of threads

	//int blockw = imagewidth / 4;
	//int blockh = imageheight / 4 / num_threads;
	//int blockStreamOffset = 8 * blockw * blockh;

	for (int i = 0; i < num_threads; ++i)
	{
		//t[i] = std::thread(compute_block, 0, i * blockh, blockw, (i + 1) * blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + i * blockStreamOffset);
	}

	return 1;
}

size_t CompressImageEnd()
{
	// join the threads with the main thread
	for (int i = 0; i < num_threads; ++i)
	{
		//t[i].join();
	}

	return 1;
}

size_t RgEtc1_CompressSingleImage(unsigned char *stream, int imagewidth, int imageheight, 
	unsigned char *imagedata, int pitch, int quality)
{
	


	// try a parallel execution

	rg_etc1::pack_etc1_block_init();

	

	// launch a group of threads

	//int blockw = imagewidth / 4;
	//int blockh = imageheight / 4 / num_threads;
	//int blockStreamOffset = 8 * blockw * blockh;

	for (int i = 0; i < num_threads; ++i)
	{
		//t[i] = std::thread(compute_block, 0, i * blockh, blockw, (i + 1) * blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + i * blockStreamOffset);
	}

	// join the threads with the main thread
	for (int i = 0; i < num_threads; ++i)
	{
		//t[i].join();
	}
	
	/*
	blockInfo info1 = { 0, 0, blockw, blockh, imagewidth, imageheight, pitch, 0, imagedata, stream };
	blockInfo info2 = { 0, blockh, blockw, 2*blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + blockStreamOffset };
	blockInfo info3 = { 0, 2*blockh, blockw, 3*blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + 2 * blockStreamOffset};
	blockInfo info4 = { 0, 3*blockh, blockw, 4*blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + 3 * blockStreamOffset };
	
	std::thread tile1(compute_block, 0, 0, blockw, blockh, imagewidth, imageheight, pitch, 0, imagedata, stream);
	std::thread tile2(compute_block, 0, blockh, blockw, 2 * blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + blockStreamOffset);
	std::thread tile3(compute_block, 0, 2 * blockh, blockw, 3 * blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + 2 * blockStreamOffset);
	std::thread tile4(compute_block, 0, 3 * blockh, blockw, 4 * blockh, imagewidth, imageheight, pitch, 0, imagedata, stream + 3 * blockStreamOffset);

	

	tile1.join();
	tile2.join();
	tile3.join();
	tile4.join();
	*/
	/*
	rg_etc1::etc1_pack_params options;
	options.m_quality = (rg_etc1::etc1_quality) quality;

	unsigned int block[16];

	rg_etc1::pack_etc1_block_init();
	for (int y = 0; y < imageheight / 4; y++)
	{
		for (int x = 0; x < imagewidth / 4; x++)
		{
			// extract block 
			CodecETC1_ExtractBlockRGBA(imagedata, x * 4, y * 4, imagewidth, imageheight, pitch, (unsigned char*)block);
			// pack block
			rg_etc1::pack_etc1_block(stream, block, options);
			stream += 8;
		}
	}
	*/
	return imagewidth*imageheight / 2;
}

///////////////////////////////////////////////////////////////////////////
//

void ComputeCameraOrthoPoints(const SViewInfo& viewInfo, vec3* points)
{
	mat4 mvInv4;
    invert(mvInv4, viewInfo.modelview);

	float fard = static_cast<float>(viewInfo.farPlane);
	float neard = static_cast<float>(viewInfo.nearPlane);

    const float ratio = viewInfo.GetRatio();

	mat3 invRot;
	mvInv4.get_rot(invRot);

	const vec3 view_dir = invRot * vec3(0.0f, 0.0f, -1.0f);
	const vec3 up = invRot * vec3(0.0f, 1.0f, 0.0f);
	const vec3 right = invRot * vec3(1.0f, 0.0f, 0.0f);
	
	const vec3 center(viewInfo.cameraPos);

	const vec3 fc = center + fard*view_dir;
	const vec3 nc = center + neard*view_dir;

	neard = viewInfo.renderHeight;
	fard = viewInfo.renderHeight;

	// these heights and widths are half the heights and widths of
	// the near and far plane rectangles
	const float near_height = neard;
	const float near_width = near_height * ratio;
	const float far_height = fard;
	const float far_width = far_height * ratio;
	
	points[0] = nc - near_height*up - near_width*right;
	points[1] = nc + near_height*up - near_width*right;
	points[2] = nc + near_height*up + near_width*right;
	points[3] = nc - near_height*up + near_width*right;

	points[4] = fc - far_height*up - far_width*right; // bottom left
	points[5] = fc + far_height*up - far_width*right; // up left
	points[6] = fc + far_height*up + far_width*right; // up right
	points[7] = fc - far_height*up + far_width*right; // bottom right
}

// DONE: FIX culling winding

void ComputeCameraFrustumPoints(const SViewInfo& viewInfo, vec3* points)
{
	mat4 mvInv4;
    invert(mvInv4, viewInfo.modelview);
	
	const float fard = static_cast<float>(viewInfo.farPlane);
	const float neard = static_cast<float>(viewInfo.nearPlane);

	const float fov = nv_to_rad  * static_cast<float>(viewInfo.fieldOfView);
    const float ratio = viewInfo.GetRatio();

	mat3 invRot;
	mvInv4.get_rot(invRot);
	const vec3 view_dir = invRot * vec3(0.0f, 0.0f, -1.0f);
	const vec3 up = invRot * vec3(0.0f, 1.0f, 0.0f);
	const vec3 right = invRot * vec3(1.0f, 0.0f, 0.0f);
	
	const vec3 center(viewInfo.cameraPos);
	
	const vec3 fc = center + fard*view_dir;
	const vec3 nc = center + neard*view_dir;

	// these heights and widths are half the heights and widths of
	// the near and far plane rectangles
	const float near_height = tanf(fov * 0.5f) * neard;
	const float near_width = near_height * ratio;
	const float far_height = tanf(fov * 0.5f) * fard;
	const float far_width = far_height * ratio;
	
	points[0] = nc - near_height*up - near_width*right;
	points[1] = nc + near_height*up - near_width*right;
	points[2] = nc + near_height*up + near_width*right;
	points[3] = nc - near_height*up + near_width*right;

	points[4] = fc - far_height*up - far_width*right; // bottom left
	points[5] = fc + far_height*up - far_width*right; // up left
	points[6] = fc + far_height*up + far_width*right; // up right
	points[7] = fc - far_height*up + far_width*right; // bottom right
}