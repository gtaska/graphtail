#include "Base.h"

#include "Config.h"
#include "ErrorUtils.h"
#include "Graphs.h"
#include "SDLWindow.h"
#include "StringUtils.h"

namespace
{

	std::string
	FloatToString(
		float			aValue)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%f", aValue);

		// Remove trailing zeroes
		size_t length = strlen(buffer);

		for(size_t i = 0; i < length; i++)
		{
			char* p = &buffer[length - i - 1];

			if(*p == '0')
			{
				*p = '\0';
			}
			else if(*p == '.')
			{
				*p = '\0';
				break;
			}
			else
			{
				break;
			}
		}

		return buffer;
	}

}

namespace graphtail
{

	SDLWindow::SDLWindow(
		Config*			aConfig)
		: m_config(aConfig)
	{
		{
			int result = SDL_Init(SDL_INIT_VIDEO);
			GRAPHTAIL_CHECK(result == 0, "SDL_Init() failed: %s", SDL_GetError());
		}

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

		{
			int result = TTF_Init();
			GRAPHTAIL_CHECK(result == 0, "TTF_Init() failed: %s", TTF_GetError());
		}

		m_fontDataRW = SDL_RWFromConstMem(m_fontData.GetData(), (int)m_fontData.GetSize());
		GRAPHTAIL_ASSERT(m_fontDataRW != NULL);

		m_font = TTF_OpenFontRW(m_fontDataRW, SDL_TRUE, 16);
		GRAPHTAIL_CHECK(m_font != NULL, "TTF_OpenFontRW() failed: %s", TTF_GetError());
	}
	
	SDLWindow::~SDLWindow()
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
		int windowWidth;
		int windowHeight;
		SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);

		m_tempGraphPoints.resize((size_t)windowWidth);

		const std::vector<std::unique_ptr<Graphs::DataGroup>>& dataGroups = aGraphs.GetDataGroups();

		if(dataGroups.size() > 0)
		{
			uint32_t dataGroupWindowHeight = windowHeight / (uint32_t)dataGroups.size();

			uint32_t dataGroupY = 0;
			bool alternatingBackground = false;

			size_t colorIndex = 0;

			for(const std::unique_ptr<Graphs::DataGroup>& dataGroup : dataGroups)
			{			
				{
					if(alternatingBackground)
						SDL_SetRenderDrawColor(m_renderer, 32, 32, 32, 255);
					else
						SDL_SetRenderDrawColor(m_renderer, 42, 42, 42, 255);

					alternatingBackground = !alternatingBackground;

					SDL_Rect rect;
					rect.x = 0;
					rect.y = (int)dataGroupY + 1;
					rect.w = windowWidth;
					rect.h = (int)dataGroupWindowHeight;
					SDL_RenderFillRect(m_renderer, &rect);
				}

				int textY = dataGroupY + 1;

				if(dataGroup->m_data.size() > 0)
				{
					for(const std::unique_ptr<Graphs::Data>& data : dataGroup->m_data)
					{
						if(data->m_values.size() == 0)
							continue;

						float valueRange = data->m_max - data->m_min;

						const Config::Color& color = m_config->m_graphColors[colorIndex % m_config->m_graphColors.size()];

						char infoBuffer[256];
						if(data->m_values.size() > 0)
						{
							snprintf(infoBuffer, sizeof(infoBuffer), " avg:%s min:%s max:%s", 
								FloatToString(data->m_sum / (float)data->m_values.size()).c_str(),
								FloatToString(data->m_min).c_str(),
								FloatToString(data->m_max).c_str());
						}
						else
						{
							infoBuffer[0] = '\0';
						}

						_DrawText(0, textY, SDL_Color{ (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255 }, "%s%s", 
							data->m_id.c_str(), 
							infoBuffer);

						SDL_SetRenderDrawColor(m_renderer, (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255);

						if(valueRange > 0)
						{
							m_tempGraphPoints.clear();

							if(data->m_values.size() < (size_t)windowWidth)
							{
								for (size_t i = 0; i < data->m_values.size(); i++)
								{
									int x = ((int)i * windowWidth) / (int)(data->m_values.size() - 1);
									int y = (int)dataGroupWindowHeight - (int)(((data->m_values[i] - data->m_min) / valueRange) * (float)dataGroupWindowHeight) + (int)dataGroupY;

									m_tempGraphPoints.push_back({ x, y });
								}
							}
							else
							{
								for(int x = 0; x < windowWidth; x++)
								{
									size_t i = ((size_t)x * data->m_values.size()) / (size_t)windowHeight;
									GRAPHTAIL_ASSERT(i < data->m_values.size());
									int y = (int)dataGroupWindowHeight - (int)(((data->m_values[i] - data->m_min) / valueRange) * (float)dataGroupWindowHeight) + (int)dataGroupY;

									m_tempGraphPoints.push_back({ x, y });
								}
							}

							SDL_RenderDrawLines(m_renderer, &m_tempGraphPoints[0], (int)m_tempGraphPoints.size());
						}

						textY += 20;
					}
				}
				else
				{
					const Config::Color& color = m_config->m_graphColors[colorIndex % m_config->m_graphColors.size()];

					_DrawText(0, textY, SDL_Color{ (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255 }, "No data to show.");
				}

				dataGroupY += dataGroupWindowHeight;

				colorIndex++;
			}
		}
		else
		{
			SDL_SetRenderDrawColor(m_renderer, 32, 32, 32, 255);
			SDL_RenderClear(m_renderer);
			_DrawText(0, 0, SDL_Color{ 255, 255, 255, 255 }, "No data to show.");
		}
	}

	void	
	SDLWindow::Present()
	{
		SDL_RenderPresent(m_renderer);
	}

	//----------------------------------------------------------------------------------------

	void	
	SDLWindow::_DrawText(
		int					aX,
		int					aY,
		const SDL_Color&	aColor,
		const char*			aFormat,
		...)
	{
		char buffer[1024];
		GRAPHTAIL_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aFormat);

		// Remove unsupported characters from buffer
		char fixedBuffer[1024];

		{
			char* pIn = buffer;
			char* pOut = fixedBuffer;
			while (*pIn != '\0')
			{
				char c = *pIn++;

				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' || c == ' ' || c == '-' || c == '_' || c == '.')
				{
					*pOut = c;
					pOut++;
				}
			}

			*pOut = '\0';
		}

		SDL_Surface* surface = TTF_RenderText_Blended(m_font, fixedBuffer, aColor);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);

		SDL_Rect rect;
		rect.x = aX;
		rect.y = aY;
		rect.w = surface->w;
		rect.h = surface->h;

		SDL_RenderCopy(m_renderer, texture, NULL, &rect);

		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);
	}

}