#include "application.h"
#include <dsc_common/dsc_common.h>

#include <dsc_common/i_file_overlay.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/vector_int2.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_text/text_manager.h>
#include <dsc_onscreen_version/onscreen_version.h>

namespace
{
    struct TKnotConstantBuffer
    {
        // we want to know the texture size so we can smooth pixel edges
        float _texture_size[4]; 
    };
}

Application::Resources::Resources() 
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::Factory(in_hwnd);

    _resources = std::make_unique<Resources>();
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_unique<DscText::TextManager>(*_draw_system, *_file_system);
        _resources->_onscreen_version = std::make_unique<DscOnscreenVersion::OnscreenVersion>(*_draw_system, *_file_system, *(_resources->_text_manager));
    }

    // compute shader         std::unique_ptr<DscRenderResource::Shader> _compute_shader_advance_spring_system;
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        std::vector<uint8> compute_shader_data;
        if (false == _file_system->LoadFile(compute_shader_data, DscCommon::FileSystem::JoinPath("shader", "advance_spring_system_cs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load compute shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            input_element_desc_array,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            // DXGI_FORMAT_D32_FLOAT,
            render_target_format,
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
            );

        _compute_shader_advance_spring_system = std::make_shared<DscRenderResource::Shader>(
                _draw_system.get(),
                shader_pipeline_state_data,
                std::vector<uint8_t>(),
                std::vector<uint8_t>(),
                std::vector<uint8_t>(),
                std::vector<std::shared_ptr<ShaderResourceInfo>>(),
                std::vector<std::shared_ptr<ConstantBufferInfo>>(),
                compute_shader_data,
                std::vector<std::shared_ptr<UnorderedAccessInfo>>()
                );

    }
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
    if (_draw_system && (false == GetMinimized()))
    {
        const auto render_size = _draw_system->GetRenderTargetBackBuffer()->GetSize();
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager, true);
        }
    }

    return true;
}
void Application::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);
    if (_draw_system)
    {
        _draw_system->OnResize();
    }
    return;
}

