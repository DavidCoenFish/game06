#pragma once
#include "dsc_ui.h"

namespace DscCommon
{
	class FileSystem;
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
	class Frame;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class GeometryGeneric;
	class RenderTargetPool;
	class Shader;
	class ShaderConstantBuffer;
}

namespace DscUi
{
	class IUiComponent;

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

		std::unique_ptr<IUiComponent> MakeComponentDebugFill(DscRender::DrawSystem& in_draw_system);

		DscDag::NodeToken MakeUiRootNode(
			DscDag::DagCollection& in_dag_collection,
			std::unique_ptr<IUiComponent>& in_component
			);
			//DscDag::NodeToken MakeUiNode(std::unique_ptr<IUiComponent>, DscDag::NodeToken in_parent, DscDag::NodeToken in_root_node)
		void DrawUiSystem(
			DscRender::IRenderTarget* const in_render_target,
			const bool in_always_draw, // if this render target is shared, need to at least redraw the top level ui
			DscDag::NodeToken in_ui_root_node,
			DscRender::Frame& in_frame
		);
			//UiRootNode, commandList or Frame, time delta, current touch, array button presses)

		//could be different per ui system? probably not however....
		const float GetUiScale() const;
		void SetUiScale(const float in_ui_scale);

	private:
		//std::shared_ptr<DscRenderResource::ShaderConstantBuffer> MakeSizeShaderConstantBuffer(DscRender::DrawSystem& in_draw_system);
		//std::shared_ptr<DscRenderResource::ShaderConstantBuffer> MakeUiPanelShaderConstantBuffer(DscRender::DrawSystem& in_draw_system);

	private:
		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};
		//std::shared_ptr<DscRenderResource::Shader> _screen_quad_texture_shader = {};
		//std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};

		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};
		std::unique_ptr<DscRenderResource::RenderTargetPool> _render_target_pool = {};

		DscDag::NodeToken _dag_node_ui_scale = {};
	};
}