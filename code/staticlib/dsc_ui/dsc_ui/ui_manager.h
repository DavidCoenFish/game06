#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"
#include <dsc_common\vector_float4.h>

namespace DscCommon
{
	class FileSystem;
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetPool;
	class Shader;
	class ShaderConstantBuffer;
	class ShaderResource;
}

namespace DscDagRender
{
	class DagResource;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi
{
	class IUiComponent;
	class UiCoord;
	class VectorUiCoord2;

	class UiManager
	{
	public:
		UiManager() = delete;
		UiManager& operator=(const UiManager&) = delete;
		UiManager(const UiManager&) = delete;

		//DscDag::DagCollection
		UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection);
		~UiManager();

		std::unique_ptr<IUiComponent> MakeComponentDebugGrid(DscRender::DrawSystem& in_draw_system);
		std::unique_ptr<IUiComponent> MakeComponentFill();
		std::unique_ptr<IUiComponent> MakeComponentImage(
			const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture
			);
		std::unique_ptr<IUiComponent> MakeComponentCanvas();
		std::unique_ptr<IUiComponent> MakeComponentText(
			DscText::TextManager& in_text_manager,
			std::unique_ptr<DscText::TextRun>&& in_text_run,
			const TUiComponentBehaviour in_behaviour
			);
		std::unique_ptr<IUiComponent> MakeComponentStack(
			const UiCoord& in_item_gap,
			const TUiFlow in_ui_flow
		);
		// margin cuts in from the parrent size
		std::unique_ptr<IUiComponent> MakeComponentMargin(
			const UiCoord& in_left,
			const UiCoord& in_top,
			const UiCoord& in_right,
			const UiCoord& in_bottom
		);
		// padding inflates the desired size of something like text
		std::unique_ptr<IUiComponent> MakeComponentPadding(
			const UiCoord& in_left,
			const UiCoord& in_top,
			const UiCoord& in_right,
			const UiCoord& in_bottom
		);

		DagGroupUiRootNode MakeUiRootNode(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const std::vector<TEffectData>& in_array_effect_data = std::vector<TEffectData>()
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		static DagGroupUiParentNode ConvertUiRootNodeToParentNode(const DagGroupUiRootNode& in_ui_root_node_group);

		// calls through to MakeUiNode, and on adding a child to a parent is when the ui component gets its ParentChildIndex set?
		DagGroupUiParentNode MakeUiNodeCanvasChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node,

			const VectorUiCoord2& in_child_size, 
			const VectorUiCoord2& in_child_pivot, 
			const VectorUiCoord2& in_attach_point,

			const std::vector<TEffectData>& in_array_effect_data = std::vector<TEffectData>()

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		// calls through to MakeUiNode, and on adding a child to a parent is when the ui component gets its ParentChildIndex set?
		DagGroupUiParentNode MakeUiNodeStackChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node,

			const std::vector<TEffectData>& in_array_effect_data = std::vector<TEffectData>()


			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		// calls through to MakeUiNode, and on adding a child to a parent is when the ui component gets its ParentChildIndex set?
		DagGroupUiParentNode MakeUiNodeMarginChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node,

			const std::vector<TEffectData>& in_array_effect_data = std::vector<TEffectData>()

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		DagGroupUiParentNode MakeUiNodePaddingChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node,

			const std::vector<TEffectData>& in_array_effect_data = std::vector<TEffectData>()

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		// could be combined with the DrawUiSystem, but doesn't need to be
		void UpdateUiSystem(
			DagGroupUiRootNode& in_ui_root_node_group, // not const as setting values on it
			const float in_time_delta
			// input state? keys down, touch pos, gamepad
			);

		void DrawUiSystem(
			DagGroupUiRootNode& in_ui_root_node_group, // not const as resolve conditional nodes may change data...
			DscRender::IRenderTarget* const in_render_target,
			DscRenderResource::Frame& in_frame,
			const bool in_force_top_level_draw, // if this render target is shared, need to at least redraw the top level ui
			const bool in_clear_on_draw, // clear the top level render target before we draw to it
			const float in_ui_scale = 1.0f
		);

	private:
		// if this has a reason to be public, then lets make it puplic
		// on adding a child to a parent is when it's clear colour is set...
		DagGroupUiParentNode MakeUiNode(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,
			const DscCommon::VectorFloat4& in_clear_colour,

			const DagGroupUiRootNode& in_root_node,
			const DagGroupUiParentNode& in_parent_node,

			const std::vector<TEffectData>& in_array_effect_data

			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
		);

		std::shared_ptr<DscRenderResource::Shader> GetEffectShader(const TEffect in_effect);
		std::shared_ptr<DscRenderResource::GeometryGeneric> GetEffectGeometry(const TEffect in_effect);

	private:
		/// dag resource hooks into the render system "callbacks" as to know when the device is restored
		std::unique_ptr < DscDagRender::DagResource> _dag_resource = {};
		
		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _image_shader = {};

		std::shared_ptr<DscRenderResource::Shader> _effect_round_corner_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_drop_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_stroke_shader = {};

		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};

	};
}