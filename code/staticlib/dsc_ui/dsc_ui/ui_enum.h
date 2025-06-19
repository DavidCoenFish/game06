#pragma once
#include "dsc_ui.h"

namespace DscUi
{
	enum class UiRootNodeInputIndex
	{
		TFrame = 0,
		TForceDraw,
		TClearOnDraw,
		TDeviceRestore, // the d3dx12 device was reset and then restored, all gpu data was potentially invalidated
		TRenderTarget,
		TTargetSize,
		TUiScale,
		//TTimeAccumulation,
		TUiComponent
	};

	enum class UiNodeInputIndex
	{
		TFrame = 0,
		TUiScale,
		TUiComponent,
		TRenderTargetPoolTexture
	};
}