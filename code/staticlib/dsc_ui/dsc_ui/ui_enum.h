#pragma once
#include "dsc_ui.h"

namespace DscUi
{
	enum class UiRootNodeInputIndex
	{
		TFrame = 0,
		TRenderTarget,
		TAvaliableSize,
		TDesiredSize,
		TDeviceRestore, // the d3dx12 device was reset and then restored, all gpu data was potentially invalidated
		TUiScale,
		TForceDraw,
		TClearOnDraw,
		TUiComponent,
		TShaderConstantBuffer
	};

	enum class UiNodeInputIndex
	{
		TFrame = 0,
		TDeviceRestore,
		TUiScale,
		TUiComponent,

		TRenderTargetPoolTexture, // <= TDesiredSize <= TParentAvaliableSizeForThisChildNode

		TShaderConstantBuffer
		//TGeometry,

		// reference
		//TUiComponentParent, // 
		//TChildIndex, // what index this child is of the parent

		//TParentAvaliableSizeForThisChildNode, // how to reach this node, keep a reference in the UiNode?
		//TDesiredSize,
	};

	/*
	UiComponent
		SetDesiredSize(vector2int)
		GetAvaliableSizeForChild(index) // expect desired size to already be set
	
	*/
}