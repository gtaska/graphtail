#include "Base.h"

#include "ErrorUtils.h"
#include "Wildcard.h"

namespace graphtail
{

	Wildcard::Wildcard(
		const char*		aWildcard)
		: m_anyEnd(false)
	{
		if(aWildcard != NULL)
			Set(aWildcard);
	}
	
	Wildcard::~Wildcard()
	{

	}

	void	
	Wildcard::Set(
		const char*		aWildcard)
	{
		// Parse wildcard string
		m_parts.clear();
		m_anyEnd = false;

		if(*aWildcard == '\0')
			return;

		Part part;
		const char* p = aWildcard;

		for(;;)
		{
			char c = *p++;

			if(c == '*')
			{			
				if(part.m_matchLen > 0)
				{
					m_parts.push_back(part);
					part.m_match.clear();
					part.m_matchLen = 0;
				}

				part.m_anyPrefix = true;
			}
			else if(c == '\0')
			{
				if(part.m_matchLen > 0)
					m_parts.push_back(part);

				break;
			}
			else 
			{
				part.m_match += c;
				part.m_matchLen++;
			}
		}

		m_anyEnd = part.m_matchLen == 0 && part.m_anyPrefix;
	}

	bool	
	Wildcard::Match(
		const char*		aString) const
	{
		const char* p = aString;
		int remaining = (int)strlen(aString);

		for(const Part& part : m_parts)
		{
			GRAPHTAIL_ASSERT(part.m_matchLen > 0);

			if(part.m_anyPrefix)
			{
				int offset = _Find(p, remaining, part.m_match.c_str(), part.m_matchLen);
				if(offset == -1)
					return false;

				p += offset;
				remaining -= offset;
			}
			else
			{
				if(!_Match(p, remaining, part.m_match.c_str(), part.m_matchLen))
					return false;
			}

			p += part.m_matchLen;
			remaining -= part.m_matchLen;
		}

		if(*p != '\0' && !m_anyEnd)
			return false;

		return true;
	}

	//----------------------------------------------------------------------

	int		
	Wildcard::_Find(
		const char*		aString,
		int				aStringLen,
		const char*		aMatch,
		int				aMatchLen) const
	{
		int n = aStringLen - aMatchLen;
		
		for(int i = 0; i <= n; i++)
		{
			if(_Match(aString + i, aStringLen - i, aMatch, aMatchLen))
				return i;
		}

		return -1;
	}

	bool
	Wildcard::_Match(
		const char*		aString,
		int				aStringLen,
		const char*		aMatch,
		int				aMatchLen) const
	{
		if(aStringLen < aMatchLen)
			return false;

		return memcmp(aString, aMatch, aMatchLen) == 0;
	}

}

