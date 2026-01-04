#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_STATISTICS "DSC_STATISTICS"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#include "bookmark.h"
#include "bookmarks_per_second.h"
#include "event_store.h"
#include "i_event.h"
#include "i_event_derrived.h"
#include "value_int.h"

#endif //#ifndef DSC_BFF_BUILD
