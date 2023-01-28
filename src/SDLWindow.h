#pragma once

#include <SDL.h>

namespace graphtail
{

	struct Config;

	class Graphs;

	class SDLWindow
	{
	public:
				SDLWindow(
					Config*			aConfig);
				~SDLWindow();

		bool	Update();
		void	DrawGraphs(
					const Graphs&	aGraphs);
		void	Present();

	private:

		const Config*	m_config;

		SDL_Window*		m_window;
		SDL_Renderer*	m_renderer;
	};

}