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

		std::unique_ptr<UiRenderTarget> MakeUiRenderTarget(
			DscRender::IRenderTarget* const in_render_target,
			const bool in_allow_clear_on_draw
			);
		std::unique_ptr<UiRenderTarget> MakeUiRenderTarget(
			const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
			const bool in_allow_clear_on_draw
			);

		UiRootNodeGroup MakeRootNode(
			const TUiComponentType in_type,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<UiRenderTarget>&& in_ui_texture
			);
#if 0
		UiNodeGroup ConvertRootNodeGroupToNodeGroup(
			const UiRootNodeGroup& in_ui_root_node_group
			);
		UiNodeGroup AddChildNode(
			const TUiComponentType in_type,
			DscRender::DrawSystem& in_draw_system,
			const UiRootNodeGroup& in_root_node_group,
			const UiNodeGroup& in_parent
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
		DscDag::NodeToken MakeDrawNode(
			const TUiComponentType in_type,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			DscDag::NodeToken in_frame_node,
			DscDag::NodeToken in_ui_render_target_node,
			DscDag::NodeToken in_render_target_viewport_size_node
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
