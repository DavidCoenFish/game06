#include "text_pre_vertex.h"
#include "glyph.h"
#include "text_enum.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\math.h>

DscText::TextPreVertex::TextPreVertex()
{
	while ((int)_horizontal_bounds.size() <= _line_index)
	{
		_horizontal_bounds.push_back(DscCommon::VectorInt2(std::numeric_limits<int>::max(), -std::numeric_limits<int>::max()));
	}
}

DscText::TextPreVertex::~TextPreVertex()
{
	//nop
}

void DscText::TextPreVertex::Reserve(const unsigned int glyph_count)
{
	_pre_vertex_data.reserve(glyph_count);
}

void DscText::TextPreVertex::AddPreVertexScale(
	const Glyph& in_cell,
	const int32 in_pos_x,
	const int32 in_pos_y,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_colour,
	const float in_ui_scale
	)
{
	const DscCommon::VectorInt2 width_height = in_cell.GetWidthHeight(in_ui_scale);
	const DscCommon::VectorInt2 bearing = in_cell.GetBearing(in_ui_scale);
	AddPreVertex(
		width_height,
		bearing,
		in_cell.GetUVRef(),
		in_cell.GetMask(),
		in_pos_x,
		in_pos_y,
		DscCommon::Math::ScaleInt(in_line_minimum_height, in_ui_scale),
		DscCommon::Math::ScaleInt(in_line_minimum_depth, in_ui_scale),
		in_colour
	);
}

void DscText::TextPreVertex::AddPreVertex(
	const Glyph& in_cell,
	const int32 in_pos_x,
	const int32 in_pos_y,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_colour
)
{
	AddPreVertex(
		in_cell.GetWidthHeightRef(),
		in_cell.GetBearingRef(),
		in_cell.GetUVRef(),
		in_cell.GetMask(),
		in_pos_x,
		in_pos_y,
		in_line_minimum_height,
		in_line_minimum_depth,
		in_colour
	);
}

void DscText::TextPreVertex::AddPreVertex(
	const DscCommon::VectorInt2& in_width_height,
	const DscCommon::VectorInt2& in_bearing,
	const DscCommon::VectorFloat4& in_uv,
	const int32 in_mask,
	const int32 in_pos_x,
	const int32 in_pos_y,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_colour
)
{
	_line_dirty = true;
	_bound_dirty = true;
	_current_line_height = std::max(_current_line_height, in_line_minimum_height);
	_current_line_depth = std::max(_current_line_depth, in_line_minimum_depth);

	const int pos_x = in_pos_x + in_bearing.GetX();
	const int pos_y = in_pos_y - (in_width_height.GetY() - in_bearing.GetY());
	const DscCommon::VectorInt4 pos = DscCommon::VectorInt4(
		pos_x,
		pos_y,
		pos_x + in_width_height.GetX(),
		pos_y + in_width_height.GetY()
	);
	_current_line_height = std::max(_current_line_height, pos[3]);
	_current_line_depth = std::max(_current_line_depth, -(pos[1]));

	_pre_vertex_data.push_back(PreVertexData({
		pos,
		in_uv,
		in_mask,
		in_colour,
		_line_index
		}));

	_line_vertical_bounds[0] = std::min(_line_vertical_bounds[0], pos[1]);
	_line_vertical_bounds[1] = std::max(_line_vertical_bounds[1], pos[3]);

	_line_vertical_bounds[0] = std::min(_line_vertical_bounds[0], -_current_line_depth);
	_line_vertical_bounds[1] = std::max(_line_vertical_bounds[1], _current_line_height);

	while ((int)_horizontal_bounds.size() <= _line_index)
	{
		_horizontal_bounds.push_back(DscCommon::VectorInt2(std::numeric_limits<int>::max(), -std::numeric_limits<int>::max()));
	}

	_horizontal_bounds[_line_index][0] = std::min(_horizontal_bounds[_line_index][0], pos[0]);
	_horizontal_bounds[_line_index][1] = std::max(_horizontal_bounds[_line_index][1], pos[2]);

	return;
}

void DscText::TextPreVertex::UpdateHorizontalBounds(
	const int in_pos_x
)
{
	// This was added to get fixed width more stable. 
	// possibly some error with GetBounds not correctly using min _horizontal_bounds, thus adding a min(0)
	_horizontal_bounds[_line_index][0] = std::min(_horizontal_bounds[_line_index][0], 0);
	_horizontal_bounds[_line_index][1] = std::max(_horizontal_bounds[_line_index][1], in_pos_x);
}

void DscText::TextPreVertex::StartNewLine(
	int32& in_out_cursor,
	const int32 in_line_gap_pixels
	)
{
	in_out_cursor = 0;
	FinishLine(in_line_gap_pixels);

	_line_index += 1;
	_current_line_height = 0;
	_current_line_depth = 0;
	_line_vertical_bounds = DscCommon::VectorInt2(std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());

	while ((int)_horizontal_bounds.size() <= _line_index)
	{
		_horizontal_bounds.push_back(DscCommon::VectorInt2(std::numeric_limits<int>::max(), -std::numeric_limits<int>::max()));
	}

	return;
}

