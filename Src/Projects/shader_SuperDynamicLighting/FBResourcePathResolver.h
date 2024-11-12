
#pragma once

// FBResourcePathResolver.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ResourcePathResolver.h"
#include <filesystem>
//--- SDK include
#include <fbsdk/fbsdk.h>

#include "mobu_logging.h"

class FBResourcePathResolver : public IResourcePathResolver {
public:
    bool CheckShadersPath(const std::filesystem::path& path, const std::vector<std::string>& localResources) const {
        
        for (const std::string shader_path : localResources)
        {
            const std::filesystem::path full_path = path / shader_path;

            if (!std::filesystem::exists(full_path))
            {
                return false;
            }
        }

        return true;
    }

    std::filesystem::path FindResourcePath(const std::vector<std::string>& systemPaths, const std::vector<std::string>& localResources) const override 
    {
        FBSystem system;
        std::filesystem::path shadersPath = system.ApplicationPath.AsString();
        shadersPath = shadersPath / "plugins";

        if (CheckShadersPath(shadersPath, localResources)) 
        {
            return shadersPath;
        }

        // Check each system path if the initial application path fails
        for (const auto& path : systemPaths) 
        {
            if (CheckShadersPath(path, localResources)) {
                shadersPath = path;
                return shadersPath;
            }
        }

        LOGE("[ShaderPathResolver] Failed to find shaders!");
        return "";
    }
};


class FBShaderPathResolver : public FBResourcePathResolver
{
public:

    // searches in application path and every plugin path
    std::filesystem::path FindShaderPath(const std::vector<std::string>& localResources, const char* shaderSubFolder = "/GLSL/") const
    {
        FBSystem system;

        std::vector<std::string> systemPaths;
        const FBStringList& plugin_paths = system.GetPluginPath();

        for (int i = 0; i < plugin_paths.GetCount(); ++i)
        {
            systemPaths.push_back( std::string(plugin_paths[i]) + shaderSubFolder);
        }

        std::filesystem::path resultPath = FindResourcePath(systemPaths, localResources);

        if (resultPath.empty())
            return resultPath;

        return resultPath;
    }

};
