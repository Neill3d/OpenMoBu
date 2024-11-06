#pragma once


/// <summary>
/// a method to transfer shared library logs into motionbuilder logs output
/// </summary>
#define DEFINE_LOGI void LOGI(const char* pFormatString, ...) \
{ \
	constexpr int BUFFER_SIZE{ 512 }; \
	char buffer[BUFFER_SIZE]{ 0 }; \
	va_list args; \
	va_start(args, pFormatString); \
	vsnprintf(buffer, static_cast<size_t>(BUFFER_SIZE - 1), pFormatString, args); \
	FBTrace(buffer); \
	va_end(args); \
}

#define DEFINE_LOGE void LOGE(const char* pFormatString, ...) \
{ \
	constexpr int BUFFER_SIZE{ 512 }; \
	char buffer[BUFFER_SIZE]{ 0 }; \
	va_list args; \
	va_start(args, pFormatString); \
	vsnprintf(buffer, static_cast<size_t>(BUFFER_SIZE - 1), pFormatString, args); \
	FBTrace(buffer); \
	va_end(args); \
}

// declaration of log functions
extern void LOGI(const char* pFormatString, ...);
extern void LOGE(const char* pFormatString, ...);
