#pragma once

#include "StringUtils.h"

namespace graphtail
{

	struct Config;
	struct MouseState;

	struct RenderContext
	{
		RenderContext(
			const Config*			aConfig,
			TTF_Font*				aFont,
			SDL_Renderer*			aRenderer,
			const MouseState*	aMouseState)
			: m_config(aConfig)
			, m_font(aFont)
			, m_renderer(aRenderer)
			, m_mouseState(aMouseState)
		{
			
		}

		void
		DrawText(
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

		// Public data
		const Config*													m_config;
		TTF_Font*														m_font;
		SDL_Renderer*													m_renderer;
		const MouseState*											m_mouseState;

		int																m_windowWidth = 0;
		int																m_dataGroupWindowHeight = 0;
		int																m_dataGroupY = 0;
		size_t															m_colorIndex = 0;
	};

}