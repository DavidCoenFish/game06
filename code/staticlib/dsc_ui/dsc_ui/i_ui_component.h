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
	class HeapWrapperItem;
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

		// so, components that want to have a desired size other than the avalaible size will not play nice with being top level ui components, that has the render viewport of the externaliily provided size
		// workaround is to just nest in a canvas or other accomidating component (padding? stack?)
		//virtual const bool IsAllowedToBeTopLevelUiComponent() = 0;

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
			);

		virtual void SetClearColour(const DscCommon::VectorFloat4& in_colour);

		// this should be set by the parent when we are made a child
		virtual void SetParentChildIndex(const int32 in_parent_child_index) = 0;

		virtual const bool HasManualScrollX() const;
		virtual const bool HasManualScrollY() const;
		virtual void SetManualScrollX(const float in_x);
		virtual void SetManualScrollY(const float in_y);

		// in_shader_constant here feels like a bit of overkill, we dont srite to it, but saves collecting 10 or so bits of data to calculate it.. alternative is to have it higher in the UiComponent like UiComponentCanvas? UiComponentStack?
		// not planning on this being called for top level ui root component?~ can not set the top level render target clear colour, parent index should never be set, 
		// if we end up with a fixed size canvas, then that could allow the top level have desired size != avaliable size, also ui component stack? but that doesn't make sense, as we don't control the render viewport size of the top level render target...
		// this may make ui component stack unable to be a top level component?
		virtual void SetNode(
			DscDag::NodeToken in_parent_child_index,
			DscDag::NodeToken in_clear_colour_node,
			DscDag::NodeToken in_manual_scroll_x,
			DscDag::NodeToken in_manual_scroll_y//,
			//DscDag::NodeToken in_shader_constant // value is a DscUi::TUiPanelShaderConstantBuffer, ie, this is the data, not the buffer to hold the data in the draw system
			);

		// so, a UiComponentText may do word wrap and have a desired size shrunk in the vertical (or longer in vertical), parent still has GetChildGeometrySize to deal with shrink
		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size, const float in_ui_scale);

		// following only needed if this is a component type that can have children
		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const;
		// combine the desired size and the avalaible, in case we want to shring the geometry size if smaller than the avaliable, for stack children?
		// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const;
		// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const;


	};
}