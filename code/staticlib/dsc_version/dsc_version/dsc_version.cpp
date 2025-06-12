#include "dsc_version.h"

const char* const DscVersion::GetGitRevision()
{
	return 
#include "git_revision.txt"
		;
}

const char* const DscVersion::GetDate()
{
	return __TIMESTAMP__;
}

const char* const DscVersion::GetVersionString()
{
	return
#include "version_string.txt"
		;
}

const char* const DscVersion::GetPlatform()
{
	return "x64";
}

const char* const DscVersion::GetConfiguration()
{
#if defined(DSC_RELEASE)
	return "Release";
#elif defined(DSC_DEBUG)
	return "Debug";
#else
	return "<unknown>";
#endif
}

