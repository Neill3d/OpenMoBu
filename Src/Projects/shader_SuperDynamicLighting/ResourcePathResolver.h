
#pragma once

// ResourcePathResolver.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <string>
#include <vector>
#include <filesystem>

class IResourcePathResolver {
public:
    virtual ~IResourcePathResolver() = default;

    // Pure virtual function to be implemented by subclasses to find the resource path
    virtual std::filesystem::path FindResourcePath(const std::vector<std::string>& systemPaths, const std::vector<std::string>& localResources) const = 0;
};