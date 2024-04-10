#pragma once

namespace graphtail
{

	class Help
	{
	public:
				Help();
				~Help();

		void	PrintCommandLine() const;
		void	PrintMarkdown() const;

	private:

		struct Entry
		{
			std::vector<std::string>			m_options;
			std::vector<std::string>			m_description;
			bool								m_group;
		};

		std::vector<std::unique_ptr<Entry>>		m_entries;

		void	_DefineEntry(
					bool							aGroup,
					const std::vector<std::string>&	aOptions,
					const std::vector<std::string>&	aDescription);
		void	_PrintIndent() const;
	};

}
