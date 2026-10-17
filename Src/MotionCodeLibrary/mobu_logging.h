#pragma once

#include <cstdlib> // for std::abort

/// <summary>
/// verbose log lines are printed only when the environment variable OPENMOBU_VERBOSE_LOG is set
///  to a non-empty value other than "0" before MotionBuilder starts (the value is read once)
/// </summary>
inline bool IsVerboseLogEnabledFromEnvironment()
{
	bool isEnabled = false;
#if defined(_MSC_VER)
	char* value = nullptr;
	size_t length = 0;
	if (_dupenv_s(&value, &length, "OPENMOBU_VERBOSE_LOG") == 0 && value)
	{
		isEnabled = (value[0] != '\0') && !(value[0] == '0' && value[1] == '\0');
		free(value);
	}
#else
	if (const char* value = std::getenv("OPENMOBU_VERBOSE_LOG"))
	{
		isEnabled = (value[0] != '\0') && !(value[0] == '0' && value[1] == '\0');
	}
#endif
	return isEnabled;
}

/// <summary>
/// a method to transfer shared library logs into motionbuilder logs output
/// </summary>
#define DEFINE_LOGV void LOGV(const char* pFormatString, ...) \
{ \
	static const bool sIsVerboseLogEnabled = IsVerboseLogEnabledFromEnvironment(); \
	if (!sIsVerboseLogEnabled) \
		return; \
	constexpr int BUFFER_SIZE{ 512 }; \
	char buffer[BUFFER_SIZE]{ 0 }; \
	va_list args; \
	va_start(args, pFormatString); \
	vsnprintf(buffer, static_cast<size_t>(BUFFER_SIZE - 1), pFormatString, args); \
	FBTrace("%s", buffer);\
	va_end(args); \
}

#define DEFINE_LOGI void LOGI(const char* pFormatString, ...) \
{ \
	constexpr int BUFFER_SIZE{ 512 }; \
	char buffer[BUFFER_SIZE]{ 0 }; \
	va_list args; \
	va_start(args, pFormatString); \
	vsnprintf(buffer, static_cast<size_t>(BUFFER_SIZE - 1), pFormatString, args); \
	FBTrace("%s", buffer);\
	va_end(args); \
}

#define DEFINE_LOGE void LOGE(const char* pFormatString, ...) \
{ \
	constexpr int BUFFER_SIZE{ 512 }; \
	char buffer[BUFFER_SIZE]{ 0 }; \
	va_list args; \
	va_start(args, pFormatString); \
	vsnprintf(buffer, static_cast<size_t>(BUFFER_SIZE - 1), pFormatString, args); \
	FBTrace("%s", buffer);\
	va_end(args); \
}

// declaration of log functions

// verbose, printed only when the OPENMOBU_VERBOSE_LOG environment variable is set (see IsVerboseLogEnabledFromEnvironment)
extern void LOGV(const char* pFormatString, ...);
// information line
extern void LOGI(const char* pFormatString, ...);
// error line
extern void LOGE(const char* pFormatString, ...);

#define ENSURE(expr) \
    do { \
        if (!(expr)) { \
        LOGE("Check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
		} \
    } while(0)

#define ENSURE_MSG(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            LOGE("Check failed: %s at %s:%d | " fmt, #expr, __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while(0)

#define VERIFY(expr) \
    do { \
        if (!(expr)) { \
        LOGE("Check failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
        std::abort(); \
		} \
    } while(0)

#define VERIFY_MSG(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            LOGE("Check failed: %s at %s:%d | " fmt, #expr, __FILE__, __LINE__, ##__VA_ARGS__); \
            std::abort(); \
        } \
    } while(0)
