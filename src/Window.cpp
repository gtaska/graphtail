#include "Base.h"

#include "Config.h"
#include "ErrorUtils.h"
#include "GraphRender.h"
#include "Graphs.h"
#include "HistogramHeatmapRender.h"
#include "RenderContext.h"
#include "StringUtils.h"
#include "Window.h"

namespace graphtail
{

	Window::Window(
		Config*			aConfig)
		: m_config(aConfig)
		, m_lastDrawnGraphsVersion(0)
		, m_windowIsDirty(true)
		, m_forceXStretch(false)
	{
		{
			int result = SDL_Init(SDL_INIT_VIDEO);
			GRAPHTAIL_CHECK(result == 0, "SDL_Init() failed: %s", SDL_GetError());
		}

		std::string windowTitle;

		{
			std::stringstream s;
			s << "graphtail";
			for(const std::string& input : aConfig->m_inputs)
				s << " - " << input;

			windowTitle = s.str();
		}

		m_window = SDL_CreateWindow(
			windowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			(int)m_config->m_width,
			(int)m_config->m_height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		GRAPHTAIL_CHECK(m_window != NULL, "SDL_CreateWindow() failed: %s", SDL_GetError());

		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
		GRAPHTAIL_CHECK(m_renderer != NULL, "SDL_CreateRenderer() failed: %s", SDL_GetError());

		{
			int result = TTF_Init();
			GRAPHTAIL_CHECK(result == 0, "TTF_Init() failed: %s", TTF_GetError());
		}

		m_fontDataRW = SDL_RWFromConstMem(m_fontData.GetData(), (int)m_fontData.GetSize());
		GRAPHTAIL_ASSERT(m_fontDataRW != NULL);

		m_font = TTF_OpenFontRW(m_fontDataRW, SDL_TRUE, (int)m_config->m_fontSize);
		GRAPHTAIL_CHECK(m_font != NULL, "TTF_OpenFontRW() failed: %s", TTF_GetError());
	}

	Window::~Window()
	{
		GRAPHTAIL_ASSERT(m_window != NULL);
		GRAPHTAIL_ASSERT(m_renderer != NULL);
		GRAPHTAIL_ASSERT(m_font != NULL);

		TTF_CloseFont(m_font);
		SDL_FreeRW(m_fontDataRW);
		TTF_Quit();
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	bool
	Window::Update()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0)
		{
			switch(event.type)
			{
			case SDL_QUIT:
				return false;

			case SDL_WINDOWEVENT:
			case SDL_DISPLAYEVENT:
			case SDL_RENDER_TARGETS_RESET:
			case SDL_RENDER_DEVICE_RESET:
				m_windowIsDirty = true;
				break;

			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
				case SDLK_ESCAPE:	return false;
				case SDLK_F1:		m_forceXStretch = !m_forceXStretch; m_windowIsDirty = true; break;
				default:			break;
				}
				break;

			default:
				break;
			}
		}

		if(m_mouseState.Update(m_window))
		{
			// If mouse moves we need to redraw since it hover over something
			m_windowIsDirty = true;
		}

		return true;
	}

	void
	Window::DrawGraphs(
		const Graphs&	aGraphs)
	{
		if(!m_windowIsDirty && m_lastDrawnGraphsVersion == aGraphs.GetVersion())
			return;

		m_lastDrawnGraphsVersion = aGraphs.GetVersion();
		m_windowIsDirty = false;

		RenderContext context(m_config, m_font, m_renderer, &m_mouseState);

		int windowHeight = 0;
		SDL_GetWindowSize(m_window, &context.m_windowWidth, &windowHeight);

		const std::vector<std::unique_ptr<Graphs::DataGroup>>& dataGroups = aGraphs.GetDataGroups();

		if(dataGroups.size() > 0)
		{
			context.m_dataGroupWindowHeight = windowHeight / (int)dataGroups.size();

			bool alternatingBackground = false;

			for(const std::unique_ptr<Graphs::DataGroup>& dataGroup : dataGroups)
			{
				bool mouseCursorInDataGroup = m_mouseState.m_isInWindow && m_mouseState.m_position.y >= context.m_dataGroupY && m_mouseState.m_position.y < context.m_dataGroupY + context.m_dataGroupWindowHeight;

				// Draw group background and set up clipping
				{
					SDL_Rect rect;
					rect.x = 0;
					rect.y = context.m_dataGroupY;
					rect.w = context.m_windowWidth;
					rect.h = context.m_dataGroupWindowHeight + 1;
					SDL_RenderSetClipRect(m_renderer, &rect);

					if (alternatingBackground)
						SDL_SetRenderDrawColor(m_renderer, 16, 16, 16, 255);
					else
						SDL_SetRenderDrawColor(m_renderer, 24, 24, 24, 255);

					alternatingBackground = !alternatingBackground;

					SDL_RenderFillRect(m_renderer, &rect);
				}

				if(dataGroup->m_config->m_histogram)
				{
					// Data group is a histogram heatmap
					m_histogramHeatmapRender.Draw(&context, dataGroup.get(), mouseCursorInDataGroup);
				}
				else
				{
					// Data group is a bunch of normal line graphs
					if (dataGroup->m_data.size() > 0)
					{
						m_graphRender.Draw(&context, dataGroup.get(), mouseCursorInDataGroup, m_forceXStretch);
					}
					else
					{
						const Config::Color& color = m_config->m_graphColors[context.m_colorIndex % m_config->m_graphColors.size()];

						context.DrawText(RenderContext::DRAW_TEXT_ALIGN_TOP_LEFT, 0, 1, SDL_Color{ (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255 }, "No data to show.");

						context.m_colorIndex++;
					}
				}

				context.m_dataGroupY += context.m_dataGroupWindowHeight;
			}

			SDL_RenderSetClipRect(m_renderer, NULL);
		}
		else
		{
			SDL_SetRenderDrawColor(m_renderer, 32, 32, 32, 255);
			SDL_RenderClear(m_renderer);

			context.DrawText(RenderContext::DRAW_TEXT_ALIGN_TOP_LEFT, 0, 0, SDL_Color{ 255, 255, 255, 255 }, "No data to show.");
		}

		if(m_forceXStretch)
			context.DrawText(RenderContext::DRAW_TEXT_ALIGN_BOTTOM_RIGHT, 0, 0, SDL_Color{ 255, 255, 255, 255 }, "Fit");

		SDL_RenderPresent(m_renderer);
	}

}
