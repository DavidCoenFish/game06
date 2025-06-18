#include <dsc_common/dsc_common.h>
#include "application.h"

#include <dsc_common/i_file_overlay.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/vector_float4.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render/render_target_format_data.h>
#include <dsc_render_resource/constant_buffer_info.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/geometry_generic.h>
#include <dsc_render_resource/render_target_texture.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_pipeline_state_data.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/shader_resource_info.h>
#include <dsc_render_resource/shader_constant_buffer.h>

namespace
{
    struct TShaderConstantBuffer
    {
        float _value[4];
    };
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::Factory(in_hwnd);

    _resources = std::make_unique<Resources>();

    //triangle
    {
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
        if (false == _file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "triangle_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == _file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "triangle_ps.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to triangle load pixel shader\n");
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
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TShaderConstantBuffer(),
                D3D12_SHADER_VISIBILITY_VERTEX
            )
        );
        _resources->_shader_triangle = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );

        _resources->_shader_constant_buffer = _resources->_shader_triangle->MakeShaderConstantBuffer(
            _draw_system.get()
            );

        std::vector<uint8_t> vertex_data = DscRenderResource::GeometryGeneric::FactoryArrayLiteral(
            {
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f
            }
        );

        _resources->_geometry_triangle = std::make_shared<DscRenderResource::GeometryGeneric>(
            _draw_system.get(),
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
            input_element_desc_array,
            vertex_data,
            6
            );
    }

    // render target
    {
        std::vector < DscRender::RenderTargetFormatData > target_format_data_array = {};
        target_format_data_array.push_back(
            DscRender::RenderTargetFormatData(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                true,
                DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 1.0f)
                )
            );

        _resources->_render_target_texture = std::make_shared<DscRenderResource::RenderTargetTexture>(
            _draw_system.get(),
            target_format_data_array,
            DscRender::RenderTargetDepthData(),
            DscCommon::VectorInt2(256, 256),
            false,
            true, 
            DscCommon::VectorInt2(128, 128)
            );
    }

    // present
    {
        std::vector < D3D12_INPUT_ELEMENT_DESC > input_element_desc_array;
        input_element_desc_array.push_back(D3D12_INPUT_ELEMENT_DESC
            {
                "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
            });
        input_element_desc_array.push_back(D3D12_INPUT_ELEMENT_DESC
            {
                "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
            });

        std::vector<uint8> vertex_shader_data;
        if (false == _file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "present_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load present vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == _file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "present_ps.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load present pixel shader\n");
        }
        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            input_element_desc_array,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _resources->_shader_present = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info
            );

        std::vector<uint8_t> vertex_data = DscRenderResource::GeometryGeneric::FactoryArrayLiteral(
            {
                -0.5f, -0.5f, 0.0f, 0.0f,
                -0.5f, 0.5f, 0.0f, 1.0f,
                0.5f, -0.5f, 1.0f, 0.0f,
                0.5f, 0.5f, 1.0f, 1.0f,
            }
        );

        _resources->_geometry_present = std::make_shared<DscRenderResource::GeometryGeneric>(
            _draw_system.get(),
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            input_element_desc_array,
            vertex_data,
            4
            );
    }
}

Application::~Application()
{
    _resources.reset();
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

        {
            TShaderConstantBuffer& buffer = _resources->_shader_constant_buffer->GetConstant<TShaderConstantBuffer>(0);
            _time_accumulation += 0.01f;
            const float x = sin(_time_accumulation);
            const float y = cos(_time_accumulation);
            buffer._value[0] = x; // 1.0f; // x;
            buffer._value[1] = y; // 0.0f; // y;
            buffer._value[2] = -y; // 0.0f; //y;
            buffer._value[3] = x; // -1.0f; //-x;
        }

        frame->SetRenderTarget(_resources->_render_target_texture.get());
        frame->SetShader(_resources->_shader_triangle, _resources->_shader_constant_buffer);
        frame->Draw(_resources->_geometry_triangle);
        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());
        _resources->_shader_present->SetShaderResourceViewHandle(0, _resources->_render_target_texture->GetShaderResourceHeapWrapperItem(0));
        frame->SetShader(_resources->_shader_present);
        frame->Draw(_resources->_geometry_present);
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

