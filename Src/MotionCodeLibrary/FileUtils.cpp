
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

#include "FileUtils.h"
#include <string>
#include <string_view>
#include <system_error>
#include <Windows.h>

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "mobu_logging.h"

namespace fs = std::filesystem;
static fs::path g_currentOpenFile;

void SetCurrentFileOpenPath(const char* filepath)
{
    if (filepath && *filepath)
        g_currentOpenFile = fs::path(AnsiToWide(filepath));
    else
        g_currentOpenFile.clear();
}

namespace
{
	bool IsRegularFile(const fs::path& path)
	{
		std::error_code ec;
		const bool result = fs::is_regular_file(path, ec);
		return result && !ec;
	}
}

/////////////////////////////////////////////////////////////

bool IsFileExists(const char* filename)
{
	if (!filename || !*filename)
		return false;

	std::error_code ec;
	return std::filesystem::exists(filename, ec) && !ec;
}

////////////////////////////////////////////////////////////
std::wstring AnsiToWide(std::string_view text)
{
    if (text.empty())
        return {};

    if (text.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        throw std::length_error("ANSI string is too long");
    }

    const int inputLength = static_cast<int>(text.size());

    const int requiredLength = MultiByteToWideChar(
        CP_ACP,
        0,
        text.data(),
        inputLength,
        nullptr,
        0);

    if (requiredLength == 0)
    {
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "ANSI to UTF-16 conversion failed");
    }

    std::wstring result(static_cast<size_t>(requiredLength), L'\0');

    if (MultiByteToWideChar(
        CP_ACP,
        0,
        text.data(),
        inputLength,
        result.data(),
        requiredLength) == 0)
    {
        throw std::system_error(
            static_cast<int>(GetLastError()),
            std::system_category(),
            "ANSI to UTF-16 conversion failed");
    }

    return result;
}

////////////////////////////////////////////////////////////
std::optional<fs::path> FindEffectLocation(const fs::path& requestedPath)
{
    if (requestedPath.empty())
        return std::nullopt;

    // A genuinely absolute filename does not need search locations.
    if (requestedPath.is_absolute())
    {
        if (IsRegularFile(requestedPath))
            return requestedPath.lexically_normal();

        return std::nullopt;
    }

    // This also tolerates old names such as "/GLSL/file.glslf".
    const fs::path relativePath = requestedPath.relative_path();
    if (relativePath.empty())
        return std::nullopt;

    auto checkLocation =
        [&relativePath](const fs::path& basePath)
        -> std::optional<fs::path>
        {
            if (basePath.empty())
                return std::nullopt;

            const fs::path candidate = (basePath / relativePath).lexically_normal();

            if (IsRegularFile(candidate))
                return candidate;

            return std::nullopt;
        };

    FBSystem& system = FBSystem::TheOne();

    const char* userConfigPath = static_cast<const char*>(system.UserConfigPath);

    if (userConfigPath && *userConfigPath)
    {
        const fs::path basePath(AnsiToWide(userConfigPath));

        if (auto result = checkLocation(basePath))
            return result;
    }

    if (!g_currentOpenFile.empty())
    {
        const fs::path currentFile(g_currentOpenFile);
        const fs::path currentDirectory = currentFile.parent_path();

        if (auto result = checkLocation(currentDirectory))
            return result;
    }

#ifndef ORSDK2013
    const FBStringList pluginPaths = system.GetPluginPath();

    for (int i = 0; i < pluginPaths.GetCount(); ++i)
    {
        const char* pluginPath = static_cast<const char*>(pluginPaths[i]);

        if (pluginPath && *pluginPath)
        {
            const fs::path basePath(AnsiToWide(pluginPath));

            if (auto result = checkLocation(basePath))
                return result;
        }
    }
#endif

    return std::nullopt;
}

std::optional<fs::path> FindEffectLocation(const LocationCheck& checkLocationFn)
{
    if (!checkLocationFn)
        return std::nullopt;

    auto checkPath = [&checkLocationFn](const fs::path& basePath) -> std::optional<fs::path>
        {
            if (basePath.empty())
                return std::nullopt;

            try
            {
                const fs::path normalizedPath = basePath.lexically_normal();

                if (checkLocationFn(normalizedPath))
                    return normalizedPath;
            }
            catch (const std::exception& exception)
            {
                LOGE("[FileUtils] Failed to check location: %s\n", exception.what());
            }

            return std::nullopt;
        };

    auto checkAnsiPath = [&checkPath](const char* ansiPath) -> std::optional<fs::path>
        {
            if (!ansiPath || !*ansiPath)
                return std::nullopt;

            try
            {
                return checkPath(fs::path(AnsiToWide(ansiPath)));
            }
            catch (const std::exception& exception)
            {
                LOGE("[FileUtils] Failed to convert location '%s': %s\n", ansiPath, exception.what());
                return std::nullopt;
            }
        };

    FBSystem& system = FBSystem::TheOne();

    const char* userConfigPath = static_cast<const char*>(system.UserConfigPath);

    if (auto result = checkAnsiPath(userConfigPath))
        return result;

    if (!g_currentOpenFile.empty())
    {
        const fs::path currentDirectory = g_currentOpenFile.parent_path();

        if (auto result = checkPath(currentDirectory))
            return result;
    }

#ifndef ORSDK2013
    const FBStringList pluginPaths = system.GetPluginPath();

    for (int i = 0; i < pluginPaths.GetCount(); ++i)
    {
        const char* pluginPath = static_cast<const char*>(pluginPaths[i]);

        if (auto result = checkAnsiPath(pluginPath))
            return result;
    }
#endif

    return std::nullopt;
}