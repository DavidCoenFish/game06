#pragma once
#include "dsc_ui.h"

namespace DscUi
{
	enum class UiRootNodeInputIndex
	{
		TFrame = 0,
		TDeviceRestore, // the d3dx12 device was reset and then restored, all gpu data was potentially invalidated
		TRenderTarget,
		TAvaliableSize, // can not be calculate from the render target as render target is st to never dirty, and we WANT to detect size changes
		TUiScale,
		TUiComponent,
		TShaderConstant, // not used directly, but hooked into render to redraw on value change
		TForceDraw,
		TAllowClearOnDraw, // if we share the render target with another system drawing to it? then we don't clear the render target when we set it active

	};

	enum class UiNodeInputIndex
	{
		TFrame = 0,
		TDeviceRestore,
		TRenderTargetPoolTexture,
		TUiScale,
		TUiComponent,
		TShaderConstant, // not used directly, but hooked into render to redraw on value change
	};
}