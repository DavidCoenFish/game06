#pragma once
#include "dsc_ui.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscRenderResource
{
	class Frame;
}

namespace DscUi
{
	class IUiComponent
	{
	public:
		virtual ~IUiComponent();

		virtual void Draw(DscRenderResource::Frame& in_frame, const DscCommon::VectorInt2& in_target_size) = 0;
	};
}