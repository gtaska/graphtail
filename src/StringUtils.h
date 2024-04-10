#pragma once

#if defined(_WIN32)

	#define GRAPHTAIL_STRING_FORMAT_VARARGS(Buffer, BufferSize, Format)					\
		{																				\
			va_list _list;																\
			va_start(_list, Format);													\
			int n = vsnprintf_s(Buffer, BufferSize, _TRUNCATE, Format, _list);			\
			if(n < 0)																	\
				Buffer[0] = '\0';														\
			va_end(_list);																\
		}

#else

	#define GRAPHTAIL_STRING_FORMAT_VARARGS(Buffer, BufferSize, Format)					\
		{																				\
			va_list _list;																\
			va_start(_list, Format);													\
			int n = vsnprintf(Buffer, BufferSize, Format, _list);						\
			if(n < 0)																	\
				Buffer[0] = '\0';														\
			va_end(_list);																\
		}

#endif

namespace graphtail::StringUtils
{

	std::string		FloatToString(
						float			aValue,
						bool			aIsSize);

}
