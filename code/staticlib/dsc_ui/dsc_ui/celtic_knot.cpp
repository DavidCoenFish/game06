#include "celtic_knot.h"
#include "screen_quad.h"

#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_render\draw_system.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\constant_buffer_info.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\render_target_pool.h>
#include <dsc_render_resource\render_target_texture.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\shader_pipeline_state_data.h>
#include <dsc_render_resource\shader_resource.h>
#include <dsc_render_resource\shader_resource_info.h>

namespace
{
    struct TKnotConstantBuffer
    {
        // we want to know the texture size so we can smooth pixel edges
        float _texture_size[4];
    };

    struct TDataConstantBuffer
    {
        // data size [x,y] for the celtic knot data map, then [z,w] of the render target size
        float _texture_size[4];
    };

    struct TFillKnotConstantBuffer
    {
        // we want to know the texture size so we can smooth pixel edges
        float _texture_size_knot_size[4];
        float _data_size[4];
    };
}

DscUi::CelticKnot::CelticKnot(
	DscRender::DrawSystem& in_draw_system,
	DscCommon::FileSystem& in_file_system
)
{
    // _knot_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "celtic", "knot_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "celtic", "knot_ps.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }
        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            DscUi::ScreenQuad::GetInputElementDesc(),
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
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
        _knot_shader_constant_buffer = _knot_shader->MakeShaderConstantBuffer(&in_draw_system);
    }

    //_data_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "celtic", "data_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "celtic", "data_ps.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }
        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            DscUi::ScreenQuad::GetInputElementDesc(),
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
                TDataConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _data_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
        _data_shader_constant_buffer = _data_shader->MakeShaderConstantBuffer(&in_draw_system);
    }

    // _fill_knot_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "celtic", "fill_knot_vs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "celtic", "fill_knot_ps.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }
        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            DscUi::ScreenQuad::GetInputElementDesc(),
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
                nullptr, // data render target
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
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
        _fill_knot_shader_constant_buffer = _fill_knot_shader->MakeShaderConstantBuffer(&in_draw_system);
    }
}

DscUi::CelticKnot::~CelticKnot()
{
	//nop
}

