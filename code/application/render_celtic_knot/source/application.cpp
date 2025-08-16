#include "application.h"
#include <dsc_common/dsc_common.h>

#include <dsc_common/i_file_overlay.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/vector_int2.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/constant_buffer_info.h>
#include <dsc_render_resource/geometry_generic.h>
#include <dsc_render_resource/render_target_texture.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_pipeline_state_data.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/shader_resource_info.h>
#include <dsc_render_resource/shader_constant_buffer.h>

namespace
{
    struct TKnotConstantBuffer
    {
        // we want to know the texture size so we can smooth pixel edges
        float _texture_size[4]; 
    };

    struct TFillKnotConstantBuffer
    {
        // we want to know the texture size so we can smooth pixel edges
        float _texture_size_knot_size[4];
        float _data_size[4];
    };
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
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        });

    // _knot_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == _file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "knot_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == _file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "knot_ps.cso")))
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
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TKnotConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _knot_shader = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
        _knot_shader_constant_buffer = _knot_shader->MakeShaderConstantBuffer(_draw_system.get());
    }

    // geometry
    {
        std::vector<uint8_t> vertex_data = DscRenderResource::GeometryGeneric::FactoryArrayLiteral(
            {
                -1.0f, -1.0f, 0.0f, 0.0f,
                -1.0f, 1.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
            }
        );
        _geometry_generic = std::make_shared<DscRenderResource::GeometryGeneric>(
            _draw_system.get(),
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            input_element_desc_array,
            vertex_data,
            4
            );
    }

    //_knot_render_target
    {
        std::vector < DscRender::RenderTargetFormatData > target_format_data_array = {};
        target_format_data_array.push_back(
            DscRender::RenderTargetFormatData(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                true,
                DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 1.0f)
            )
        );

        _knot_render_target = std::make_shared<DscRenderResource::RenderTargetTexture>(
            _draw_system.get(),
            target_format_data_array,
            DscRender::RenderTargetDepthData(),
            DscCommon::VectorInt2(_k_knot_texture_size, _k_knot_texture_size)
            );
    }

    //_texture
    {
        std::vector<uint8_t> texture_data = {
              255,  0,  0,  255,
              0,  255, 255,  0,
              0,255, 255,  0,
              0,  0,  0,  255,
            };

        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
            _k_texture_size_x, //UINT64 Width;
            _k_texture_size_y, //UINT Height;
            1, //UINT16 DepthOrArraySize;
            1, //UINT16 MipLevels;
            DXGI_FORMAT_R8_UNORM, //DXGI_FORMAT Format;
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

        _texture = std::make_shared<DscRenderResource::ShaderResource>(
            _draw_system.get(),
            _draw_system->MakeHeapWrapperCbvSrvUav(), //const std::shared_ptr < HeapWrapperItem >&in_shader_resource,
            desc, //const D3D12_RESOURCE_DESC & in_desc,
            shader_resource_view_desc, //const D3D12_SHADER_RESOURCE_VIEW_DESC & in_shader_resource_view_desc,
            texture_data //const std::vector<uint8_t>&in_data
            );
    }

    // _fill_knot_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == _file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "fill_knot_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == _file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "fill_knot_ps.cso")))
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
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                _texture->GetHeapWrapperItem(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        array_shader_resource_info.push_back(
            DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                nullptr, // render target shader resource
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TFillKnotConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _fill_knot_shader = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
        _fill_knot_shader_constant_buffer = _fill_knot_shader->MakeShaderConstantBuffer(_draw_system.get());
    }
}

Application::~Application()
{
    _fill_knot_shader.reset();
    _fill_knot_shader_constant_buffer.reset();
    _texture.reset();
    _knot_render_target.reset();
    _geometry_generic.reset();
    _knot_shader_constant_buffer.reset();
    _knot_shader.reset();
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

        frame->SetRenderTargetTexture(_knot_render_target);
        {
            auto& buffer = _knot_shader_constant_buffer->GetConstant<TKnotConstantBuffer>(0);
            buffer._texture_size[0] = static_cast<float>(_k_knot_texture_size);
        }
        frame->SetShader(_knot_shader, _knot_shader_constant_buffer);
        frame->Draw(_geometry_generic);

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer());
        {
            const auto render_size = _draw_system->GetRenderTargetBackBuffer()->GetSize();
            auto& buffer = _fill_knot_shader_constant_buffer->GetConstant<TFillKnotConstantBuffer>(0);
            buffer._texture_size_knot_size[0] = static_cast<float>(render_size.GetX());
            buffer._texture_size_knot_size[1] = static_cast<float>(render_size.GetY());
            buffer._texture_size_knot_size[2] = static_cast<float>(_k_knot_texture_size);
            buffer._data_size[0] = static_cast<float>(_k_texture_size_x);
            buffer._data_size[1] = static_cast<float>(_k_texture_size_y);
        }
        _fill_knot_shader->SetShaderResourceViewHandle(0, _texture->GetHeapWrapperItem());
        _fill_knot_shader->SetShaderResourceViewHandle(1, _knot_render_target->GetShaderResourceHeapWrapperItem());
        frame->SetShader(_fill_knot_shader, _fill_knot_shader_constant_buffer);
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

