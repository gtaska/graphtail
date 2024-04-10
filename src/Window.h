#pragma once

#include "FontData.h"
#include "GraphRender.h"
#include "Graphs.h"
#include "HistogramHeatmapRender.h"
#include "MouseState.h"

namespace graphtail
{

	struct Config;

	class Window
	{
	public:
				Window(
					Config*					aConfig);
				~Window();

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

		bool						m_windowIsDirty;
		uint32_t					m_lastDrawnGraphsVersion;

		GraphRender					m_graphRender;
		HistogramHeatmapRender		m_histogramHeatmapRender;
		MouseState					m_mouseState;

		bool						m_forceXStretch;

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
