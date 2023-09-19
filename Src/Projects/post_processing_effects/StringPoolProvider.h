#pragma once

#include <string_view>
#include "libraryMain.h"

// forward
class CStringPoolProviderImpl;

/// <summary>
/// The wrapper around StringPool to hide template class and provide simple interface to work with pool
/// </summary>
class POST_PROCESSING_EFFECTS_API CStringPoolProvider
{
public:
	CStringPoolProvider();

	std::string_view AddString(std::string_view str);

private:
	CStringPoolProviderImpl* impl{ nullptr };
};
