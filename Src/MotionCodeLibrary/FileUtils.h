
#pragma once

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2024
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


#include <functional>
#include <string>

void SetCurrentFileOpenPath(const char* filepath);

/// <summary>
/// return true if a given filename could be found, otherwise false
/// </summary>
bool IsFileExists ( const char* filename );

//
// search first of all in mobu config folder, then in all plugins folders
//

/// <summary>
/// search a given effect file in the absolute path, in user config path and all registered plugin paths
/// </summary>
/// <param name="effect">an effect file to look for</param>
/// <param name="outPath">a location where a given effect file could be found</param>
/// <param name="outPathLength">a length of outPath array</param>
/// <returns>true if a location for a given effect file is found</returns>
bool FindEffectLocation(const char *effect, char* outPath, const int outPathLength=256);


bool FindEffectLocation(std::function<bool(const char* testPath)> const& lambda, char* outPath, const int outPathLength=256);

/// <summary>
/// open file for reading and keep it open for a class life scope
/// </summary>
class FileReadScope
{
public:
	FileReadScope(const char* filename)
	{
		fopen_s(&fp, filename, "r");
	}

	~FileReadScope()
	{
		if (fp)
			fclose(fp);
	}

	FILE* Get() const { return fp; }

	size_t GetFileSize() const
	{
		fseek(fp, 0, SEEK_END);
		const size_t fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		return fileLen;
	}

	bool ReadString(std::string& textBuffer)
	{
		if (!fp)
			return false;

		const size_t fileSize = GetFileSize();
		if (fileSize == 0)
			return false;

		textBuffer.resize(fileSize, 0);
		const size_t readSize = fread(&textBuffer[0], sizeof(char), fileSize, fp);

		if (readSize < textBuffer.size())
			textBuffer.resize(readSize);

		return (readSize == fileSize);
	}

private:

	FILE* fp{ nullptr };
};