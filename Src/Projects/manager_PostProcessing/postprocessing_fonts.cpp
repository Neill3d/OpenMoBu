
//--- SDK include
#include <fbsdk/fbsdk.h>

#include "FileUtils.h"
#include "postprocessing_fonts.h"

#if defined(HUD_FONT)
#include "ft2build.h"
#endif
#include <math.h>
#include <iostream>
#include <sstream>
//#include <winreg.h>
//#include <sysinfoapi.h>
//#include <winerror.h>

//////////////////////
//
#if defined(HUD_FONT)
#define SHADER_VERTEX	"\\GLSL\\text.vert"
#define SHADER_FRAGMENT	"\\GLSL\\text.frag"

#define FONT_NAME		"Arial"
#define FONT_PATH		"C:\\Windows\\Fonts\\Arial.ttf"


#define ATLAS_SIZE		1024

//////////////////////////////////
//

// ------------------------------------------------------- typedef & struct ---
typedef struct {
	float x, y, z;
	float r, g, b, a;
} vertex_t;


GLSLShaderProgram* CFont::g_glslShader = nullptr;
int CFont::g_shaderCounter = 0;

////////////////////////////////////////////////////////////////////////////
//

// Get system font file path
std::string GetSystemFontFile(const std::string &faceName) {

	static const LPCSTR fontRegistryPath = "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
	HKEY hKey;
	LONG result;
	std::string sFaceName(faceName.begin(), faceName.end());

	// Open Windows font registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fontRegistryPath, 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		return "";
	}

	DWORD maxValueNameSize, maxValueDataSize;
	result = RegQueryInfoKey(hKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0);
	if (result != ERROR_SUCCESS) {
		return "";
	}

	DWORD valueIndex = 0;
	LPSTR valueName = new TCHAR[maxValueNameSize];
	LPBYTE valueData = new BYTE[maxValueDataSize];
	DWORD valueNameSize, valueDataSize, valueType;
	std::string sFontFile;

	// Look for a matching font name
	do {

		sFontFile.clear();
		valueDataSize = maxValueDataSize;
		valueNameSize = maxValueNameSize;

		result = RegEnumValue(hKey, valueIndex, valueName, &valueNameSize, 0, &valueType, valueData, &valueDataSize);

		valueIndex++;

		if (result != ERROR_SUCCESS || valueType != REG_SZ) {
			continue;
		}

		std::string sValueName(valueName, valueNameSize);
		
		// Found a match
		if (strncmp(sFaceName.c_str(), sValueName.c_str(), sFaceName.length()) == 0) {

			sFontFile.assign((LPSTR)valueData, valueDataSize);
			break;
		}
	} while (result != ERROR_NO_MORE_ITEMS);

	delete[] valueName;
	delete[] valueData;

	RegCloseKey(hKey);

	if (sFontFile.empty()) {
		return "";
	}

	// Build full font file path
	TCHAR winDir[MAX_PATH];
	GetWindowsDirectory(winDir, MAX_PATH);

	std::stringstream ss;
	ss << winDir << "\\Fonts\\" << sFontFile;
	sFontFile = ss.str();

	return std::string(sFontFile.begin(), sFontFile.end());
}

///////////////////////////////////////////////////////////////////////////////
//


CFont::CFont()
{
	font_manager = nullptr;
	buffer = nullptr;

	g_shaderCounter += 1;
}

CFont::~CFont()
{
	Free();
}

void CFont::Init()
{
	vec4 white = { { 1.0, 1.0, 1.0, 1.0 } };
	vec4 none = { { 1.0, 1.0, 1.0, 0.0 } };

	buffer = text_buffer_new();

	std::string fontName(FONT_NAME);
	fontPath = FONT_PATH; // GetSystemFontFile(fontName);

	normal.family = (char*)fontPath.c_str();
	normal.size = 13.0f;
	normal.bold = 0;
	normal.italic = 0;
	normal.spacing = 0.0;
	normal.gamma = 2.;
	normal.foreground_color = white;
	normal.background_color = none;
	normal.underline = 0;
	normal.underline_color = white;
	normal.overline = 0;
	normal.overline_color = white;
	normal.strikethrough = 0;
	normal.strikethrough_color = white;
	
	projection = nv::mat4_id;
	model = nv::mat4_id;
	view = nv::mat4_id;
}

