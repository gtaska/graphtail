#pragma once

namespace graphtail
{

	struct MouseState
	{
		bool
		Update(
			SDL_Window*		aWindow)
		{
			SDL_Point mouse;
			SDL_GetGlobalMouseState(&mouse.x, &mouse.y);

			int windowX;
			int windowY;
			SDL_GetWindowPosition(aWindow, &windowX, &windowY);

			mouse.x -= windowX;
			mouse.y -= windowY;

			int windowWidth;
			int windowHeight;
			SDL_GetWindowSize(aWindow, &windowWidth, &windowHeight);

			bool mouseIsInWindow = mouse.x >= 0 && mouse.y >= 0 && mouse.x < windowWidth && mouse.y < windowHeight;

			bool updated = false;

			if (mouseIsInWindow != m_isInWindow)
			{
				updated = true;

				m_isInWindow = mouseIsInWindow;
			}

			if (mouse.x != m_position.x || mouse.y != m_position.y)
			{
				updated = true;

				m_isMoving = true;
				m_position = mouse;				
			}
			else
			{
				m_isMoving = false;
			}

			return updated;
		}

		// Public data
		SDL_Point					m_position;
		bool						m_isInWindow = false;
		bool						m_isMoving = false;
	};

}