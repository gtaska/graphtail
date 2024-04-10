#include "Base.h"

#include "ErrorUtils.h"
#include "HistogramHeatmapRender.h"
#include "MouseState.h"
#include "RenderContext.h"

namespace graphtail
{

	HistogramHeatmapRender::HistogramHeatmapRender()
	{

	}

	HistogramHeatmapRender::~HistogramHeatmapRender()
	{

	}

	void
	HistogramHeatmapRender::Draw(
		RenderContext*				aDrawContext,
		const Graphs::DataGroup*	aDataGroup,
		bool						aHover)
	{
		int textY = aDrawContext->m_dataGroupY + 1;

		bool isSize = aDataGroup->m_config->m_config.m_isSize.has_value() && aDataGroup->m_config->m_config.m_isSize.value();

		GRAPHTAIL_ASSERT(aDataGroup->m_data.size() == 1);
		const Graphs::Data* histogramData = aDataGroup->m_data[0].get();
		if (histogramData->m_values.size() > 0)
		{
			GRAPHTAIL_ASSERT(aDataGroup->m_config->m_histogram->m_ids.size() > 0);
			size_t histogramStepCount = histogramData->m_values.size() / aDataGroup->m_config->m_histogram->m_ids.size();
			if (histogramData->m_values.size() % aDataGroup->m_config->m_histogram->m_ids.size())
				histogramStepCount++;

			int xStep = 15;
			if (aDataGroup->m_config->m_config.m_xStep.has_value())
				xStep = (int)aDataGroup->m_config->m_config.m_xStep.value();

			int histogramWidth = (int)histogramStepCount * xStep;

			int x = histogramWidth < aDrawContext->m_windowWidth ? histogramWidth - xStep : aDrawContext->m_windowWidth - xStep;

			float cursorValue = 0.0f;
			const char* cursorId = NULL;

			for (size_t i = 0; i < histogramStepCount && x > -xStep; i++)
			{
				size_t valueIndex = (histogramStepCount - i - 1) * aDataGroup->m_config->m_histogram->m_ids.size();

				for (size_t j = 0; j < aDataGroup->m_config->m_histogram->m_ids.size() && valueIndex < histogramData->m_values.size(); j++)
				{
					float value = histogramData->m_values[valueIndex++];

					if (aDataGroup->m_config->m_config.m_histogramThreshold.has_value() && value <= aDataGroup->m_config->m_config.m_histogramThreshold.value())
						continue;

					SDL_Rect rect;
					rect.x = x;
					rect.y = (aDrawContext->m_dataGroupWindowHeight * (int)j) / (int)aDataGroup->m_config->m_histogram->m_ids.size();
					rect.w = xStep;
					rect.h = aDrawContext->m_dataGroupWindowHeight / (int)aDataGroup->m_config->m_histogram->m_ids.size() + 1;

					SDL_Color color = _GetHistogramColor(aDrawContext, value, aDataGroup->GetMin(), aDataGroup->GetMax());

					if (aHover
						&& aDrawContext->m_mouseState->m_position.x >= rect.x
						&& aDrawContext->m_mouseState->m_position.y >= rect.y
						&& aDrawContext->m_mouseState->m_position.x < rect.x + rect.w
						&& aDrawContext->m_mouseState->m_position.y < rect.y + rect.h)
					{
						color.r = (uint8_t)std::min<uint32_t>(((uint32_t)color.r * 5) / 4, 255);
						color.g = (uint8_t)std::min<uint32_t>(((uint32_t)color.g * 5) / 4, 255);
						color.b = (uint8_t)std::min<uint32_t>(((uint32_t)color.b * 5) / 4, 255);

						cursorValue = value;
						cursorId = aDataGroup->m_config->m_histogram->m_ids[j].c_str();
					}

					SDL_SetRenderDrawColor(aDrawContext->m_renderer, color.r, color.g, color.b, color.a);
					SDL_RenderFillRect(aDrawContext->m_renderer, &rect);
				}

				x -= xStep;
			}

			char infoBuffer[256];
			if (cursorId != NULL)
				snprintf(infoBuffer, sizeof(infoBuffer), " %s:%s", cursorId, StringUtils::FloatToString(cursorValue, isSize).c_str());
			else
				infoBuffer[0] = '\0';

			aDrawContext->DrawText(RenderContext::DRAW_TEXT_ALIGN_TOP_LEFT, 0, textY, SDL_Color{ 255, 255, 255, 255 }, "%s%s",
				aDataGroup->m_config->m_histogram->m_name.c_str(),
				infoBuffer);
		}
		else
		{
			aDrawContext->DrawText(RenderContext::DRAW_TEXT_ALIGN_TOP_LEFT, 0, textY, SDL_Color{ 255, 255, 255, 255 }, "No data to show.");
		}
	}

	//----------------------------------------------------------------------------------------

	SDL_Color
	HistogramHeatmapRender::_GetHistogramColor(
		const RenderContext*	aDrawContext,
		float					aValue,
		float					aMin,
		float					aMax)
	{
		GRAPHTAIL_ASSERT(aDrawContext->m_config->m_histogramColors.size() > 1);

		float range = aMax - aMin;
		if(range <= 0.0f)
			return SDL_Color{ 0, 0, 0, 255 };

		float f = (aValue - aMin) / range;

		if(f < 0.0f)
		{
			const Config::Color& c = aDrawContext->m_config->m_histogramColors[0];
			return SDL_Color{ (uint8_t)c.m_r, (uint8_t)c.m_g, (uint8_t)c.m_b, 255 };
		}

		float i = (float)(aDrawContext->m_config->m_histogramColors.size() - 1) * f;
		float iFloor = floorf(i);
		size_t i0 = (size_t)iFloor;
		float s = (i - iFloor);
		size_t i1 = i0 + 1;

		if(i1 >= aDrawContext->m_config->m_histogramColors.size())
		{
			const Config::Color& c = aDrawContext->m_config->m_histogramColors[aDrawContext->m_config->m_histogramColors.size() - 1];
			return SDL_Color{ (uint8_t)c.m_r, (uint8_t)c.m_g, (uint8_t)c.m_b, 255 };
		}

		const Config::Color& c0 = aDrawContext->m_config->m_histogramColors[i0];
		const Config::Color& c1 = aDrawContext->m_config->m_histogramColors[i1];

		int r = std::min<int>(255, (int)((float)c0.m_r * (1.0f - s) + (float)c1.m_r * s));
		int g = std::min<int>(255, (int)((float)c0.m_g * (1.0f - s) + (float)c1.m_g * s));
		int b = std::min<int>(255, (int)((float)c0.m_b * (1.0f - s) + (float)c1.m_b * s));

		return SDL_Color{ (uint8_t)r, (uint8_t)g, (uint8_t)b, 255 };
	}

}
