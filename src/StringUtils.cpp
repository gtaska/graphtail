#include "Base.h"

#include "StringUtils.h"

namespace graphtail::StringUtils
{

	std::string		
	FloatToString(
		float			aValue, 
		bool			aIsSize)
	{
		char buffer[256];

		if(aIsSize)
		{
			if(aValue > 1024.0f * 1024.0f * 1024.0f)
				snprintf(buffer, sizeof(buffer), "%.1fG", aValue / (1024.0f * 1024.0f * 1024.0f));
			else if (aValue > 1024.0f * 1024.0f)
				snprintf(buffer, sizeof(buffer), "%.1fM", aValue / (1024.0f * 1024.0f));
			else if (aValue > 1024.0f)
					snprintf(buffer, sizeof(buffer), "%.1fK", aValue / (1024.0f));
			else
				snprintf(buffer, sizeof(buffer), "%f", aValue);
		}
		else
		{
			snprintf(buffer, sizeof(buffer), "%f", aValue);
		}

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