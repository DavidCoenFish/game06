#include "screen_quad.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\math.h>
#include <dsc_render\draw_system.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>

namespace
{
	static const std::vector<D3D12_INPUT_ELEMENT_DESC> s_input_element_desc_array({
		D3D12_INPUT_ELEMENT_DESC
		{
			"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		},
		D3D12_INPUT_ELEMENT_DESC
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
		}
	});

} // namespace

const std::vector<D3D12_INPUT_ELEMENT_DESC>& DscUi::ScreenQuad::GetInputElementDesc()
{
	return s_input_element_desc_array;
}

DscUi::ScreenQuad::ScreenQuad(
	const VectorUiCoord2& in_quad_size,
	const VectorUiCoord2& in_quad_pivot,
	const VectorUiCoord2& in_parent_attach,
	const DscCommon::VectorInt2& in_parent_size
)
	: _quad_size(in_quad_size)
	, _quad_pivot(in_quad_pivot)
	, _parent_attach(in_parent_attach)
	, _parent_size(in_parent_size)
	, _geometry_generic(nullptr)
	, _dirty(true)
{
	//nop
}

void DscUi::ScreenQuad::SetParentSize(
	const DscCommon::VectorInt2& in_parent_size
)
{
	if (_parent_size != in_parent_size)
	{
		_parent_size = in_parent_size;
		_dirty = true;
	}
}

void DscUi::ScreenQuad::Draw(
	DscRender::DrawSystem& in_draw_system,
	DscRenderResource::Frame& in_frame
)
{
	if (true == _dirty)
	{
		_dirty = false;
		std::vector<uint8_t> vertex_raw_data;
		BuildVertexData(vertex_raw_data);

		if (nullptr == _geometry_generic)
		{
			_geometry_generic = std::make_shared<DscRenderResource::GeometryGeneric>(
				&in_draw_system,
				D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
				s_input_element_desc_array,
				vertex_raw_data,
				4
				);
		}
		else
		{
			_geometry_generic->UpdateVertexData(
				&in_draw_system,
				in_frame.GetCommandList(),
				in_draw_system.GetD3dDevice(),
				vertex_raw_data
			);
		}
	}

	in_frame.Draw(_geometry_generic);
}

void DscUi::ScreenQuad::BuildVertexData(std::vector<uint8_t>& out_vertex_raw_data)
{
	const DscCommon::VectorInt2 size = _quad_size.EvalueUICoord(_parent_size);
	const DscCommon::VectorInt2 pivot = _quad_pivot.EvalueUICoord(size);
	const DscCommon::VectorInt2 attach = _parent_attach.EvalueUICoord(_parent_size);

	const DscCommon::VectorFloat4 pos(
		DscCommon::Math::UIPixelsToRenderSpaceHorizontal(attach.GetX() - pivot.GetX(), _parent_size.GetX()),
		DscCommon::Math::UIPixelsToRenderSpaceVertical(attach.GetY() - pivot.GetY(), _parent_size.GetY()),
		DscCommon::Math::UIPixelsToRenderSpaceHorizontal(attach.GetX() - pivot.GetX() + size.GetX(), _parent_size.GetX()),
		DscCommon::Math::UIPixelsToRenderSpaceVertical(attach.GetY() - pivot.GetY() + size.GetY(), _parent_size.GetY())
		);

	//0.0f, 0.0f,
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[0]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[1]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 0.0f);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 0.0f);

	//1.0f, 0.0f,
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[2]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[1]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 1.0f);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 0.0f);

	//0.0f, 1.0f,
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[0]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[3]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 0.0f);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 1.0f);

	//1.0f, 1.0f,
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[2]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[3]);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 1.0f);
	DscCommon::DataHelper::AppendValue(out_vertex_raw_data, 1.0f);

	return;
}
