#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_float4.h>
#include <dsc_text/i_text_run.h>

namespace DscRenderResource
{
	class GeometryGeneric;
}

namespace DscText
{
	class ITextRun;
	class GlyphCollectionIcon;
	class GlyphCollectionText;
	class TextLocale;

	class TextRun
	{
	public:
		static std::unique_ptr<ITextRun> MakeTextRunDataString(
			const std::string& in_string_utf8 = std::string(""),
			TextLocale* const in_locale_token = nullptr,
			GlyphCollectionText* const in_text_font = nullptr,
			const int in_font_size = 0,
			const float in_new_line_gap_ratio = 0.0f,
			const DscCommon::VectorFloat4& in_colour = DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
		);
		static std::unique_ptr<ITextRun> MakeTextRunDataIcon(
			const int in_icon_id = 0,
			GlyphCollectionIcon* const in_icon_font = nullptr,
			const DscCommon::VectorFloat4& in_colour_tint = DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f),
			const float in_new_line_gap_ratio = 0.0f
		);

		TextRun() = delete;
		TextRun& operator=(const TextRun&) = delete;
		TextRun(const TextRun&) = delete;

		// Do we keep the defaults, and each run has the settings? or simplify so we just have global settings at top, and data for each run
		TextRun(
			std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array,
			const DscCommon::VectorInt2& in_container_size,
			const bool in_width_limit_enabled = false,
			const int in_width_limit = 0,
			//const TextEnum::HorizontalLineAlignment in_horizontal_line_alignment = TextEnum::HorizontalLineAlignment::Left,
			//const TextEnum::VerticalBlockAlignment in_vertical_block_alignment = TextEnum::VerticalBlockAlignment::Top,
			const int in_em_size = 0,
			const float in_ui_scale = 1.0f
		);

		~TextRun();

		// Get the natural size required by the text using current width limit if enabled
		DscCommon::VectorInt2 GetTextBounds();

		void SetTextRunArray(
			std::vector<std::unique_ptr<ITextRun>>&& in_text_run_array
		);
		void SetTextContainerSize(
			const DscCommon::VectorInt2& in_size
		);
		void SetWidthLimit(
			const bool in_width_limit_enabled,
			const int in_width_limit
		);
		//const bool SetHorizontalLineAlignment(
		//	const TextEnum::HorizontalLineAlignment in_horizontal_line_alignment
		//);
		//const bool SetVerticalBlockAlignment(
		//	const TextEnum::VerticalBlockAlignment in_vertical_block_alignment
		//);
		void SetEMSize(
			const int in_em_size
		);
		void SetUIScale(
			const float in_ui_scale
		);

		// Rather than a seperate Update function, that could be forgoten, have the update in the getter, results in out_geometry_dirty

		//GeometryGeneric* const GetGeometryRef(
		//std::shared_ptr<GeometryGeneric>& GetGeometryRef(
		//	bool& out_geometry_dirty,
		//	DrawSystem* const in_draw_system,
		//	ID3D12GraphicsCommandList* const in_command_list
		//	);
		//void Draw(
		//	DrawSystem* const in_draw_system,
		//	DrawSystemFrame* const in_draw_system_frame,
		//	std::shared_ptr<Shader>& in_shader,
		//	TextTexture& in_text_texture
		//);

	private:
		std::vector<std::unique_ptr<ITextRun>> _text_run_array;
		DscCommon::VectorInt2 _container_size;
		bool _width_limit_enabled;
		int _width_limit;
		//TextEnum::HorizontalLineAlignment _horizontal_line_alignment;
		//TextEnum::VerticalBlockAlignment _vertical_block_alignment;
		int _em_size; // Vertical alignment can use a EM size 
		float _ui_scale;

		bool _calculate_dirty;
		std::unique_ptr<TextPreVertex> _pre_vertex_data;
		DscCommon::VectorInt2 _text_bounds;
		bool _geometry_dirty;
		std::unique_ptr<DscRenderResource::GeometryGeneric> _geometry;

	};
}
