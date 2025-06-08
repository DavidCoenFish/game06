#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_TEXT "DSC_TEXT"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

//G:\development\game06\code\staticlib\dsc_text\dsc_text\dsc_text.h
#include "..\..\..\sdk\free_type\Include\ft2build.h"
#include "..\..\..\sdk\free_type\Include\FreeType\config\ftheader.h"
#include "..\..\..\sdk\harf_buzz\Include\hb.h"
#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"
#include "..\..\dsc_locale\dsc_locale\dsc_locale.h"

#include "glyph.h"
#include "glyph_atlas_row.h"
#include "glyph_atlas_texture.h"
#include "glyph_collection_icon.h"
#include "glyph_collection_text.h"
#include "i_text_run.h"
#include "text_locale.h"
#include "text_manager.h"
#include "text_pre_vertex.h"
#include "text_run_icon.h"
#include "text_run_text.h"

#endif //#ifndef DSC_BFF_BUILD