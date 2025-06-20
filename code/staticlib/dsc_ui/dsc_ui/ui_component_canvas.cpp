#include "ui_component_canvas.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentCanvas::UiComponentCanvas(
	const int32 in_parent_child_index,
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
	const DscCommon::VectorFloat4& in_fill_colour
)
	: _parent_child_index(in_parent_child_index)
	, _fill_colour(in_fill_colour)
	, _shader(in_shader)
	, _shader_constant_buffer(in_shader_constant_buffer)
{
	// nop
}

void DscUi::UiComponentCanvas::AddChild(const VectorUiCoord2& in_child_size, const VectorUiCoord2& in_child_pivot, const VectorUiCoord2& in_attach_point, const int32 in_index)
{
	if (_child_slot_array.size() <= static_cast<size_t>(in_index))
	{
		_child_slot_array.resize(in_index + 1);
	}
	_child_slot_array[in_index] = ChildSlot({ in_child_size, in_child_pivot, in_attach_point });
}

void DscUi::UiComponentCanvas::Draw(
	DscRenderResource::Frame& ,//in_frame,
	const DscCommon::VectorInt2& //in_target_size//,
	//const std::vector<DscRender::IRenderTarget*>& in_child_render_target_array
)
{

}

const DscCommon::VectorFloat4& DscUi::UiComponentCanvas::GetClearColour() const
{
	return _fill_colour;
}

const int32 DscUi::UiComponentCanvas::GetParentChildIndex() const
{
	return _parent_child_index;
}
