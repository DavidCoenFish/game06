#include "ui_manager.h"
#include <dsc_common\file_system.h>
#include <dsc_render\draw_system.h>
#include <dsc_render_resource\render_target_pool.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\shader_pipeline_state_data.h>
#include <dsc_render_resource\shader_resource_info.h>

namespace
{
} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& /*in_draw_system*/, DscCommon::FileSystem& /*in_file_system*/)
{
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

/*
    // screen quad grid
	{
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_grid_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_grid_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TSizeShaderConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _screen_quad_grid_shader = std::make_unique<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );
	}

    // _screen_quad_texture_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource = {};
        array_shader_resource.push_back(DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
            nullptr,
            D3D12_SHADER_VISIBILITY_PIXEL
            ));
        _screen_quad_grid_shader = std::make_unique<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource
            );
    }

    // _ui_panel_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TUiPanelShaderConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource = {};
        array_shader_resource.push_back(DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
            nullptr,
            D3D12_SHADER_VISIBILITY_PIXEL
        ));
        _screen_quad_grid_shader = std::make_unique<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource,
            array_shader_constants_info
            );
    }
*/
}

DscUi::UiManager::~UiManager()
{
    //nop
}


//std::unique_ptr<DscRenderResource::ShaderConstantBuffer> DscUi::UiManager::MakeSizeShaderConstantBuffer(DscRender::DrawSystem& in_draw_system)
//{
//    return _screen_quad_grid_shader->MakeShaderConstantBuffer(
//        &in_draw_system
//    );
//}
//
//std::unique_ptr<DscRenderResource::ShaderConstantBuffer> DscUi::UiManager::MakeUiPanelShaderConstantBuffer(DscRender::DrawSystem& in_draw_system)
//{
//    return _ui_panel_shader->MakeShaderConstantBuffer(
//        &in_draw_system
//    );
//}
