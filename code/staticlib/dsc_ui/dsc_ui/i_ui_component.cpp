#include "i_ui_component.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>

DscUi::IUiComponent::~IUiComponent()
{
	// nop
}

const DscCommon::VectorFloat4& DscUi::IUiComponent::GetClearColour() const
{
	return DscCommon::VectorFloat4::s_zero;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size)
{
	return in_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildAvaliableSize(const DscCommon::VectorInt2& in_our_desired_size, const int32) const
{
	return in_our_desired_size;
}

const DscCommon::VectorInt2 DscUi::IUiComponent::GetChildOffset(const DscCommon::VectorInt2&, const int32) const
{
	return DscCommon::VectorInt2::s_zero;
}

void DscUi::IUiComponent::Update(const float)
{
	//nop
}

void DscUi::IUiComponent::AddChildRef(IUiComponent*)
{
	DSC_ASSERT_ALWAYS("unimplemented");
}

void DscUi::IUiComponent::RemoveChild(IUiComponent*)
{
	DSC_ASSERT_ALWAYS("unimplemented");
}

void DscUi::IUiComponent::SetScrollTraveralPixelDistance(const DscCommon::VectorInt2&)
{
	// nop
}

void DscUi::IUiComponent::SetSrollNode(DscDag::NodeToken)
{
	DSC_ASSERT_ALWAYS("unimplemented");
}

