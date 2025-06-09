#pragma once
#include <dsc_common/dsc_common.h>

namespace DscCommon
{
	class VectorInt2;
}

namespace DscText
{
	class TextPreVertex;
	class ITextRun
	{
	public:
		virtual ~ITextRun() {};
		virtual void BuildPreVertexData(
			TextPreVertex& in_out_pre_vertex_data,
			DscCommon::VectorInt2& in_out_cursor,
			const bool in_width_limit_enabled,
			const int in_width_limit,
			const float in_ui_scale
		) = 0;

	};
}
