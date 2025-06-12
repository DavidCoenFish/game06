#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_VERSION "DSC_VERSION"

namespace DscVersion
{
	//git rev-parse HEAD
	//78fb12afa2ff329078582eaa53edab0bc20aa0de
	const char* const GetGitRevision();
	//
	const char* const GetDate();
	// 0.0.1.30791
	const char* const GetVersionString();
	const char* const GetPlatform();
	const char* const GetConfiguration();

}// namespace

#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#endif //#ifndef DSC_BFF_BUILD

