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
		, m_lastDrawnGraphsVersion(0)
		, m_windowIsDirty(true)
		, m_mouseIsInWindow(false)
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
				default:			break;
				}
				break;

			default:
				break;
			}
		}
		
		// Mouse position
		{
			SDL_Point mouse;
			SDL_GetGlobalMouseState(&mouse.x, &mouse.y);

			int windowX;
			int windowY;
			SDL_GetWindowPosition(m_window, &windowX, &windowY);

			mouse.x -= windowX;
			mouse.y -= windowY;

			int windowWidth;
			int windowHeight;
			SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);

			bool mouseIsInWindow = mouse.x >= 0 && mouse.y >= 0 && mouse.x < windowWidth&& mouse.y < windowHeight;

			if(mouseIsInWindow != m_mouseIsInWindow)
			{
				m_windowIsDirty = true;

				m_mouseIsInWindow = mouseIsInWindow;
			}

			if(mouse.x != m_mousePosition.x || mouse.y != m_mousePosition.y)
			{
				m_windowIsDirty = true;

				m_mousePosition = mouse;
			}
		}

		return true;
	}

	void	
	SDLWindow::DrawGraphs(
		const Graphs&	aGraphs)
	{
		if(!m_windowIsDirty && m_lastDrawnGraphsVersion == aGraphs.GetVersion())
			return;

		m_lastDrawnGraphsVersion = aGraphs.GetVersion();
		m_windowIsDirty = false;

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
				bool mouseCursorInDataGroup = m_mouseIsInWindow && m_mousePosition.y >= (int)dataGroupY && (int)m_mousePosition.y < (int)dataGroupY + (int)dataGroupWindowHeight;

				{
					SDL_Rect rect;
					rect.x = 0;
					rect.y = (int)dataGroupY;
					rect.w = windowWidth;
					rect.h = (int)dataGroupWindowHeight + 1; 
					SDL_RenderSetClipRect(m_renderer, &rect);

					if (alternatingBackground)
						SDL_SetRenderDrawColor(m_renderer, 16, 16, 16, 255);
					else
						SDL_SetRenderDrawColor(m_renderer, 24, 24, 24, 255);

					alternatingBackground = !alternatingBackground;

					SDL_RenderFillRect(m_renderer, &rect);
				}

				int textY = dataGroupY + 1;

				if(dataGroup->m_data.size() > 0)
				{
					float valueMin = dataGroup->GetMin();
					float valueMax = dataGroup->GetMax();

					float valueRange = valueMax - valueMin;

					for(const std::unique_ptr<Graphs::Data>& data : dataGroup->m_data)
					{
						if(data->m_values.size() == 0)
							continue;

						const Config::Color& color = m_config->m_graphColors[colorIndex % m_config->m_graphColors.size()];

						// Graph
						int cursorX = 0;
						float cursorValue = 0.0f;

						if(valueRange > 0)
						{
							m_tempGraphPoints.clear();

							if(dataGroup->m_config != NULL && dataGroup->m_config->m_config.m_xStep.has_value())
								_CreateFixedXStepGraph(data.get(), windowWidth, dataGroupWindowHeight, dataGroupY, valueMin, valueRange, dataGroup->m_config->m_config.m_xStep.value(), cursorValue, cursorX);
							else
								_CreateStretchGraph(data.get(), windowWidth, dataGroupWindowHeight, dataGroupY, valueMin, valueRange, cursorValue, cursorX);

							if(mouseCursorInDataGroup)
							{
								SDL_SetRenderDrawColor(m_renderer, 128, 128, 128, 255);
								SDL_RenderDrawLine(m_renderer, cursorX, (int)dataGroupY, cursorX, (int)dataGroupY + (int)dataGroupWindowHeight);
							}

							SDL_SetRenderDrawColor(m_renderer, (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255);
							SDL_RenderDrawLines(m_renderer, &m_tempGraphPoints[0], (int)m_tempGraphPoints.size());
						}

						// Text
						char infoBuffer[256];
						if (data->m_values.size() > 0)
						{
							char cursorValueBuffer[128];
							if(mouseCursorInDataGroup)
								snprintf(cursorValueBuffer, sizeof(cursorValueBuffer), " cursor:%s", FloatToString(cursorValue).c_str());
							else
								cursorValueBuffer[0] = '\0';

							snprintf(infoBuffer, sizeof(infoBuffer), " avg:%s min:%s max:%s%s",
								FloatToString(data->m_sum / (float)data->m_values.size()).c_str(),
								FloatToString(data->m_min).c_str(),
								FloatToString(data->m_max).c_str(),
								cursorValueBuffer);
						}
						else
						{
							infoBuffer[0] = '\0';
						}

						_DrawText(0, textY, SDL_Color{ (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255 }, "%s%s",
							data->m_id.c_str(),
							infoBuffer);

						colorIndex++;

						textY += 20;
					}
				}
				else
				{
					const Config::Color& color = m_config->m_graphColors[colorIndex % m_config->m_graphColors.size()];

					_DrawText(0, textY, SDL_Color{ (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255 }, "No data to show.");

					colorIndex++;
				}

				dataGroupY += dataGroupWindowHeight;
			}

			SDL_RenderSetClipRect(m_renderer, NULL);
		}
		else
		{
			SDL_SetRenderDrawColor(m_renderer, 32, 32, 32, 255);
			SDL_RenderClear(m_renderer);

			_DrawText(0, 0, SDL_Color{ 255, 255, 255, 255 }, "No data to show.");
		}

		SDL_RenderPresent(m_renderer);
	}

	//----------------------------------------------------------------------------------------

	void	
	SDLWindow::_CreateStretchGraph(
		const Graphs::Data*		aData,
		int						aWindowWidth,
		uint32_t				aDataGroupWindowHeight,
		uint32_t				aDataGroupY,
		float					aValueMin,
		float					aValueRange,
		float&					aOutCursorValue,
		int&					aOutCursorX)
	{
		if (aData->m_values.size() < (size_t)aWindowWidth && aData->m_values.size() > 1)
		{
			for (size_t i = 0; i < aData->m_values.size(); i++)
			{
				int x = ((int)i * aWindowWidth) / (int)(aData->m_values.size() - 1);
				int y = (int)aDataGroupWindowHeight - (int)(((aData->m_values[i] - aValueMin) / aValueRange) * (float)aDataGroupWindowHeight) + (int)aDataGroupY;

				if(x <= m_mousePosition.x + ((int)aWindowWidth / (int)aData->m_values.size()) / 2)
				{
					aOutCursorX = x;
					aOutCursorValue = aData->m_values[i];
				}

				m_tempGraphPoints.push_back({ x, y });
			}
		}
		else
		{
			for (int x = 0; x < aWindowWidth; x++)
			{
				size_t i = ((size_t)x * aData->m_values.size()) / (size_t)aWindowWidth;
				GRAPHTAIL_ASSERT(i < aData->m_values.size());
				int y = (int)aDataGroupWindowHeight - (int)(((aData->m_values[i] - aValueMin) / aValueRange) * (float)aDataGroupWindowHeight) + (int)aDataGroupY;

				if (x <= m_mousePosition.x)
				{
					aOutCursorX = x;
					aOutCursorValue = aData->m_values[i];
				}

				m_tempGraphPoints.push_back({ x, y });
			}
		}

	}

	void	
	SDLWindow::_CreateFixedXStepGraph(
		const Graphs::Data*		aData,
		int						aWindowWidth,
		uint32_t				aDataGroupWindowHeight,
		uint32_t				aDataGroupY,
		float					aValueMin,
		float					aValueRange,
		uint32_t				aXStep,
		float&					aOutCursorValue,
		int&					aOutCursorX)
	{
		size_t iMin = 0;
		size_t iMax = aData->m_values.size() - 1;

		if(iMax * (size_t)aXStep > (size_t)aWindowWidth)
			iMin = (iMax * (size_t)aXStep - (size_t)aWindowWidth) / (size_t)aXStep;

		int x = 0;

		for (size_t i = iMin; i <= iMax; i++)
		{
			GRAPHTAIL_ASSERT(i < aData->m_values.size());
			int y = (int)aDataGroupWindowHeight - (int)(((aData->m_values[i] - aValueMin) / aValueRange) * (float)aDataGroupWindowHeight) + (int)aDataGroupY;

			if (x < m_mousePosition.x + (int)aXStep / 2)
			{
				aOutCursorX = x;
				aOutCursorValue = aData->m_values[i];
			}

			m_tempGraphPoints.push_back({ x, y });
			x += (int)aXStep;
		}
	}

	void	
	SDLWindow::_DrawText(
		int						aX,
		int						aY,
		const SDL_Color&		aColor,
		const char*				aFormat,
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