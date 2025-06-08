#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_RENDER_RESOURCE "DSC_RENDER_RESOURCE"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

//G:\development\game06\code\staticlib\dsc_dag\dsc_dag\dsc_dag.h
#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"

#include "constant_buffer.h"
#include "constant_buffer_info.h"
#include "dsc_render_resource.h"
#include "frame.h"
#include "geometry_generic.h"
#include "render_target_pool.h"
#include "render_target_texture.h"
#include "shader.h"
#include "shader_constant_buffer.h"
#include "shader_pipeline_state_data.h"
#include "shader_resource.h"
#include "shader_resource_info.h"
#include "shader_resource_partial_upload.h"
#include "unordered_access.h"
#include "unordered_access_info.h"

#endif //#ifndef DSC_BFF_BUILD