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

		//virtual const DscCommon::VectorInt2 CalculateDesiredSize(const DscCommon::VectorInt2& in_avaliable_size) const;
		//virtual DscDag::NodeToken GetChildAvalableSizeNode(const int32 in_child_index) const;

		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size);
		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_our_desired_size, const int32 in_child_index) const;
		virtual const DscCommon::VectorInt2 GetChildActualSize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const;// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildOffset(const DscCommon::VectorInt2& in_our_desired_size, const int32 in_child_index) const;// may need to fetch the desired size of all the children first, ie, for stack
		virtual const int32 GetParentChildIndex() const = 0; // what child index are we of out parent

	};
}