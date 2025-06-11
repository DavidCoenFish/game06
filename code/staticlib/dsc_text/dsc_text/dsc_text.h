#pragma once

#define LOG_TOPIC_DSC_TEXT "DSC_TEXT"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

//G:\development\game06\code\staticlib\dsc_text\dsc_text\dsc_text.h
#include "..\..\..\sdk\freetype_x64-windows\include\ft2build.h"
#include "..\..\..\sdk\freetype_x64-windows\include\freetype\freetype.h"
#include "..\..\..\sdk\harfbuzz_x64-windows\include\harfbuzz\hb.h"
#include "..\..\..\sdk\harfbuzz_x64-windows\include\harfbuzz\hb-ft.h"


#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"
#include "..\..\dsc_render_resource\dsc_render_resource\dsc_render_resource.h"
#include "..\..\dsc_locale\dsc_locale\dsc_locale.h"

#include "glyph.h"
#include "glyph_atlas_row.h"
#include "glyph_atlas_texture.h"
#include "glyph_collection_icon.h"
#include "glyph_collection_text.h"
#include "i_text_run.h"
#include "text_enum.h"
#include "text_locale.h"
#include "text_manager.h"
#include "text_pre_vertex.h"
#include "text_run.h"
#include "text_run_icon.h"
#include "text_run_text.h"

#endif //#ifndef DSC_BFF_BUILD


#include <dsc_common/dsc_common.h>
