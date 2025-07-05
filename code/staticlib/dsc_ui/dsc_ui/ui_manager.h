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
			DscCommon::VectorFloat4 _clear_colour;
			DscCommon::VectorFloat4 _fill;
			std::shared_ptr<DscRenderResource::ShaderResource> _texture;
		};
		static TComponentConstructionHelper MakeComponentDebugGrid();
		static TComponentConstructionHelper MakeComponentFill(const DscCommon::VectorFloat4& in_colour);
		static TComponentConstructionHelper MakeComponentImage(const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture);

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
		UiNodeGroup ConvertRootNodeGroupToNodeGroup(
			DscDag::DagCollection& in_dag_collection,
			const UiRootNodeGroup& in_ui_root_node_group
			);

		UiNodeGroup AddChildNode(
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const UiRootNodeGroup& in_root_node_group,
			const UiNodeGroup& in_parent,
			const std::vector<TEffectConstructionHelper>& in_effect_array = std::vector<TEffectConstructionHelper>()
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = "")
		);

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
			const TComponentConstructionHelper& in_construction_helper,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::vector<TEffectConstructionHelper>& in_effect_array,
			DscDag::NodeToken in_frame_node,
			// we don't dirty on ui render target being set, so have a render target viewport size which dirties on size change
			DscDag::NodeToken in_render_target_viewport_size_node,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_last_render_target_or_null,
			UiComponentResourceNodeGroup& in_component_resource_group
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		DscDag::NodeToken MakeDrawNode(
			const TUiDrawType in_type,
			const TComponentConstructionHelper* const in_construction_helper_or_null,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::vector<DscDag::NodeToken>& in_array_input_stack,
			DscDag::NodeToken in_frame_node,
			DscDag::NodeToken in_ui_render_target_node,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_effect_param_or_null,
			DscDag::NodeToken in_effect_tint_or_null,
			UiComponentResourceNodeGroup& in_component_resource_group
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

	private:
		/// dag resource hooks into the render system "callbacks" as to know when the device is restored
		std::unique_ptr < DscDagRender::DagResource> _dag_resource = {};

		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _image_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _fill_shader = {};

		std::shared_ptr<DscRenderResource::Shader> _effect_round_corner_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_drop_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_inner_shadow_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_stroke_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _effect_tint_shader = {};

		// a full quad is of pos range [-1 ... 1] and uv range of [0 ... 1]
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_quad_pos_uv = {};
		// a full quad is of pos range [-1 ... 1]
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_quad_pos = {};
		// panel geometry is vertex of pos range [0 ... 1] intended to be affected by shader variables
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};
	};
}
