#include "text_run.h"

#include "text_run_icon.h"
#include "text_run_text.h"
#include "text_pre_vertex.h"
#include "text_enum.h"
#include "text_manager.h"
#include <dsc_render\draw_system.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>


std::unique_ptr<DscText::ITextRun> DscText::TextRun::MakeTextRunDataString(
	const std::string& in_string_utf8,
	const TextLocale* const in_locale_token,
	GlyphCollectionText* const in_text_font,
	const int32 in_font_size,
	const int32 in_colour,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_base_line_offset
)
{
	return std::make_unique<DscText::TextRunText>(
		in_string_utf8,
		in_text_font,
		in_locale_token,
		in_font_size,
		in_colour,
		in_line_minimum_height,
		in_line_minimum_depth,
		in_base_line_offset
		);
}

std::unique_ptr<DscText::ITextRun> DscText::TextRun::MakeTextRunDataIcon(
	const int32 in_icon_id,
	GlyphCollectionIcon* const in_icon_font,
	const int32 in_colour_tint,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_base_line_offset
)
{
	return std::make_unique<DscText::TextRunIcon>(
		in_icon_id,
		in_icon_font,
		in_colour_tint,
		in_line_minimum_height,
		in_line_minimum_depth,
		in_base_line_offset
		);
}

// Do we keep the defaults, and each run has the settings? or simplify so we just have global settings at top, and data for each run
DscText::TextRun::TextRun(
	std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array,
	const DscCommon::VectorInt2& in_container_size,
	const bool in_width_limit_enabled,
	const int in_width_limit,
	const THorizontalAlignment in_horizontal_line_alignment,
	const TVerticalAlignment in_vertical_block_alignment,
	const int32 in_line_gap_pixels,
	const float in_ui_scale
)
: _text_run_array(std::move(in_text_run_array))
, _container_size(in_container_size)
, _width_limit_enabled(in_width_limit_enabled)
, _width_limit(in_width_limit)
, _horizontal_line_alignment(in_horizontal_line_alignment)
, _vertical_block_alignment(in_vertical_block_alignment)
, _line_gap_pixels(in_line_gap_pixels)
, _ui_scale(in_ui_scale)
, _calculate_dirty(true)
, _pre_vertex_data()
, _text_bounds()
, _geometry_dirty(true)
, _geometry()
{
	//nop
}

DscText::TextRun::~TextRun()
{
	//nop
}

const std::shared_ptr<DscRenderResource::GeometryGeneric>& DscText::TextRun::GetGeometry(
	DscRender::DrawSystem* const in_draw_system,
	DscRenderResource::Frame* const in_draw_system_frame
	)
{
	GetTextBounds();

	if (true == _geometry_dirty)
	{
		_geometry_dirty = false;

		std::vector<uint8_t> vertex_raw_data;
		_pre_vertex_data->BuildVertexData(
			vertex_raw_data,
			_container_size,
			_horizontal_line_alignment,
			_vertical_block_alignment
		);

		// the problem with resizing an existing geometry, is what if that geometry is still on a command list
		// we can modify the data, as that just pokes different data onto the command list, but resize can end up being destructive, so better is to destroy and recreate the geometry if size changes
		// again, found by fps text doing something like "0.0" => "59.9"
		if ((nullptr != _geometry) && (vertex_raw_data.size() != _geometry->GetVertexDataByteSize()))
		{
			// note, the DrawSystem may still be holding a reference to the shared pointer if the geometry is still on an in use command list
			_geometry.reset();
		}

		if (nullptr == _geometry)
		{
			_geometry = std::make_shared<DscRenderResource::GeometryGeneric>(
				in_draw_system,
				D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
				TextManager::GetInputElementDesc(),
				vertex_raw_data,
				6
			);
		}
		else
		{
			_geometry->UpdateVertexData(
				in_draw_system,
				in_draw_system_frame->GetCommandList(),
				in_draw_system->GetD3dDevice(),
				vertex_raw_data
				);
		}
	}
	return _geometry;
}

// Get the natural size required by the text using current width limit if enabled
DscCommon::VectorInt2 DscText::TextRun::GetTextBounds()
{
	if (true == _calculate_dirty)
	{
		_calculate_dirty = false;

		_pre_vertex_data = std::make_unique<TextPreVertex>();
		int32 cursor = 0;
		for (const auto& item : _text_run_array)
		{
			item->BuildPreVertexData(
				*_pre_vertex_data,
				cursor,
				_width_limit_enabled,
				_width_limit,
				_line_gap_pixels,
				_ui_scale
			);
		}
		_text_bounds = _pre_vertex_data->GetBounds();
	}

	return _text_bounds;
}

void DscText::TextRun::SetTextRunArray(
	std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array
)
{
	_text_run_array = std::move(in_text_run_array);
	_calculate_dirty = true;
	_geometry_dirty = true;
}

void DscText::TextRun::SetTextContainerSize(
	const DscCommon::VectorInt2& in_container_size
)
{
	if (_container_size != in_container_size)
	{
		_container_size = in_container_size;
		_geometry_dirty = true;
	}
}

const bool DscText::TextRun::GetWidthLimitEnabled() const
{
	return _width_limit_enabled;
}

void DscText::TextRun::SetWidthLimit(
	const bool in_width_limit_enabled,
	const int in_width_limit
)
{
	if (_width_limit_enabled != in_width_limit_enabled)
	{
		_width_limit_enabled = in_width_limit_enabled;
		_calculate_dirty = true;
		_geometry_dirty = true;
	}
	if (_width_limit != in_width_limit)
	{
		_width_limit = in_width_limit;
		if (true == _width_limit_enabled)
		{
			_calculate_dirty = true;
			_geometry_dirty = true;
		}
	}
}

void DscText::TextRun::SetHorizontalLineAlignment(
	const THorizontalAlignment in_horizontal_line_alignment
)
{
	if (_horizontal_line_alignment != in_horizontal_line_alignment)
	{
		_horizontal_line_alignment = in_horizontal_line_alignment;
		_geometry_dirty = true;
	}
}

void DscText::TextRun::SetVerticalBlockAlignment(
	const TVerticalAlignment in_vertical_block_alignment
)
{
	if (_vertical_block_alignment != in_vertical_block_alignment)
	{
		_vertical_block_alignment = in_vertical_block_alignment;
		_geometry_dirty = true;
	}
}

void DscText::TextRun::SetLineGapPixels(
	const int in_line_gap_pixels
)
{
	if (_line_gap_pixels != in_line_gap_pixels)
	{
		_line_gap_pixels = in_line_gap_pixels;
		_calculate_dirty = true;
		_geometry_dirty = true;
	}
}

void DscText::TextRun::SetUIScale(
	const float in_ui_scale
)
{
	if (_ui_scale != in_ui_scale)
	{
		_ui_scale = in_ui_scale;
		_calculate_dirty = true;
		_geometry_dirty = true;
	}
}
