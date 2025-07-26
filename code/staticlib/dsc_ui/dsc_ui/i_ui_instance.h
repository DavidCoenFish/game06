#pragma once
#include "dsc_ui.h"

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscUi
{
	class IUiInstance
	{
	public:
		virtual ~IUiInstance() {}

		virtual void Update() = 0;

		virtual DscDag::NodeToken GetDagUiGroupNode() = 0;
		virtual DscDag::NodeToken GetDagUiDrawNode() = 0;
		virtual DscDag::NodeToken GetDagUiDrawBaseNode() = 0;

		//virtual void UiRootUpdate(
		//	UiManager& in_ui_manager,
		//	const float in_time_delta,
		//	const UiInputParam& in_input_param,
		//	DscRender::IRenderTarget* const in_external_render_target_or_null = nullptr
		//);

		//virtual void UiRootDraw(
		//	UiManager& in_ui_manager,
		//	DscDag::DagCollection& in_dag_collection,
		//	DscRenderResource::Frame& in_frame,
		//	const bool in_force_draw,
		//	DscRender::IRenderTarget* const in_external_render_target_or_null = nullptr
		//);

	};
}



