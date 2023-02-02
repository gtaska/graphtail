#include "Base.h"

#include "StringUtils.h"

namespace graphtail::StringUtils
{

	std::string		
	FloatToString(
		float			aValue)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%f", aValue);

		// Remove trailing zeroes
		size_t length = strlen(buffer);

		for (size_t i = 0; i < length; i++)
		{
			char* p = &buffer[length - i - 1];

			if (*p == '0')
			{
				*p = '\0';
			}
			else if (*p == '.')
			{
				*p = '\0';
				break;
			}
			else
			{
				break;
			}
		}

		return buffer;
	}

}