#pragma once

namespace graphtail
{

	struct Config
	{
		struct Color
		{
			uint32_t	m_r = 0;
			uint32_t	m_g = 0;
			uint32_t	m_b = 0;
		};

		Config(
			int		/*aNumArgs*/,
			char**	/*aArgs*/)
		{

		}

		char						m_rowDelimiter		= '\n';
		char						m_columnDelimiter	= ';';
		std::vector<std::string>	m_inputs;
		uint32_t					m_width = 1000;
		uint32_t					m_height = 500;
		
		std::vector<Color>			m_graphColors = 
		{ 
			{ 255, 0, 0 },
			{ 0, 255, 0 },
			{ 0, 0, 255 },
			{ 255, 255, 0 },
			{ 255, 0, 255 },
			{ 0, 255, 255 },
			{ 128, 0, 0 },
			{ 0, 128, 0 },
			{ 0, 0, 128 },
			{ 128, 128, 0 },
			{ 128, 0, 255 },
			{ 0, 128, 128 }
		};
	};

}