void CFont::Free()
{
	if (nullptr != font_manager)
	{
		glDeleteTextures(1, &font_manager->atlas->id);
		font_manager->atlas->id = 0;
		font_manager_delete(font_manager);
		font_manager = nullptr;
	}
	if (nullptr != buffer)
	{
		text_buffer_delete(buffer);
		buffer = nullptr;
	}

	g_shaderCounter -= 1;

	if (g_shaderCounter <= 0 && g_glslShader)
	{
		g_shaderCounter = 0;
		delete g_glslShader;
		g_glslShader = nullptr;
	}
}

void CFont::TextClear()
{
}

void CFont::TextAdd(float x, float y, float _size, float rectw, float recth, const char *text, const unsigned length)
{
	bool hasSizeChange = abs(_size - normal.size) > 0.01f;
	bool hasTextChange = (0 == lasttext.size() || 0 != strcmp(text, lasttext.c_str()));

	if (hasSizeChange || hasTextChange || nullptr == font_manager)
	{
		// DONE: rebuild a font texture and atlas

		if (nullptr != font_manager)
		{
			glDeleteTextures(1, &font_manager->atlas->id);
			font_manager->atlas->id = 0;
			font_manager_delete(font_manager);
			font_manager = nullptr;
		}

		normal.size = _size;

		font_manager = font_manager_new(ATLAS_SIZE, ATLAS_SIZE, LCD_FILTERING_ON);
		normal.font = font_manager_get_from_markup(font_manager, &normal);
		normal.font->kerning = 0;
		
		text_buffer_clear(buffer);

		vec2 pen = { { 0, 0 } };
		text_buffer_add_text(buffer, &pen, &normal, text, length);
		//text_buffer_align(buffer, &pen, ALIGN_CENTER);
		glGenTextures(1, &font_manager->atlas->id);
		glBindTexture(GL_TEXTURE_2D, font_manager->atlas->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(font_manager->atlas->width),
			static_cast<GLsizei>(font_manager->atlas->height), 0, GL_RGB, GL_UNSIGNED_BYTE,
			font_manager->atlas->data);

		vec4 bounds = text_buffer_get_bounds(buffer, &pen);

		mBounds[0] = bounds.x;
		mBounds[1] = bounds.y;
		mBounds[2] = bounds.z;
		mBounds[3] = bounds.w;

		lasttext = text;
	}

	//
	//vec2 pen = { { x, y } };
	//text_buffer_add_text(buffer, &pen, &normal, text, length);

	model = nv::mat4_id;
	
	float stretch = 1.01f; // mBounds[2] / (0.8f * rectw);
	float xoffset = 0.25f * mBounds[2] / length;
	float yoffset = recth - 0.5f * (recth - mBounds[3]);

	model.set_scale(nv::vec3(stretch, 1.0f, 1.0f));
	model.set_translation(nv::vec3(x + xoffset, y + yoffset, 0.0f));
	
}

bool CFont::Display()
{
	if (nullptr == font_manager || nullptr == buffer)
		return false;

	// TODO: check if we need to load a glsl shader
	if (!g_glslShader)
	{
		char path[MAX_PATH];
		if (FindEffectLocation(SHADER_VERTEX, path, MAX_PATH))
		{
			FBString vert(path, SHADER_VERTEX);
			FBString frag(path, SHADER_FRAGMENT);

			g_glslShader = new GLSLShaderProgram;
			g_glslShader->LoadShaders(vert, frag);
		}
	}

	if (!g_glslShader)
		return false;

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	g_glslShader->Bind();
	{

		g_glslShader->setUniformMatrix("model", model.mat_array);
		g_glslShader->setUniformMatrix("view", view.mat_array);
		g_glslShader->setUniformMatrix("projection", projection.mat_array);

		g_glslShader->setUniformUINT("tex", 0);
		g_glslShader->setUniformVector3f("pixel",
			1.0f / font_manager->atlas->width,
			1.0f / font_manager->atlas->height,
			(float)font_manager->atlas->depth);

		glEnable(GL_BLEND);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font_manager->atlas->id);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendColor(1, 1, 1, 1);

		vertex_buffer_render(buffer->buffer, GL_TRIANGLES);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBlendColor(0, 0, 0, 0);
		glUseProgram(0);

		glDisable(GL_BLEND);
	}
	g_glslShader->UnBind();

	return true;
}

void CFont::Resize(int width, int height)
{
	nv::ortho(projection, 0, width, 0, height, -1, 1);
}

#endif