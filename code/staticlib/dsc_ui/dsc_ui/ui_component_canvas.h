#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_float4.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class Shader;
	class ShaderConstantBuffer;
}

namespace DscUi
{
	class UiComponentCanvas : public IUiComponent
	{
	public:
		UiComponentCanvas(
			const int32 in_parent_child_index,
			const std::shared_ptr<DscRenderResource::Shader>& in_shader,
			const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
			const DscCommon::VectorFloat4& in_fill_colour
		);

		void AddChild(const VectorUiCoord2& in_child_size, const VectorUiCoord2& in_child_pivot, const VectorUiCoord2& in_attach_point, const int32 in_index);

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			const DscCommon::VectorInt2& in_target_size//,
			//const std::vector<DscRender::IRenderTarget*>& in_child_render_target_array
		) override;
		//virtual DscDag::NodeToken GetChildAvalableSizeNode(const int32 in_child_index) const override;

		virtual const DscCommon::VectorFloat4& GetClearColour() const override;

		virtual const int32 GetParentChildIndex() const override; // what child index are we of out parent

	private:
		int32 _parent_child_index = 0;
		DscCommon::VectorFloat4 _fill_colour;
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry = {};

		struct ChildSlot
		{
			VectorUiCoord2 _child_size; 
			VectorUiCoord2 _child_pivot;
			VectorUiCoord2 _attach_point;
			//DscDag::NodeToken _child_render; the IRenderTarget of the child node? of the node chain that will update the render
		};
		std::vector<ChildSlot> _child_slot_array = {};

		// feels a little mixed up to have the render target for the children in the parent, but trying to allow root node. so then is non root nodes, do they have their own render targets?
		//array of data for each input [render target, size, pivot, attach]
	};
}