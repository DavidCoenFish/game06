#include "application.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/timer.h>
#include <dsc_common/vector_int2.h>
#include <dsc_dag/dag_collection.h>
#include <dsc_locale/dsc_locale.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_text/text_manager.h>
#include <dsc_text/text_run.h>
#include <dsc_text/i_text_run.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_ui/i_ui_component.h>
#include <dsc_ui/ui_manager.h>
#include <dsc_ui/ui_coord.h>
#include <dsc_ui/vector_ui_coord2.h>
#include <dsc_png/dsc_png.h>

namespace
{
#if 1
    std::shared_ptr<DscRenderResource::ShaderResource> MakeShaderResource(DscCommon::FileSystem & in_file_system, DscRender::DrawSystem & in_draw_system, const std::string & in_file_path)
    {
        std::vector<uint8> data = {};
        int32 byte_per_pixel = 0;
        DscCommon::VectorInt2 size = {};

        DscPng::LoadPng(
            data,
            byte_per_pixel,
            size,
            in_file_system,
            in_file_path
            );
        DscPng::ForceRgba(
            data,
            byte_per_pixel,
            size
            );
        std::shared_ptr<DscRenderResource::ShaderResource> result = {};
        if (0 < data.size())
        {
            D3D12_RESOURCE_DESC desc = {
                D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
                D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
                (UINT64)size.GetX(), //UINT64 Width;
                (UINT)size.GetY(), //UINT Height;
                1, //UINT16 DepthOrArraySize;
                1, //UINT16 MipLevels;
                DXGI_FORMAT_R8G8B8A8_UNORM, //DXGI_FORMAT Format;
                DXGI_SAMPLE_DESC{ 1, 0 }, //DXGI_SAMPLE_DESC SampleDesc;
                D3D12_TEXTURE_LAYOUT_UNKNOWN, //D3D12_TEXTURE_LAYOUT Layout;
                D3D12_RESOURCE_FLAG_NONE //D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE //D3D12_RESOURCE_FLAGS Flags;
            };
            // Describe and create a SRV for the texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
            shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            shader_resource_view_desc.Format = desc.Format;
            shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            shader_resource_view_desc.Texture2D.MipLevels = 1;

            result = std::make_shared<DscRenderResource::ShaderResource>(
                &in_draw_system,
                in_draw_system.MakeHeapWrapperCbvSrvUav(), //const std::shared_ptr < HeapWrapperItem >&in_shader_resource,
                desc, //const D3D12_RESOURCE_DESC & in_desc,
                shader_resource_view_desc, //const D3D12_SHADER_RESOURCE_VIEW_DESC & in_shader_resource_view_desc,
                data //const std::vector<uint8_t>&in_data
                );
        }
        return result;
    }
#endif
}

Application::Resources::Resources() 
    : _ui_root_node_group(nullptr)
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f));

    _resources = std::make_unique<Resources>();

    if (nullptr != _resources)
    {
        _resources->_timer = std::make_unique<DscCommon::Timer>();
    }

    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_unique<DscText::TextManager>(*_draw_system, *_file_system);
        _resources->_onscreen_version = std::make_unique<DscOnscreenVersion::OnscreenVersion>(*_draw_system, *_file_system, *(_resources->_text_manager));
        _resources->_dag_collection = std::make_unique<DscDag::DagCollection>();
        _resources->_ui_manager = std::make_unique<DscUi::UiManager>(*_draw_system, *_file_system, *(_resources->_dag_collection));
    }

    {
        auto ui_canvas_commponent = _resources->_ui_manager->MakeComponentCanvas();
        _resources->_ui_root_node_group = _resources->_ui_manager->MakeUiRootNode(
            *_resources->_dag_collection,
            std::move(ui_canvas_commponent)
            DSC_DEBUG_ONLY(DSC_COMMA "canvas"));

        auto parent_node_group = DscUi::UiManager::ConvertUiRootNodeToParentNode(_resources->_ui_root_node_group);

        auto background_texture = MakeShaderResource(*_file_system, *_draw_system, DscCommon::FileSystem::JoinPath("data", "background", "background_00.png"));
        auto image_component = _resources->_ui_manager->MakeComponentImage(
            background_texture
            );

        _resources->_ui_manager->MakeUiNodeCanvasChild(
            *_draw_system,
            *_resources->_dag_collection,
            std::move(image_component),
            DscCommon::VectorFloat4(1.0f, 1.0f, 1.0f, 1.0f),
            _resources->_ui_root_node_group,
            parent_node_group,
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 2.2222f, DscUi::UiCoord::TMethod::TSecondaryPoroportinal), DscUi::UiCoord(0, 1.0f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f)),
            DscUi::VectorUiCoord2(DscUi::UiCoord(0, 0.5f), DscUi::UiCoord(0, 0.5f))
            );
    }

    return;
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }

    _resources.reset();
    _draw_system.reset();
    _file_system.reset();
}

const bool Application::Update()
{
    BaseType::Update();
    if (_draw_system && _resources && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        float time_delta = 0.0f;
        if (_resources && _resources->_timer)
        {
            time_delta = _resources->_timer->GetDeltaSeconds();
        }

        if (_resources->_ui_manager)
        {
            _resources->_ui_manager->UpdateUiSystem(
                _resources->_ui_root_node_group,
                time_delta
            );

            _resources->_ui_manager->DrawUiSystem(
                _resources->_ui_root_node_group,
                _draw_system->GetRenderTargetBackBuffer(),
                *frame,
                true,
                true
            );
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager);
        }

        frame.reset();
    }
    
    return true;
}
void Application::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);

    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "OnWindowSizeChanged size:%d %d scale:%f\n", in_size.GetX(), in_size.GetY(), in_monitor_scale);

    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    return;
}


