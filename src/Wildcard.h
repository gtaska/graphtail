#pragma once

namespace graphtail
{

	class Wildcard
	{
	public:
				Wildcard(
					const char*		aWildcard = NULL);
				~Wildcard();

		void	Set(
					const char*		aWildcard);
		bool	Match(
					const char*		aString) const;

	private:
		// Data
		struct Part
		{
			Part()
				: m_anyPrefix(false)
				, m_matchLen(0)
			{

			}

			bool			m_anyPrefix;
			std::string		m_match;
			int				m_matchLen;
		};

		std::vector<Part>	m_parts;
		bool				m_anyEnd;

		int		_Find(
					const char*		aString,
					int				aStringLen,
					const char*		aMatch,
					int				aMatchLen) const;
		bool	_Match(
					const char*		aString,
					int				aStringLen,
					const char*		aMatch,
					int				aMatchLen) const;

	};

}
