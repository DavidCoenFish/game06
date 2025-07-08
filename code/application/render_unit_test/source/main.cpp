#include "main.h"
#include "test_util.h"
#include "test\application_ui.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/log_system.h>
#include <dsc_version/dsc_version.h>

int APIENTRY wWinMain(_In_ HINSTANCE in_hInstance,
    _In_opt_ HINSTANCE,// in_hPrevInstance,
    _In_ LPWSTR,//    in_lpCmdLine,
    _In_ int       in_nCmdShow)
{
    DscCommon::LogSystem logSystem(DscCommon::LogLevel::Diagnostic);
    DSC_LOG_INFO(LOG_TOPIC_APPLICATION, "Version time:%s %s %s %s\n", DscVersion::GetTimestamp(), DscVersion::GetGitRevision(), DscVersion::GetPlatform(), DscVersion::GetConfiguration());

    TestUtil::AddTest(ApplicationUi::Factory);

    int32 exitCode = 0;
    if (true != TestUtil::RunTests(in_hInstance, in_nCmdShow))
    {
        exitCode = -1;
    }

    return exitCode;
}