#pragma once

#include "Graphs.h"

namespace graphtail
{

	struct RenderContext;

	class HistogramHeatmapRender
	{
	public:
					HistogramHeatmapRender();
					~HistogramHeatmapRender();

		void		Draw(
						RenderContext*				aDrawContext,
						const Graphs::DataGroup*	aDataGroup,
						bool						aHover);

	private:

		SDL_Color	_GetHistogramColor(
						const RenderContext*		aDrawContext,
						float						aValue,
						float						aMin,
						float						aMax);
	};



}