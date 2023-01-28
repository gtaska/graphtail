#include "Base.h"

#include "Config.h"
#include "ErrorUtils.h"
#include "Graphs.h"
#include "SDLWindow.h"

namespace graphtail
{

	SDLWindow::SDLWindow(
		Config*			aConfig)
		: m_config(aConfig)
	{
		int result = SDL_Init(SDL_INIT_VIDEO);
		GRAPHTAIL_CHECK(result == 0, "SDL_Init() failed: %s", SDL_GetError());

		m_window = SDL_CreateWindow(
			"graphtail",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			(int)m_config->m_width,
			(int)m_config->m_height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		GRAPHTAIL_CHECK(m_window != NULL, "SDL_CreateWindow() failed: %s", SDL_GetError());

		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
		GRAPHTAIL_CHECK(m_renderer != NULL, "SDL_CreateRenderer() failed: %s", SDL_GetError());
	}
	
	SDLWindow::~SDLWindow()
	{
		GRAPHTAIL_ASSERT(m_window != NULL);
		GRAPHTAIL_ASSERT(m_renderer != NULL);

		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	bool	
	SDLWindow::Update()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) 
		{
			switch(event.type)
			{
			case SDL_QUIT:	
				return false;

			default:
				break;
			}
		}

		return true;
	}

	void	
	SDLWindow::DrawGraphs(
		const Graphs&	aGraphs)
	{
		uint32_t windowWidth = 0;
		uint32_t windowHeight = 0;

		{
			int w;
			int h;
			SDL_GetWindowSize(m_window, &w, &h);
			windowWidth = (uint32_t)w;
			windowHeight = (uint32_t)h;
		}

		const std::vector<std::unique_ptr<Graphs::DataGroup>>& dataGroups = aGraphs.GetDataGroups();

		if(dataGroups.size() > 0)
		{
			uint32_t dataGroupWindowHeight = windowHeight / (uint32_t)dataGroups.size();

			uint32_t y = 0;
			bool alternatingBackground = false;

			size_t colorIndex = 0;

			for(const std::unique_ptr<Graphs::DataGroup>& dataGroup : dataGroups)
			{			
				{
					if(alternatingBackground)
						SDL_SetRenderDrawColor(m_renderer, 32, 32, 32, 255);
					else
						SDL_SetRenderDrawColor(m_renderer, 48, 48, 48, 255);

					alternatingBackground = !alternatingBackground;

					SDL_Rect rect;
					rect.x = 0;
					rect.y = (int)y + 1;
					rect.w = (int)windowWidth;
					rect.h = (int)dataGroupWindowHeight;
					SDL_RenderFillRect(m_renderer, &rect);
				}

				for(const std::unique_ptr<Graphs::Data>& data : dataGroup->m_data)
				{
					float valueRange = data->m_max - data->m_min;

					const Config::Color& color = m_config->m_graphColors[colorIndex % m_config->m_graphColors.size()];

					SDL_SetRenderDrawColor(m_renderer, (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255);

					if(valueRange > 0)
					{
						for (size_t i = 1; i < data->m_values.size(); i++)
						{
							int prevX = ((int)(i - 1) * windowWidth) / (int)(data->m_values.size() - 1);
							int prevY = (int)dataGroupWindowHeight - (int)(((data->m_values[i - 1] - data->m_min) / valueRange) * (float)dataGroupWindowHeight) + (int)y;

							int currX = ((int)i * windowWidth) / (int)(data->m_values.size() - 1);
							int currY = (int)dataGroupWindowHeight - (int)(((data->m_values[i] - data->m_min) / valueRange) * (float)dataGroupWindowHeight) + (int)y;

							SDL_RenderDrawLine(m_renderer, prevX, prevY, currX, currY);
						}
					}
				}

				y += dataGroupWindowHeight;

				colorIndex++;
			}
		}
	}

	void	
	SDLWindow::Present()
	{
		SDL_RenderPresent(m_renderer);
	}

}