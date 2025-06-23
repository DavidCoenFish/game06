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

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target//,
			//const float in_ui_scale // no, ui scale needs to be earlier, in size calculation pass, ie, change text size
			);

		virtual const DscCommon::VectorFloat4& GetClearColour() const;
		// return true if value changes
		virtual const bool SetClearColour(const DscCommon::VectorFloat4& in_colour);

		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const;
		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size);
		// combine the desired size and the avalaible, in case we want to shring the geometry size if smaller than the avaliable, for stack children?
		// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const;
		// may need to fetch the desired size of all the children first, ie, for stack
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const;

		virtual const int32 GetParentChildIndex() const = 0; // what child index are we of out parent

		// if this can be called after the top level render target viewport size is set, then also update the ScrollTraveralPixelDistance?
		virtual void Update(const float in_time_delta);
		
		//virtual void AddChildRef(IUiComponent* in_ui_component);
		//virtual void RemoveChild(IUiComponent* in_ui_component);

		virtual void SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node);

#if 0
		// we either need the data out of the child, or need to tell the child to draw with our panel shader?
		//virtual void ParentDraw(
		//	DscRenderResource::Frame& in_frame,
		//	DscRender::IRenderTarget& in_render_target,
		//	ui panel shader
		//	);
#else
		virtual void BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer);
		virtual std::shared_ptr<DscRender::HeapWrapperItem> GetRenderTexture();
#endif
	};
}