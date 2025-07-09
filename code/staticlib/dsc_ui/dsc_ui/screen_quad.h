#pragma once
#include "dsc_ui.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_int2.h>

namespace DscRender
{
	class DrawSystem;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
}

namespace DscUi
{

	///just generate some geometry to put something on screen, less features than a UiPanel
	///vertex format [2 float pos, 2 float uv]
	class ScreenQuad
	{
	public:
		// pos, uv
		static const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputElementDesc();

		explicit ScreenQuad(
			const VectorUiCoord2& in_quad_size,
			const VectorUiCoord2& in_quad_pivot,
			const VectorUiCoord2& in_parent_attach,
			const DscCommon::VectorInt2& in_parent_size
		);

		void SetParentSize(
			const DscCommon::VectorInt2& in_parent_size
			);

		const DscCommon::VectorInt2 GetCalculatedSize() const;

		/// <summary>
		/// updates the geometry if inputs are dirty
		/// </summary>
		/// <param name="in_draw_system"></param>
		/// <param name="in_command_list"></param>
		/// <returns></returns>
		std::shared_ptr<DscRenderResource::GeometryGeneric> GetGeometry(
			DscRender::DrawSystem& in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list
		);

	private:
		void BuildVertexData(std::vector<uint8_t>& in_vertex_raw_data);

	private:
		VectorUiCoord2 _quad_size;// = {};
		VectorUiCoord2 _quad_pivot;// = {};
		VectorUiCoord2 _parent_attach;// = {};
		DscCommon::VectorInt2 _parent_size;// = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry_generic = {};
		bool _dirty = true;

	};
}