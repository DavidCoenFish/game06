#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_RENDER_RESOURCE_PNG "DSC_RENDER_RESOURCE_PNG"

namespace DscCommon
{
    class FileSystem;
}

namespace DscRender
{
    class DrawSystem;
}

namespace DscRenderResource
{
    class ShaderResource;
}

namespace DscRenderResourcePng
{
    std::shared_ptr<DscRenderResource::ShaderResource> MakeShaderResource(
        DscCommon::FileSystem& in_file_system, 
        DscRender::DrawSystem& in_draw_system, 
        const std::string& in_file_path
        );
}

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"
#include "..\..\dsc_render_resource\dsc_render_resource\dsc_render_resource.h"
#include "..\..\dsc_png\dsc_png\dsc_png.h"

#endif //#ifndef DSC_BFF_BUILD