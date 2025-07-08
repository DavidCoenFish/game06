#pragma once
#include "render_unit_test.h"
#include <dsc_common/dsc_common.h>
#include <dsc_windows/i_window_application.h>
#include <dsc_dag/dag_group.h>
#include <dsc_ui/ui_enum.h>

namespace DscCommon
{
    class FileSystem;
    class Timer;
}

namespace DscDag
{
    class DagCollection;
    class IDagNode;
    typedef IDagNode* NodeToken;
}

namespace DscRender
{
    class DrawSystem;
}

namespace DscRenderResource
{
    class GeometryGeneric;
    class RenderTargetTexture;
    class Shader;
    class ShaderConstantBuffer;
}

namespace DscDagRender
{
    class DagResource;
}

namespace DscUi
{
    class UiManager;
}

class ApplicationUi : public DscWindows::IWindowApplication
{
public:
    static DscWindows::IWindowApplication* const Factory(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight);

    ApplicationUi(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight);
    ~ApplicationUi();

    ApplicationUi() = delete;
    ApplicationUi& operator=(const ApplicationUi&) = delete;
    ApplicationUi(const ApplicationUi&) = delete;

private:
    virtual const bool Update() override;
    virtual void OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale) override;

    const bool TestCreateDeleteRootAndChild();
    const bool TestRemoveChild();

private:
    typedef DscWindows::IWindowApplication BaseType;
    std::unique_ptr<DscCommon::FileSystem> _file_system;
    std::unique_ptr<DscRender::DrawSystem> _draw_system;
};