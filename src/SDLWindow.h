#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include "FontData.h"
#include "Graphs.h"

namespace graphtail
{

	struct Config;

	class SDLWindow
	{
	public:
				SDLWindow(
					Config*					aConfig);
				~SDLWindow();

		bool	Update();
		void	DrawGraphs(
					const Graphs&			aGraphs);

	private:

		const Config*				m_config;

		SDL_Window*					m_window;
		SDL_Renderer*				m_renderer;
		TTF_Font*					m_font;

		FontData					m_fontData;
		SDL_RWops*					m_fontDataRW;

		std::vector<SDL_Point>		m_tempGraphPoints;
		bool						m_windowIsDirty;
		uint32_t					m_lastDrawnGraphsVersion;

		SDL_Point					m_mousePosition;
		bool						m_mouseIsInWindow;

		void		_CreateStretchGraph(
						const Graphs::Data*		aData,
						int						aWindowWidth,
						uint32_t				aDataGroupWindowHeight,
						uint32_t				aDataGroupY,
						float					aValueMin,
						float					aValueRange,
						float&					aOutCursorValue,
						int&					aOutCursorX);
		void		_CreateFixedXStepGraph(
						const Graphs::Data*		aData,
						int						aWindowWidth,
						uint32_t				aDataGroupWindowHeight,
						uint32_t				aDataGroupY,
						float					aValueMin,
						float					aValueRange,
						uint32_t				aXStep,
						float&					aOutCursorValue,
						int&					aOutCursorX);
		void		_DrawText(
						int						aX,
						int						aY,
						const SDL_Color&		aColor,
						const char*				aFormat,
						...);
		SDL_Color	_GetHistogramColor(
						float					aValue,
						float					aMin,
						float					aMax);
	};

}