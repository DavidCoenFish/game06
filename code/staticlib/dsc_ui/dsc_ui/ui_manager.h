#pragma once
#include "dsc_ui.h"

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
}

namespace DscDagRender
{
	class DagResource;
}

namespace DscUi
{
	class IUiComponent;
	class VectorUiCoord2;

	class UiManager
	{
	public:
		struct TSizeShaderConstantBuffer
		{
			float _value[4]; // _width_height
		};
		//struct TUiPanelShaderConstantBuffer
		//{
		//	float _value[4]; // _scroll_x_y;
		//};

		UiManager() = delete;
		UiManager& operator=(const UiManager&) = delete;
		UiManager(const UiManager&) = delete;

		//DscDag::DagCollection
		UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection);
		~UiManager();

		std::unique_ptr<IUiComponent> MakeComponentDebugFill(DscRender::DrawSystem& in_draw_system, const int32 in_parent_child_index = 0);
		std::unique_ptr<IUiComponent> MakeComponentFill(DscRender::DrawSystem& in_draw_system, const DscCommon::VectorFloat4& in_background_colour, const int32 in_parent_child_index = 0);
		std::unique_ptr<IUiComponent> MakeComponentCanvas(DscRender::DrawSystem& in_draw_system, const DscCommon::VectorFloat4& in_background_colour, const int32 in_parent_child_index = 0);

		struct ResultNodeData
		{
			DscDag::NodeToken _ui_node = {};
			DscDag::NodeToken _ui_component_node = {};
			DscDag::NodeToken _desired_size_node = {};
		};

		ResultNodeData MakeUiRootNode(
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component
			);

		ResultNodeData MakeUiNode(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,

			DscDag::NodeToken in_parent_ui_component_node,
			DscDag::NodeToken in_parent_desired_size_node, // parent desired size not in directly in the parent UiComponent, and not of a know offset in the node [root?node?]
			DscDag::NodeToken in_ui_root_node
			);

		ResultNodeData MakeUiNodeCanvasChild(
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>&& in_component,

			DscDag::NodeToken in_parent_ui_component_node, // assert if not a UiComponentCanvas
			DscDag::NodeToken in_parent_desired_size_node, // parent desired size not in directly in the parent UiComponent, and not of a know offset in the node [root?node?]
			DscDag::NodeToken in_ui_root_node,

			const VectorUiCoord2& in_child_size, 
			const VectorUiCoord2& in_child_pivot, 
			const VectorUiCoord2& in_attach_point
		);


		//static IUiComponent& GetComponentFromUiRootNode(DscDag::NodeToken in_ui_root_node);
		//static IUiComponent& GetComponentFromUiNode(DscDag::NodeToken in_ui_node);

		void DrawUiSystem(
			DscRender::IRenderTarget* const in_render_target,
			const bool in_always_draw, // if this render target is shared, need to at least redraw the top level ui
			const bool in_clear_on_draw, // clear the top level render target before we draw to it
			DscDag::NodeToken in_ui_root_node,
			DscRenderResource::Frame& in_frame
		);

		//could be different per ui system? probably not however....
		const float GetUiScale() const;
		void SetUiScale(const float in_ui_scale);

	private:
		//std::shared_ptr<DscRenderResource::ShaderConstantBuffer> MakeSizeShaderConstantBuffer(DscRender::DrawSystem& in_draw_system);
		//std::shared_ptr<DscRenderResource::ShaderConstantBuffer> MakeUiPanelShaderConstantBuffer(DscRender::DrawSystem& in_draw_system);

	private:
		std::unique_ptr < DscDagRender::DagResource> _dag_resource = {};
		
		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		//std::shared_ptr<DscRenderResource::Shader> _screen_quad_texture_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};

		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};
		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};

		DscDag::NodeToken _dag_node_ui_scale = {};
		DscDag::NodeToken _dag_node_frame = {}; // a place holder node to hold the render frame
	};
}