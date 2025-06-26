#include "i_ui_component.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>

DscUi::IUiComponent::~IUiComponent()
{
	// nop
}

void DscUi::IUiComponent::Draw(
	DscRenderResource::Frame&,
	DscRender::IRenderTarget&,
	const float
)
{
	//nop
}

const DscCommon::VectorInt2 DscUi::IUiComponent::ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size, const float)
{
	return in_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildAvaliableSize(const DscCommon::VectorInt2& in_our_desired_size, const int32, const float) const
{
	return in_our_desired_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildGeometrySize(const DscCommon::VectorInt2&, const DscCommon::VectorInt2& in_child_avaliable_size) const
{
	return in_child_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildGeometryOffset(const DscCommon::VectorInt2&, const int32, const float) const
{
	return DscCommon::VectorInt2::s_zero;
}

void DscUi::IUiComponent::SetClearColour(const DscCommon::VectorFloat4&)
{
	//nop
}

const bool DscUi::IUiComponent::HasManualScrollX() const
{
	return false;
}

const bool DscUi::IUiComponent::HasManualScrollY() const
{
	return false;
}

void DscUi::IUiComponent::SetManualScrollX(const float)
{
	//nop
}

void DscUi::IUiComponent::SetManualScrollY(const float)
{
	//nop
}

void DscUi::IUiComponent::SetNode(DscDag::NodeToken, DscDag::NodeToken, DscDag::NodeToken, DscDag::NodeToken)
{
	DSC_ASSERT_ALWAYS("unimplemented");
}

