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
	enum class TUiFlow : uint8
	{
		THorizontal,
		TVertical,
		TCount
	};

	enum class TUiComponentBehaviour : uint32
	{
		TNone = 0,
		//TVisible = 1 << 0, ? why have the node in the tree if it is invisible
		THasManualScrollX = 1 << 0,
		THasManualScrollY = 1 << 1,
	};

	enum class TUiRootNodeGroup : uint8
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

		//TInputState // touch pos, keys down, gamepad...

		TCount
	};

	typedef DscDag::DagGroup<TUiRootNodeGroup, static_cast<std::size_t>(TUiRootNodeGroup::TCount)> DagGroupUiRootNode;

	enum class TUiParentNodeGroup : uint8
	{
		TUiComponent, // the ui dag node component
		TUiAvaliableSize, // the avaliable size this node had avaliable to it, used for child geometry size and geometry offset
		TUiDesiredSize,
		TUiRenderSize, // the eventually calculated viewport size of the render target. for stack components, this may need child geoemtry size and geometry offset to be calculated. 
						// possibly remove TUiRenderSize and caculate it as the viewport size of the render target?
						// no, it is needed to pass info to child
		TUiGeometrySize, 
		TDraw, // returns the render target that this ui component draws to

		TUiPanelShaderConstant, // kind of goes with TDraw (which returns the shader resrource/ render target texture for drawing

		TCount
	};

	typedef DscDag::DagGroup<TUiParentNodeGroup, static_cast<std::size_t>(TUiParentNodeGroup::TCount)> DagGroupUiParentNode;

	// ment to be the group of nodes a UiComponent may need to write to
	// in_shader_constant here feels like a bit of overkill, we dont write to it, but saves collecting 10 or so bits of data to calculate it.. alternative is to have it higher in the UiComponent like UiComponentCanvas? UiComponentStack?
	enum class TUiComponentGroup : uint8
	{
		TParentChildIndex,
		TClearColourNode,
		TManualScrollX,
		TManualScrollY,
		TCount
	};
	typedef DscDag::DagGroup<TUiComponentGroup, static_cast<std::size_t>(TUiComponentGroup::TCount)> DagGroupUiComponent;


}