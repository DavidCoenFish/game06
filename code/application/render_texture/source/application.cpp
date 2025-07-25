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
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/shader_resource_info.h>

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
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        });

    D3D12_RESOURCE_DESC desc = {
        D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
        D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
        16, //UINT64 Width;
        16, //UINT Height;
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

    std::vector<uint8_t> texture_data = {};
    texture_data.resize(16 * 16 * 4);
    for (int index_y = 0; index_y < 16; ++index_y)
    {
        for (int index_x = 0; index_x < 16; ++index_x)
        {
            const uint8_t paint = (0 == ((index_x + index_y) & 0x01)) ? 0 : 255;
            texture_data[(((index_y * 16) + index_x) * 4) + 0] = static_cast<uint8_t>(index_y * 16);
            texture_data[(((index_y * 16) + index_x) * 4) + 1] = static_cast<uint8_t>(index_x * 16);
            texture_data[(((index_y * 16) + index_x) * 4) + 2] = paint; //static_cast<uint8_t>(0);
            texture_data[(((index_y * 16) + index_x) * 4) + 3] = static_cast<uint8_t>(255);
        }
    }

    _texture = std::make_shared<DscRenderResource::ShaderResource>(
        _draw_system.get(),
        _draw_system->MakeHeapWrapperCbvSrvUav(), //const std::shared_ptr < HeapWrapperItem >&in_shader_resource,
        desc, //const D3D12_RESOURCE_DESC & in_desc,
        shader_resource_view_desc, //const D3D12_SHADER_RESOURCE_VIEW_DESC & in_shader_resource_view_desc,
        texture_data //const std::vector<uint8_t>&in_data
        );

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
    _shader = std::make_shared<DscRenderResource::Shader>(
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

    _geometry_generic = std::make_shared<DscRenderResource::GeometryGeneric>(
        _draw_system.get(),
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
        input_element_desc_array,
        vertex_data,
        4
        );
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }
    _texture.reset();
    _geometry_generic.reset();
    _shader.reset();
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
        //_shader->SetShaderResourceViewHandle(0, _texture->GetHeapWrapperItem());
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

