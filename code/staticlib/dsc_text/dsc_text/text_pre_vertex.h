#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_int4.h>
#include <dsc_common/vector_float4.h>
#include <dsc_text/text_enum.h>

namespace DscText
{
	class Glyph;
	enum class THorizontalAlignment;

	/*
	collect data about a text run before converting it to vertex data for the gpu
	*/
	class TextPreVertex
	{
	public:
		struct PreVertexData
		{
			DscCommon::VectorInt4 _pos_low_high;
			DscCommon::VectorFloat4 _uv_low_high;
			int32 _mask;//DscCommon::VectorFloat4 _mask;
			int32 _colour; // DscCommon::VectorFloat4 _colour;
			int _line_index;
		};

		TextPreVertex();
		~TextPreVertex();

		void Reserve(const unsigned int glyph_count);

		void AddPreVertexScale(
			const Glyph& in_cell,
			const int32 in_pos_x,
			const int32 in_pos_y,
			const int32 in_line_minimum_height,
			const int32 in_colour, // DscCommon::VectorFloat4& in_colour,
			const float in_ui_scale
		);

		void AddPreVertex(
			const Glyph& in_cell,
			const int32 in_pos_x,
			const int32 in_pos_y,
			const int32 in_line_minimum_height,
			const int32 in_colour // in_colour
		);

		void AddPreVertex(
			const DscCommon::VectorInt2& in_width_height,
			const DscCommon::VectorInt2& in_bearing,
			const DscCommon::VectorFloat4& in_uv,
			const int32 in_mask,
			const int32 in_pos_x,
			const int32 in_pos_y,
			const int32 in_line_minimum_height,
			const int32 in_colour // const DscCommon::VectorFloat4& in_colour
		);

		void UpdateHorizontalBounds(
			const int in_pos_x
		);

		void StartNewLine(
			int32& in_out_cursor,
			const int32 in_line_gap_pixels
		);

		const DscCommon::VectorInt2 GetBounds();

		void BuildVertexData(
			std::vector<uint8_t>& out_vertex_raw_data,
			const DscCommon::VectorInt2& in_container_size,
			const THorizontalAlignment in_horizontal_line_alignment,
			const TVerticalAlignment in_vertical_line_alignment
		);

	private:
		void FinishLine(const int32 in_line_gap_pixels);

	private:
		std::vector<PreVertexData> _pre_vertex_data;
		DscCommon::VectorInt2 _vertical_bounds;
		std::vector<DscCommon::VectorInt2> _horizontal_bounds;

		bool _bound_dirty;
		DscCommon::VectorInt2 _bounds;
		int _line_index;

		bool _line_dirty; // Pre vertex added to line, but not adjusted for max height on line
		DscCommon::VectorInt2 _line_vertical_bounds;
		int _current_line_height; // Allow for things added to the line to be taller
		int32 _accumulate_line_height_offset = 0;

	};
}
