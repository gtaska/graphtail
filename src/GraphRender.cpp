#include "Base.h"

#include "ErrorUtils.h"
#include "GraphRender.h"
#include "MouseState.h"
#include "RenderContext.h"
#include "StringUtils.h"

namespace graphtail
{

	GraphRender::GraphRender()
	{

	}

	GraphRender::~GraphRender()
	{

	}

	void	
	GraphRender::Draw(
		RenderContext*				aDrawContext,
		const Graphs::DataGroup*	aDataGroup,
		bool						aHover)
	{
		float valueMin = aDataGroup->GetMin();
		float valueMax = aDataGroup->GetMax();

		float valueRange = valueMax - valueMin;

		int textY = aDrawContext->m_dataGroupY + 1;

		for (const std::unique_ptr<Graphs::Data>& data : aDataGroup->m_data)
		{
			if (data->m_values.size() == 0)
				continue;

			const Config::Color& color = aDrawContext->m_config->m_graphColors[aDrawContext->m_colorIndex % aDrawContext->m_config->m_graphColors.size()];

			// Graph
			int cursorX = 0;
			float cursorValue = 0.0f;

			if (valueRange > 0)
			{
				m_tempGraphPoints.clear();

				if (aDataGroup->m_config != NULL && aDataGroup->m_config->m_config.m_xStep.has_value())
					_CreateFixedXStepGraph(aDrawContext, data.get(), valueMin, valueRange, (int)aDataGroup->m_config->m_config.m_xStep.value(), cursorValue, cursorX);
				else
					_CreateStretchGraph(aDrawContext, data.get(), valueMin, valueRange, cursorValue, cursorX);

				if (aHover)
				{
					SDL_SetRenderDrawColor(aDrawContext->m_renderer, 128, 128, 128, 255);
					SDL_RenderDrawLine(aDrawContext->m_renderer, cursorX, aDrawContext->m_dataGroupY, cursorX, aDrawContext->m_dataGroupY + aDrawContext->m_dataGroupWindowHeight);
				}

				SDL_SetRenderDrawColor(aDrawContext->m_renderer, (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255);
				SDL_RenderDrawLines(aDrawContext->m_renderer, &m_tempGraphPoints[0], (int)m_tempGraphPoints.size());
			}

			// Text
			char infoBuffer[256];
			if (data->m_values.size() > 0)
			{
				char cursorValueBuffer[128];
				if (aHover)
					snprintf(cursorValueBuffer, sizeof(cursorValueBuffer), " cursor:%s", StringUtils::FloatToString(cursorValue).c_str());
				else
					cursorValueBuffer[0] = '\0';

				snprintf(infoBuffer, sizeof(infoBuffer), " avg:%s min:%s max:%s%s",
					StringUtils::FloatToString(data->m_sum / (float)data->m_values.size()).c_str(),
					StringUtils::FloatToString(data->m_min).c_str(),
					StringUtils::FloatToString(data->m_max).c_str(),
					cursorValueBuffer);
			}
			else
			{
				infoBuffer[0] = '\0';
			}

			aDrawContext->DrawText(0, textY, SDL_Color{ (uint8_t)color.m_r, (uint8_t)color.m_g, (uint8_t)color.m_b, 255 }, "%s%s",
				data->m_id.c_str(),
				infoBuffer);

			aDrawContext->m_colorIndex++;

			textY += (int)aDrawContext->m_config->m_fontSize + 6;
		}
	}

	//-----------------------------------------------------------------------------------

	void		
	GraphRender::_CreateStretchGraph(
		RenderContext*			aDrawContext,
		const Graphs::Data*		aData,
		float					aValueMin,
		float					aValueRange,
		float&					aOutCursorValue,
		int&					aOutCursorX)
	{
		if (aData->m_values.size() < (size_t)aDrawContext->m_windowWidth && aData->m_values.size() > 1)
		{
			for (size_t i = 0; i < aData->m_values.size(); i++)
			{
				int x = ((int)i * aDrawContext->m_windowWidth) / (int)(aData->m_values.size() - 1);
				int y = aDrawContext->m_dataGroupWindowHeight - (int)(((aData->m_values[i] - aValueMin) / aValueRange) * (float)(aDrawContext->m_dataGroupWindowHeight - 1)) + aDrawContext->m_dataGroupY;

				if (x <= aDrawContext->m_mouseState->m_position.x + (aDrawContext->m_windowWidth / (int)aData->m_values.size()) / 2)
				{
					aOutCursorX = x;
					aOutCursorValue = aData->m_values[i];
				}

				m_tempGraphPoints.push_back({ x, y });
			}
		}
		else
		{
			for (int x = 0; x < aDrawContext->m_windowWidth; x++)
			{
				size_t i = ((size_t)x * aData->m_values.size()) / (size_t)aDrawContext->m_windowWidth;
				GRAPHTAIL_ASSERT(i < aData->m_values.size());
				int y = aDrawContext->m_dataGroupWindowHeight - (int)(((aData->m_values[i] - aValueMin) / aValueRange) * (float)(aDrawContext->m_dataGroupWindowHeight - 1)) + aDrawContext->m_dataGroupY;

				if (x <= aDrawContext->m_mouseState->m_position.x)
				{
					aOutCursorX = x;
					aOutCursorValue = aData->m_values[i];
				}

				m_tempGraphPoints.push_back({ x, y });
			}
		}

	}
	
	void		
	GraphRender::_CreateFixedXStepGraph(
		RenderContext*			aDrawContext,
		const Graphs::Data*		aData,
		float					aValueMin,
		float					aValueRange,
		int						aXStep,
		float&					aOutCursorValue,
		int&					aOutCursorX)
	{
		size_t iMin = 0;
		size_t iMax = aData->m_values.size() - 1;

		if (iMax * (size_t)aXStep > (size_t)aDrawContext->m_windowWidth)
			iMin = (iMax * (size_t)aXStep - (size_t)aDrawContext->m_windowWidth) / (size_t)aXStep;

		int x = 0;

		for (size_t i = iMin; i <= iMax; i++)
		{
			GRAPHTAIL_ASSERT(i < aData->m_values.size());
			int y = aDrawContext->m_dataGroupWindowHeight - (int)(((aData->m_values[i] - aValueMin) / aValueRange) * (float)(aDrawContext->m_dataGroupWindowHeight - 1)) + aDrawContext->m_dataGroupY;

			if (x < aDrawContext->m_mouseState->m_position.x + aXStep / 2)
			{
				aOutCursorX = x;
				aOutCursorValue = aData->m_values[i];
			}

			m_tempGraphPoints.push_back({ x, y });
			x += aXStep;
		}
	}

}