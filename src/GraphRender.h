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
						bool						aHover,
						bool						aForceXStretch);

	private:

		void		_CreateStretchGraph(
						RenderContext*				aDrawContext,
						const Graphs::Data*			aData,
						float						aValueMin,
						float						aValueRange,
						size_t&						aOutCursorIndex,
						int&						aOutCursorX);
		void		_CreateFixedXStepGraph(
						RenderContext*				aDrawContext,
						const Graphs::Data*			aData,
						float						aValueMin,
						float						aValueRange,
						int							aXStep,
						size_t&						aOutCursorIndex,
						int&						aOutCursorX);

		std::vector<SDL_Point>			m_tempGraphPoints;

		struct StickyCursor
		{
			const Graphs::DataGroup*	m_dataGroup = NULL;
			size_t						m_index = 0;
		};

		std::optional<StickyCursor>		m_stickyCursor;
	};

}