#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_enum.h"
#include <dsc_dag\dag_group.h>

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi
{
	class UiComponentButton : public IUiComponent
	{
	public:
		UiComponentButton() = delete;
		UiComponentButton& operator=(const UiComponentButton&) = delete;
		UiComponentButton(const UiComponentButton&) = delete;

		UiComponentButton(
			const TUiComponentBehaviour in_behaviour
			);

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
		{
			return true;
		}

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
		) override;

	private:
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};
		TUiComponentBehaviour _behaviour = {};

		//current state
		enum class TState : uint8
		{
			TNone = 0,
			TRollover,
			TClickDown,

		};

		struct StateSlot
		{
			// state mask
			DscDag::NodeToken _render_node = {};
			DscDag::NodeToken _ui_panel_shader_constant_node = {};
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
			DscDag::NodeToken _desired_size = {};
		};
		std::vector<StateSlot> _state_slot_array = {};

		// click callback
		// rollover duration

	};
}