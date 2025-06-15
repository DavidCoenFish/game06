#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_VERSION "DSC_VERSION"

/*
get the version nb of the last commit
*/
namespace DscVersion
{
	//git rev-parse HEAD
	//78fb12afa2ff329078582eaa53edab0bc20aa0de (last commit hash)
	const char* const GetGitRevision();
	//2025-06-12T19:02:13 (timestamp of the last commit)
	const char* const GetTimestamp();
	// 0.1.30791 (major, minor, patch) currently incrementing patch on each commit
	const char* const GetVersionString();
	const char* const GetPlatform();
	const char* const GetConfiguration();

}// namespace

#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#endif //#ifndef DSC_BFF_BUILD

