#include "i_render_target.h"
#include <dsc_common/vector_int2.h>

// #include "Common/DrawSystem/IResource.h"
DscRender::IRenderTarget::IRenderTarget()// DrawSystem* const pDrawSystem)
// : IResource(pDrawSystem)

{
	return;
}

DscRender::IRenderTarget::~IRenderTarget()
{
	return;
}

const DscCommon::VectorInt2 DscRender::IRenderTarget::GetViewportSize() const
{
	return GetSize();
}

