#pragma once
#include "dsc_ui.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class IRenderTarget;
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

		virtual void Draw(
			DscRenderResource::Frame& in_frame, 
			const DscCommon::VectorInt2& in_target_size//, 
			//const std::vector<DscRender::IRenderTarget*>& in_child_render_target_array
			) = 0;
		virtual const DscCommon::VectorFloat4& GetClearColour() const;

		virtual const DscCommon::VectorInt2 CalculateDesiredSize(const DscCommon::VectorInt2& in_avaliable_size) const;

		virtual DscDag::NodeToken GetChildAvalableSizeNode(const int32 in_child_index) const;

	};
}