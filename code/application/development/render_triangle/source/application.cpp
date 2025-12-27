#include <dsc_common/dsc_common.h>
#include "application.h"

#include <dsc_common/i_file_overlay.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/geometry_generic.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_pipeline_state_data.h>

namespace
{
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::Factory(in_hwnd);

    std::vector < D3D12_INPUT_ELEMENT_DESC > input_element_desc_array;
    input_element_desc_array.push_back(D3D12_INPUT_ELEMENT_DESC
        {
            "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        });
    input_element_desc_array.push_back(D3D12_INPUT_ELEMENT_DESC
        {
            "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        });

    std::vector<uint8> vertex_shader_data;
    if (false == _file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "test_vs.cso")))
    {
        DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load vertex shader\n");
    }
    std::vector<uint8> pixel_shader_data;
    if (false == _file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "test_ps.cso")))
    {
        DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load pixel shader\n");
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
    _shader = std::make_shared<DscRenderResource::Shader>(
        _draw_system.get(),
        shader_pipeline_state_data,
        vertex_shader_data,
        std::vector<uint8_t>(),
        pixel_shader_data
        );

    std::vector<uint8_t> vertex_data = DscRenderResource::GeometryGeneric::FactoryArrayLiteral(
        {
            0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f
        }
    );

    _geometry_generic = std::make_shared<DscRenderResource::GeometryGeneric>(
        _draw_system.get(),
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        input_element_desc_array,
        vertex_data,
        6
        );
}

Application::~Application()
{
    _geometry_generic.reset();
    _shader.reset();
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }
    _draw_system.reset();
    _file_system.reset();
}

const bool Application::Update()
{
    BaseType::Update();
    if (_draw_system && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());
        frame->SetShader(_shader);
        frame->Draw(_geometry_generic);
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

