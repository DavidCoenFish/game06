#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"
#include "ui_coord.h"
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
	class RenderTargetTexture;
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
	class UiInputState;
	class UiRenderTarget;
	class VectorUiCoord2;

	class UiManager
	{
	public:
		UiManager() = delete;
		UiManager& operator=(const UiManager&) = delete;
		UiManager(const UiManager&) = delete;

		UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection);
		~UiManager();

		// these are std::shared_ptr as do not have a DagNodeCalculate unique, and we are 
		std::shared_ptr<UiRenderTarget> MakeUiRenderTarget(
			DscRender::IRenderTarget* const in_render_target,
			const bool in_allow_clear_on_draw
			);
		std::shared_ptr<UiRenderTarget> MakeUiRenderTarget(
			const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
			const bool in_allow_clear_on_draw
			);

		struct TComponentConstructionHelper
		{
			TUiComponentType _component_type;
		};
		static TComponentConstructionHelper MakeComponentGridFill();

		struct TEffectConstructionHelper
		{
			TUiEffectType _effect_type;
			DscCommon::VectorFloat4 _effect_param;
			DscCommon::VectorFloat4 _effect_param_tint;
		};
		UiRootNodeGroup MakeRootNode(
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::shared_ptr<UiRenderTarget>& in_ui_render_target,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>()
			);
#if 0
		UiNodeGroup ConvertRootNodeGroupToNodeGroup(
			const UiRootNodeGroup& in_ui_root_node_group
			);
		UiNodeGroup AddChildNode(
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const UiRootNodeGroup& in_root_node_group,
			const UiNodeGroup& in_parent,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>()
		);
#endif
		// no seperating update from draw as worried about not having the correct render size/ layout to consume input

		void Draw(
			const UiRootNodeGroup& in_root_node_group,
			DscDag::DagCollection& in_dag_collection,
			DscRenderResource::Frame& in_frame,
			const bool in_force_draw,
			const float in_time_delta,
			const UiInputState& in_input_state,
			DscRender::IRenderTarget* const in_external_render_target_or_null = nullptr
			);

	private:
		DscDag::NodeToken MakeDrawStack(
			const TUiDrawType in_type,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::vector<TEffectConstructionHelper>& in_effect_array,
			DscDag::NodeToken in_frame_node,
			// we don't dirty on ui render target being set, so have a render target viewport size which dirties on size change
			DscDag::NodeToken in_render_target_viewport_size_node,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_last_render_target_or_null,
			DscDag::NodeToken in_clear_colour_or_null
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		DscDag::NodeToken MakeDrawNode(
			const TUiDrawType in_type,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::vector<DscDag::NodeToken>& in_array_input_stack,
			DscDag::NodeToken in_frame_node,
			DscDag::NodeToken in_ui_render_target_node,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_effect_param_or_null,
			DscDag::NodeToken in_effect_tint_or_null
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

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
