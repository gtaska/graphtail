#pragma once

#include "Wildcard.h"

namespace graphtail
{

	struct Config
	{
		struct Color
		{
			uint32_t								m_r = 0;
			uint32_t								m_g = 0;
			uint32_t								m_b = 0;
		};

		struct GroupConfig
		{
			bool	TrySetMember(
						const std::string&		aArgName,
						const std::string&		aValue);
			void	ApplyDefaults(
						const GroupConfig&		aDefaults);

			// Public data
			std::optional<uint32_t>					m_xStep;
			std::optional<float>					m_yMin;
			std::optional<float>					m_yMax;
			std::optional<float>					m_histogramThreshold;
			std::optional<bool>						m_isSize;
		};

		struct GroupHistogram
		{
			std::string								m_name;
			std::vector<std::string>				m_ids;
		};

		struct Group
		{
			std::vector<std::unique_ptr<Wildcard>>	m_idWildcards;
			GroupConfig								m_config;
			std::unique_ptr<GroupHistogram>			m_histogram;
		};

					Config(
						int						aNumArgs,
						char**					aArgs);
					~Config();

		// Public data
		char										m_rowDelimiter = '\n';
		char										m_columnDelimiter = ';';
		std::vector<std::string>					m_inputs;
		uint32_t									m_width = 1000;
		uint32_t									m_height = 500;
		std::vector<std::unique_ptr<Group>>			m_groups;
		uint32_t									m_fontSize = 14;
		GroupConfig									m_defaultGroupConfig;
		bool										m_showHelp = false;
		bool										m_showHelpMarkdown = false;

		std::vector<Color>							m_graphColors =
		{
			{ 255, 0, 0 },
			{ 0, 255, 0 },
			{ 64, 64, 255 },
			{ 255, 255, 0 },
			{ 255, 0, 255 },
			{ 0, 255, 255 }
		};

		std::vector<Color>							m_histogramColors =
		{
			{ 52, 129, 181 },
			{ 74, 92, 165 },
			{ 97, 64, 147 },
			{ 118, 46, 129 },
			{ 139, 42, 106 },
			{ 157, 52, 84 },
			{ 177, 74, 59 },
			{ 197, 98, 34 }
		};
	};

}
