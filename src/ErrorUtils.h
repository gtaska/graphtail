#pragma once

#define GRAPHTAIL_CHECK(_Condition, ...)									\
	do																		\
	{																		\
		if(!(_Condition))													\
			graphtail::ErrorUtils::Terminate("" __VA_ARGS__);				\
	} while(false)

#define GRAPHTAIL_ASSERT(_Condition, ...)									\
	do																		\
	{																		\
		if(!(_Condition))													\
			graphtail::ErrorUtils::AssertFailed(							\
				__FILE__,													\
				__LINE__,													\
				#_Condition, 												\
				"" __VA_ARGS__);											\
	} while(false)
	
#define GRAPHTAIL_FATAL_ERROR(...)											\
	graphtail::ErrorUtils::Terminate("" __VA_ARGS__)

namespace graphtail
{

	namespace ErrorUtils
	{

		void			Terminate(
							const char*			aFormat,
							...);
		void			AssertFailed(
							const char*			aFile,
							int					aLineNum,
							const char*			aAssertString,
							const char*			aMessageFormat,
							...);
		void			DebugBreak();

	}

}