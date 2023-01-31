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

		struct Group
		{
			std::vector<std::unique_ptr<Wildcard>>	m_idWildcards;
		};

		Config(
			int		aNumArgs,
			char**	aArgs);
		~Config();

		// Public data
		char										m_rowDelimiter		= '\n';
		char										m_columnDelimiter	= ';';
		std::vector<std::string>					m_inputs;
		uint32_t									m_width = 1000;
		uint32_t									m_height = 500;
		std::vector<std::unique_ptr<Group>>			m_groups;
		uint32_t									m_fontSize = 14;

		std::vector<Color>							m_graphColors = 
		{ 
			{ 255, 0, 0 },
			{ 0, 255, 0 },
			{ 64, 64, 255 },
			{ 255, 255, 0 },
			{ 255, 0, 255 },
			{ 0, 255, 255 },
			{ 128, 0, 0 },
			{ 0, 128, 0 },
			{ 32, 32, 128 },
			{ 128, 128, 0 },
			{ 128, 0, 255 },
			{ 0, 128, 128 }
		};
	};

}
