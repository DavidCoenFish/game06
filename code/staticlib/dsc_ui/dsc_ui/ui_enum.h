#pragma once
#include "dsc_ui.h"
//#include <dag_group.h>

namespace DscDag
{
	template <typename ENUM, std::size_t SIZE>
	class DagGroup;
}

namespace DscUi
{
	enum class TUiRootNodeGroup
	{
		TFrame = 0,
		TDeviceRestore, // the d3dx12 device was reset and then restored, all gpu data was potentially invalidated
		TForceDraw,
		TAllowClearOnDraw, // if we share the render target with another system drawing to it? then we don't clear the render target when we set it active
		TRenderTarget,
		TRenderTargetViewportSize, // ~root avaliable size, can not be calculate from the render target as render target is set to never dirty, and we WANT to detect size changes
		TUiScale,
		TUiComponent,
		TTimeDelta,
		TDrawRoot, // getting the value of this node will trigger the frame command list of what needs to be drawn to be populated

		TCount
	};

	typedef DscDag::DagGroup<TUiRootNodeGroup, static_cast<std::size_t>(TUiRootNodeGroup::TCount)> DagGroupUiRootNode;

	//enum class UiNodeInputIndex
	//{
	//	TFrame = 0,
	//	TDeviceRestore,
	//	TRenderTargetPoolTexture,
	//	TUiScale,
	//	TUiComponent,
	//	TShaderConstant, // not used directly, but hooked into render to redraw on value change
	//};
}