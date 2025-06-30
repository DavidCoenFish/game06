#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_VERSION "DSC_VERSION"

/*
get the version nb of the last commit
*/
namespace DscVersion
{
	//git describe --long
	//v0.1-0-g108976e
	const char* const GetGitRevision();
	//2025-06-12T19:02:13 (timestamp of the last commit)
	const char* const GetTimestamp();
	const char* const GetPlatform();
	const char* const GetConfiguration();

}// namespace

#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#endif //#ifndef DSC_BFF_BUILD

