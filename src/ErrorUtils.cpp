#include "Base.h"

#if defined(_WIN32)
	#include <windows.h>
#endif

#include "ErrorUtils.h"
#include "StringUtils.h"

namespace graphtail
{

	namespace ErrorUtils
	{

		void
		Terminate(
			const char*		aFormat,
			...)
		{
			char buffer[2048];
			GRAPHTAIL_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aFormat);
			fprintf(stderr, "%s\n", buffer);

			#if defined(_WIN32)
				if(IsDebuggerPresent())
					DebugBreak();
			#endif

			exit(EXIT_FAILURE);
		}

		void
		AssertFailed(
			const char*		aFile,
			int				aLineNum,
			const char*		aAssertString,
			const char*		aMessageFormat,
			...)
		{
			fprintf(stderr, "ASSERT FAILED: %s:%d: %s\n", aFile, aLineNum, aAssertString);

			char buffer[2048];
			GRAPHTAIL_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aMessageFormat);
			if(buffer[0] != '\0')
				fprintf(stderr, "%s\n", buffer);

			#if defined(_DEBUG)
				DebugBreak();
			#else
				#if defined(_WIN32)
					if (IsDebuggerPresent())
						DebugBreak();
				#endif

				exit(EXIT_FAILURE);
			#endif
		}

		void
		DebugBreak()
		{
			#if defined(_WIN32)
				__debugbreak();
			#else
				raise(SIGINT);
			#endif
		}
	}

}