const DscCommon::VectorInt2 DscText::TextPreVertex::GetBounds()
{
	FinishLine(0);

	if (true == _bound_dirty)
	{
		_bound_dirty = false;
		DscCommon::VectorInt2 horizontal(
			std::numeric_limits<int>::max(),
			-std::numeric_limits<int>::max()
		);
		for (const auto& iter : _horizontal_bounds)
		{
			horizontal[0] = std::min(horizontal[0], iter[0]);
			horizontal[1] = std::max(horizontal[1], iter[1]);
		}
		_bounds[0] = horizontal[1] - horizontal[0];
		_bounds[1] = _vertical_bounds[1] - _vertical_bounds[0];
	}

	return _bounds;
}

void DscText::TextPreVertex::BuildVertexData(
	std::vector<uint8_t>& out_vertex_raw_data,
	const DscCommon::VectorInt2& in_container_size,
	const THorizontalAlignment in_horizontal_line_alignment,
	const TVerticalAlignment in_vertical_line_alignment
)
{
	FinishLine(0);

	const int line_count = static_cast<int>(_horizontal_bounds.size());
	std::vector<int> horizontal_line_delta(line_count);
	for (int index = 0; index < line_count; ++index)
	{
		const int width = _horizontal_bounds[index][1] - _horizontal_bounds[index][0];
		switch (in_horizontal_line_alignment)
		{
		default:
			break;
		case THorizontalAlignment::TMiddle:
			horizontal_line_delta[index] = ((in_container_size[0] - width) / 2) - _horizontal_bounds[index][0];
			break;
		case THorizontalAlignment::TRight:
			horizontal_line_delta[index] = (in_container_size[0] - _horizontal_bounds[index][1]);
			break;
		}
	}

	int vertical_delta = 0; // in_container_size.GetY();
	{
		const int32 bounds_height = _vertical_bounds[1] - _vertical_bounds[0];
		switch (in_vertical_line_alignment)
		{
		default:
			vertical_delta = in_container_size.GetY();
			break;
		case TVerticalAlignment::TMiddle:
			vertical_delta = ((in_container_size[1] - bounds_height) / 2) + bounds_height;
			break;
		case TVerticalAlignment::TBottom:
			vertical_delta = bounds_height;
			break;
		}
	}

	for (const auto& item : _pre_vertex_data)
	{
		const int horizontal_delta = horizontal_line_delta[item._line_index];
		const DscCommon::VectorFloat4 pos = DscCommon::VectorFloat4(
			((static_cast<float>(item._pos_low_high[0] + horizontal_delta) / static_cast<float>(in_container_size.GetX())) * 2.0f) - 1.0f,
			((static_cast<float>(item._pos_low_high[1] + vertical_delta) / static_cast<float>(in_container_size.GetY())) * 2.0f) - 1.0f,
			((static_cast<float>(item._pos_low_high[2] + horizontal_delta) / static_cast<float>(in_container_size.GetX())) * 2.0f) - 1.0f,
			((static_cast<float>(item._pos_low_high[3] + vertical_delta) / static_cast<float>(in_container_size.GetY())) * 2.0f) - 1.0f
		);

		//0.0f, 0.0f,
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[0]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[1]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[0]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[1]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._mask);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._colour);

		//0.0f, 1.0f,
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[0]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[3]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[0]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[3]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._mask);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._colour);

		//1.0f, 0.0f,
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[2]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[1]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[2]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[1]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._mask);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._colour);

		//1.0f, 0.0f,
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[2]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[1]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[2]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[1]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._mask);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._colour);

		//0.0f, 1.0f,
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[0]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[3]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[0]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[3]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._mask);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._colour);

		//1.0f, 1.0f,
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[2]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, pos[3]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[2]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._uv_low_high[3]);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._mask);
		DscCommon::DataHelper::AppendValue(out_vertex_raw_data, item._colour);
	}

	return;
}

void DscText::TextPreVertex::FinishLine(const int32 in_line_gap_pixels)
{
	if (false == _line_dirty)
	{
		return;
	}

	_accumulate_line_height_offset += _current_line_height;
	_accumulate_line_height_offset += _current_line_depth;
	_line_dirty = false;

	//move backward over _pre_vertex_data, items on _line_index
	for (std::vector<PreVertexData>::reverse_iterator i = _pre_vertex_data.rbegin();
		i != _pre_vertex_data.rend(); ++i)
	{
		PreVertexData& item = *i;
		if (item._line_index != _line_index)
		{
			break;
		}
		item._pos_low_high[1] -= _accumulate_line_height_offset;
		item._pos_low_high[3] -= _accumulate_line_height_offset;
	}

	_line_vertical_bounds[0] -= _accumulate_line_height_offset;
	_line_vertical_bounds[1] -= _accumulate_line_height_offset;

	_vertical_bounds[0] = std::min(_vertical_bounds[0], _line_vertical_bounds[0]);
	_vertical_bounds[1] = std::max(_vertical_bounds[1], _line_vertical_bounds[1]);

	_accumulate_line_height_offset += in_line_gap_pixels;

	return;

}

