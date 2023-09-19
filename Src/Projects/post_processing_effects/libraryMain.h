#pragma once

#ifdef POST_PROCESSING_EFFECTS_EXPORT
#define POST_PROCESSING_EFFECTS_API __declspec(dllexport)
#else
#define POST_PROCESSING_EFFECTS_API __declspec(dllimport)
#endif