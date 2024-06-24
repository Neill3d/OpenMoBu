
#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>

#if defined(HUD_FONT)

#include "freetype-gl.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "markup.h"

#include "nv_math.h"
#include "glslShader.h"

//////////////////////////
//

class CFont
{
public:

	CFont();
	~CFont();

	void Init();
	void Free();

	void	TextClear();
	void	TextAdd(float x, float y, float fontsize, float rectw, float recth, const char *text, const unsigned length);

	bool Display();

	void Resize(int width, int height);

private:

	std::string		fontPath;
	std::string		lasttext;
	markup_t		normal;
	float			mBounds[4];

	NVMath::mat4 model;
	NVMath::mat4 view;
	NVMath::mat4 projection;

	font_manager_t* font_manager;
	text_buffer_t* buffer;	// store pen position and characters

	static GLSLShader* g_glslShader;
	static int		g_shaderCounter;
};

#endif