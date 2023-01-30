#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include "FontData.h"

namespace graphtail
{

	struct Config;

	class Graphs;

	class SDLWindow
	{
	public:
				SDLWindow(
					Config*				aConfig);
				~SDLWindow();

		bool	Update();
		void	DrawGraphs(
					const Graphs&		aGraphs);
		void	Present();

	private:

		const Config*			m_config;

		SDL_Window*				m_window;
		SDL_Renderer*			m_renderer;
		TTF_Font*				m_font;

		FontData				m_fontData;
		SDL_RWops*				m_fontDataRW;

		std::vector<SDL_Point>	m_tempGraphPoints;

		void	_DrawText(
					int					aX,
					int					aY,
					const SDL_Color&	aColor,
					const char*			aFormat,
					...);
	};

}