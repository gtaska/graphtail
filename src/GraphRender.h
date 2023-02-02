#pragma once

#include "Graphs.h"

namespace graphtail
{

	struct RenderContext;

	class GraphRender
	{
	public:
					GraphRender();
					~GraphRender();

		void		Draw(
						RenderContext*				aDrawContext,
						const Graphs::DataGroup*	aDataGroup,
						bool						aHover);

	private:

		void		_CreateStretchGraph(
						RenderContext*				aDrawContext,
						const Graphs::Data*			aData,
						float						aValueMin,
						float						aValueRange,
						float&						aOutCursorValue,
						int&						aOutCursorX);
		void		_CreateFixedXStepGraph(
						RenderContext*				aDrawContext,
						const Graphs::Data*			aData,
						float						aValueMin,
						float						aValueRange,
						int							aXStep,
						float&						aOutCursorValue,
						int&						aOutCursorX);

		std::vector<SDL_Point>		m_tempGraphPoints;
	};

}