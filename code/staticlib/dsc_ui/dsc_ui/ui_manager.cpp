#include "ui_manager.h"
#include "screen_quad.h"
#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_state.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_common\vector_float2.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>
#include <dsc_dag_render\dag_resource.h>
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
#include <dsc_render_resource\shader_resource_info.h>

namespace
{
    static const std::vector<D3D12_INPUT_ELEMENT_DESC> s_input_element_desc_array({
        D3D12_INPUT_ELEMENT_DESC
        {
            "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        }
        });

    const DscUi::TUiDrawType GetDrawTypeFromComponentType(const DscUi::TUiComponentType in_component_type)
    {
        switch (in_component_type)
        {
        default:
            DSC_ASSERT_ALWAYS("missing case");
            break;
        case DscUi::TUiComponentType::TGridFill:
            return DscUi::TUiDrawType::TGridFill;
        }
        return DscUi::TUiDrawType::TCount;
    }

    const DscUi::TUiDrawType GetDrawTypeFromEffectType(const DscUi::TUiEffectType in_effect_type)
    {
        switch (in_effect_type)
        {
        default:
            DSC_ASSERT_ALWAYS("missing case");
            break;
        case DscUi::TUiEffectType::TEffectCorner:
            return DscUi::TUiDrawType::TEffectCorner;
        case DscUi::TUiEffectType::TEffectDropShadow:
            return DscUi::TUiDrawType::TEffectDropShadow;
        case DscUi::TUiEffectType::TEffectInnerShadow:
            return DscUi::TUiDrawType::TEffectInnerShadow;
        case DscUi::TUiEffectType::TEffectStroke:
            return DscUi::TUiDrawType::TEffectStroke;
        }
        return DscUi::TUiDrawType::TCount;
    }

    DscDag::NodeToken MakeUiRenderTargetNode(
        DscRender::DrawSystem& in_draw_system,
        DscRenderResource::RenderTargetPool& in_render_target_pool,
        DscDag::DagCollection& in_dag_collection,
        DscDag::NodeToken in_clear_colour,
        DscDag::NodeToken in_request_size_node)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate<std::shared_ptr<DscUi::UiRenderTarget>>([&in_render_target_pool, &in_draw_system](std::shared_ptr<DscUi::UiRenderTarget>& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2 request_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorFloat4 clear_colour = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[1]);

            if (nullptr == value)
            {
                value = std::make_shared<DscUi::UiRenderTarget>(true);
            }

            std::vector<DscRender::RenderTargetFormatData> target_format_data_array = {};
            target_format_data_array.push_back(
                DscRender::RenderTargetFormatData(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    true,
                    clear_colour
                )
            );

            value->UpdateRenderTargetPool(
                in_draw_system,
                in_render_target_pool,
                request_size,
                clear_colour
                );

        } DSC_DEBUG_ONLY(DSC_COMMA "ui render target"));

        DscDag::DagCollection::LinkIndexNodes(0, in_request_size_node, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_clear_colour, node);

        return node;
    }

    DscDag::NodeToken MakeEffectDrawNode(
        const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_geometry,
        const std::shared_ptr<DscRenderResource::Shader>& in_shader,
        DscDag::DagCollection& in_dag_collection,
        DscRender::DrawSystem& in_draw_system,
        DscDag::NodeToken in_frame_node,
        DscDag::NodeToken in_ui_render_target_node,
        DscDag::NodeToken in_ui_scale,
        DscDag::NodeToken in_effect_param,
        DscDag::NodeToken in_effect_tint,
        const std::vector<DscDag::NodeToken>& in_array_input_stack,
        const int32 in_input_texture_count
        DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_geometry;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = in_shader;
        DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader, in_input_texture_count](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscRenderResource::Frame* const frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<DscUi::UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[3]);
            const DscCommon::VectorFloat4 effect_param = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[4]);
            const DscCommon::VectorFloat4 effect_tint = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[5]);

            const DscCommon::VectorInt2 viewport_size = ui_render_target->GetViewportSize();

            auto& buffer = shader_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
            buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
            buffer._width_height[1] = static_cast<float>(viewport_size.GetY());
            buffer._effect_param[0] = effect_param[0] * ui_scale;
            buffer._effect_param[1] = effect_param[1] * ui_scale;
            buffer._effect_param[2] = effect_param[2] * ui_scale;
            buffer._effect_param[3] = effect_param[3] * ui_scale;
            buffer._effect_tint[0] = effect_tint[0];
            buffer._effect_tint[1] = effect_tint[1];
            buffer._effect_tint[2] = effect_tint[2];
            buffer._effect_tint[3] = effect_tint[3];

            std::shared_ptr<DscRenderResource::Shader> shader = weak_shader.lock();
            DSC_ASSERT(nullptr != shader, "invalid state");

            if (0 < in_input_texture_count)
            {
                DscUi::UiRenderTarget* const input_texture = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(in_input_array[6]);
                const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
                buffer._texture_param_0[0] = static_cast<float>(input_texture_viewport_size.GetX());
                buffer._texture_param_0[1] = static_cast<float>(input_texture_viewport_size.GetY());
                const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
                buffer._texture_param_0[2] = static_cast<float>(input_texture_size.GetX());
                buffer._texture_param_0[3] = static_cast<float>(input_texture_size.GetY());
                shader->SetShaderResourceViewHandle(0, input_texture->GetTexture());
            }
            if (1 < in_input_texture_count)
            {
                DscUi::UiRenderTarget* const input_texture = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(in_input_array[7]);
                const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
                buffer._texture_param_0[0] = static_cast<float>(input_texture_viewport_size.GetX());
                buffer._texture_param_0[1] = static_cast<float>(input_texture_viewport_size.GetY());
                const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
                buffer._texture_param_0[2] = static_cast<float>(input_texture_size.GetX());
                buffer._texture_param_0[3] = static_cast<float>(input_texture_size.GetY());
                shader->SetShaderResourceViewHandle(1, input_texture->GetTexture());
            }

            ui_render_target->ActivateRenderTarget(*frame);
            frame->SetShader(shader, shader_buffer);
            frame->Draw(weak_geometry.lock());
            frame->SetRenderTarget(nullptr);

            out_value = ui_render_target.get();
        }
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name + "Draw"));

        auto shader_buffer = in_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(3, in_ui_scale, result_node);
        DscDag::DagCollection::LinkIndexNodes(4, in_effect_param, result_node);
        DscDag::DagCollection::LinkIndexNodes(5, in_effect_tint, result_node);
        for (int32 index = 0; index < in_input_texture_count; ++index)
        {
            DSC_ASSERT(0 < in_array_input_stack.size(), "invalid state");
            DscDag::DagCollection::LinkIndexNodes(6 + index, in_array_input_stack[in_array_input_stack.size() - 1 - index], result_node);
        }

        return result_node;
    }

} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection)
{
    _dag_resource = DscDagRender::DagResource::Factory(&in_draw_system, &in_dag_collection);
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

    //_full_target_quad
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        _full_target_quad = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            ScreenQuad::GetInputElementDesc(),
            vertex_raw_data,
            4
            );
    }

    //_ui_panel_geometry
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        _ui_panel_geometry = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            s_input_element_desc_array,
            vertex_raw_data,
            2
            );
    }

    // _debug_grid_shader
	{
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "debug_grid_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "debug_grid_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
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
                TFillConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _debug_grid_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );
	}

    // _ui_panel_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            s_input_element_desc_array,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TUiPanelShaderConstantBuffer(),
                D3D12_SHADER_VISIBILITY_VERTEX
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // Factory sampler for subpixel accuracy, if we are scrolling, allow possitioning on non pixel boundaries, FactoryDataSampler is goof to force pixel boundiries if needed
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _ui_panel_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    // _image_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "image_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "image_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // Factory sampler for subpixel accuracy, may not be 1:1 pixels source to screen size
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _image_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info
            );
    }

    // _effect_round_corner_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_round_corner_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_round_corner_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // data sampiler as expecting to be reading source texture at 1:1 scale
            DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _effect_round_corner_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    //_effect_drop_shadow_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_drop_shadow_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_drop_shadow_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // default sampiler as drop shadow tries to sample at the corner of 4 pixels to reduce sample calls
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _effect_drop_shadow_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    //_effect_inner_shadow_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_inner_shadow_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_inner_shadow_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // default sampiler as drop shadow tries to sample at the corner of 4 pixels to reduce sample calls
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _effect_inner_shadow_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    // _effect_stroke_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_stroke_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_stroke_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // default sampiler as stroke tries to sample at the corner of 4 pixel block to reduce sample calls
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _effect_stroke_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }
}

DscUi::UiManager::~UiManager()
{
    //nop
}

std::shared_ptr<DscUi::UiRenderTarget> DscUi::UiManager::MakeUiRenderTarget(
    DscRender::IRenderTarget* const in_render_target,
    const bool in_allow_clear_on_draw
)
{
    return std::make_shared<DscUi::UiRenderTarget>(in_render_target, in_allow_clear_on_draw);
}

std::shared_ptr<DscUi::UiRenderTarget> DscUi::UiManager::MakeUiRenderTarget(
    const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
    const bool in_allow_clear_on_draw
)
{
    return std::make_shared<DscUi::UiRenderTarget>(in_render_target_texture, in_allow_clear_on_draw);
}

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentGridFill()
{
    return TComponentConstructionHelper({ TUiComponentType::TGridFill});
}

DscUi::UiRootNodeGroup DscUi::UiManager::MakeRootNode(
    const TComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::shared_ptr<UiRenderTarget>& in_ui_render_target,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    )
{
    UiRootNodeGroup result;

    result.SetNodeToken(TUiRootNodeGroup::TFrame, in_dag_collection.CreateValue<DscRenderResource::Frame*>(
        (DscRenderResource::Frame*)(nullptr),
        DscDag::CallbackNever<DscRenderResource::Frame*>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "frame")));

    result.SetNodeToken(TUiRootNodeGroup::TTimeDelta, in_dag_collection.CreateValue<float>(
        0.0f,
        DscDag::CallbackNoZero<float>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "time delta")));

    result.SetNodeToken(TUiRootNodeGroup::TUiScale, in_dag_collection.CreateValue(
        1.0f,
        DscDag::CallbackOnValueChange<float>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "ui scale")));

    result.SetNodeToken(TUiRootNodeGroup::TInputState, in_dag_collection.CreateValue(
        UiInputState(),
        DscDag::CallbackOnSetValue<UiInputState>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "input state")));

    result.SetNodeToken(TUiRootNodeGroup::TUiComponentType, in_dag_collection.CreateValue(
        in_construction_helper._component_type,
        DscDag::CallbackOnValueChange<TUiComponentType>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "ui component type")));

    result.SetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue<std::vector<UiNodeGroup>>(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    result.SetNodeToken(TUiRootNodeGroup::TForceDraw, in_dag_collection.CreateValue<bool>(
        false,
        DscDag::CallbackNoZero<bool>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "force draw")));

    result.SetNodeToken(TUiRootNodeGroup::TUiRenderTarget, in_dag_collection.CreateValue(
        in_ui_render_target,
        DscDag::CallbackOnSetValue<std::shared_ptr<DscUi::UiRenderTarget>>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "ui render target")));

    result.SetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize, in_dag_collection.CreateValue(
        DscCommon::VectorInt2::s_zero,
        DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "render target size")));

    result.SetNodeToken(TUiRootNodeGroup::TScreenSpaceSize, in_dag_collection.CreateValue(
        DscCommon::VectorFloat4::s_zero,
        DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "screen space size")));

    //result.SetNodeToken(TUiRootNodeGroup::TEffectParamArray, in_dag_collection.CreateValue(
    //    std::vector<DscDag::NodeToken>(),
    //    DscDag::CallbackOnSetValue<std::vector<DscDag::NodeToken>>::Function
    //    DSC_DEBUG_ONLY(DSC_COMMA "effect param array")));

    auto draw_node = MakeDrawStack(
        GetDrawTypeFromComponentType(in_construction_helper._component_type), //TUiDrawType
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        result.GetNodeToken(TUiRootNodeGroup::TFrame),
        result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize),
        result.GetNodeToken(TUiRootNodeGroup::TUiScale),
        result.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget),
        nullptr
        DSC_DEBUG_ONLY(DSC_COMMA "root draw")
    );

    // if force draw is true, we just need to re apply the last draw step, even if nothing else has changed
    DscDag::DagCollection::LinkNodes(result.GetNodeToken(TUiRootNodeGroup::TForceDraw), draw_node);
    DscDag::DagCollection::LinkNodes(result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize), draw_node);

    result.SetNodeToken(TUiRootNodeGroup::TDrawNode, draw_node);

    result.Validate();

    return result;
}

void DscUi::UiManager::Draw(
    const UiRootNodeGroup& in_root_node_group,
    DscDag::DagCollection& in_dag_collection,
    DscRenderResource::Frame& in_frame,
    const bool in_force_draw,
    const float in_time_delta,
    const UiInputState& in_input_state,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame), &in_frame);
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TForceDraw), in_force_draw);
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TTimeDelta), in_time_delta);
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TInputState), in_input_state);

    {
        DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        if (in_external_render_target_or_null)
        {
            render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
        }

        DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize), render_target->GetViewportSize());
    }

    in_dag_collection.ResolveDirtyConditionNodes();

    in_root_node_group.GetNodeToken(TUiRootNodeGroup::TDrawNode)->Update();

    return;
}

DscDag::NodeToken DscUi::UiManager::MakeDrawStack(
    const TUiDrawType in_type,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array,
    DscDag::NodeToken in_frame_node,
    // we don't dirty on ui render target being set, so have a render target viewport size which dirties on size change
    DscDag::NodeToken in_render_target_viewport_size_node,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_last_render_target_or_null,
    DscDag::NodeToken in_clear_colour_or_null
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    std::vector<DscDag::NodeToken> array_draw_nodes;
    DscDag::NodeToken last_draw_node = nullptr;

    {
        DscDag::NodeToken ui_render_target_node = nullptr;
        if ((0 == in_effect_array.size()) && (nullptr != in_last_render_target_or_null))
        {
            ui_render_target_node = in_last_render_target_or_null;
        }
        else
        {
            DscDag::NodeToken component_clear_colour = in_clear_colour_or_null;
            // so, the root node, if it has an effect, will not have a clear colour?
            //DSC_ASSERT(nullptr != in_clear_colour_or_null, "invalid param for this case");
            if (nullptr == component_clear_colour)
            {
                component_clear_colour = in_dag_collection.CreateValue(
                    DscCommon::VectorFloat4::s_zero,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function
                    DSC_DEBUG_ONLY(DSC_COMMA "effect clear colour"));
            }

            ui_render_target_node = MakeUiRenderTargetNode(
                in_draw_system, 
                *_render_target_pool, 
                in_dag_collection, 
                component_clear_colour,
                in_render_target_viewport_size_node);
        }

        last_draw_node = MakeDrawNode(
            in_type,
            in_draw_system,
            in_dag_collection,
            array_draw_nodes,
            in_frame_node,
            ui_render_target_node,
            in_ui_scale,
            nullptr,
            nullptr
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
        );
        array_draw_nodes.push_back(last_draw_node);
    }
    if (0 < in_effect_array.size())
    {
        std::vector<DscDag::NodeToken> array_effect_param_nodes;

        for (size_t index = 0; index < in_effect_array.size(); ++index)
        {
            DscDag::NodeToken ui_render_target_node = nullptr;
            if ((index == in_effect_array.size() - 1) && (nullptr != in_last_render_target_or_null))
            {
                ui_render_target_node = in_last_render_target_or_null;
            }
            else
            {
                DscDag::NodeToken effect_clear_colour = in_dag_collection.CreateValue(
                    DscCommon::VectorFloat4::s_zero,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function
                    DSC_DEBUG_ONLY(DSC_COMMA "effect clear colour"));

                ui_render_target_node = MakeUiRenderTargetNode(
                    in_draw_system, 
                    *_render_target_pool, 
                    in_dag_collection, 
                    effect_clear_colour, 
                    in_render_target_viewport_size_node);
            }

            const auto& effect_data = in_effect_array[index];
            DscDag::NodeToken effect_param = in_dag_collection.CreateValue(
                effect_data._effect_param,
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function
                DSC_DEBUG_ONLY(DSC_COMMA "effect param"));
            DscDag::NodeToken effect_tint = in_dag_collection.CreateValue(
                effect_data._effect_param_tint,
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function
                DSC_DEBUG_ONLY(DSC_COMMA "effect tint"));
            // todo, keep ref of effect_param_node and effect_tint_node in a ui component resource group?

            last_draw_node = MakeDrawNode(
                GetDrawTypeFromEffectType(effect_data._effect_type),
                in_draw_system,
                in_dag_collection,
                array_draw_nodes,
                in_frame_node,
                ui_render_target_node,
                in_ui_scale,
                effect_param,
                effect_tint
                DSC_DEBUG_ONLY(DSC_COMMA "effect draw")
            );
            array_draw_nodes.push_back(last_draw_node);
        }
    }

    DSC_ASSERT(nullptr != last_draw_node, "invalid state");

    return last_draw_node;
}


DscDag::NodeToken DscUi::UiManager::MakeDrawNode(
    const TUiDrawType in_type,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::vector<DscDag::NodeToken>& in_array_input_stack,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_param_or_null,
    DscDag::NodeToken in_effect_tint_or_null
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    DSC_ASSERT(nullptr != in_frame_node, "invalid param");
    //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "MakeDrawNode in_frame_node:%s TYPE:%s\n", in_frame_node->GetTypeInfo().name(), typeid(DscRenderResource::Frame*).name());
    DSC_ASSERT(in_frame_node->GetTypeInfo() == typeid(DscRenderResource::Frame*), "invalid param");

    DSC_ASSERT(nullptr != in_ui_scale, "invalid param");
    //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "MakeDrawNode in_ui_scale:%s TYPE:%s\n", in_ui_scale->GetTypeInfo().name(), typeid(float).name());
    DSC_ASSERT(in_ui_scale->GetTypeInfo() == typeid(float), "invalid param");

    DscDag::NodeToken result_node = {};
    switch (in_type)
    {
    default:
        DSC_ASSERT_ALWAYS("missing switch");
        break;
    case TUiDrawType::TGridFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_full_target_quad = _full_target_quad;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _debug_grid_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_full_target_quad, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");

            const DscCommon::VectorInt2 viewport_size = ui_render_target->GetViewportSize();

            auto& buffer = shader_buffer->GetConstant<TFillConstantBuffer>(0);
            buffer._value[0] = static_cast<float>(viewport_size.GetX());
            buffer._value[1] = static_cast<float>(viewport_size.GetY());

            ui_render_target->ActivateRenderTarget(*frame);
            frame->SetShader(weak_shader.lock(), shader_buffer);
            frame->Draw(weak_full_target_quad.lock());
            frame->SetRenderTarget(nullptr);

            out_value = ui_render_target.get();
        }
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        auto shader_buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
    }
    break;
    case TUiDrawType::TEffectCorner:
        result_node = MakeEffectDrawNode(
            _full_target_quad,
            _effect_round_corner_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
    break;
    case TUiDrawType::TEffectDropShadow:
        result_node = MakeEffectDrawNode(
            _full_target_quad,
            _effect_drop_shadow_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectInnerShadow:
        result_node = MakeEffectDrawNode(
            _full_target_quad,
            _effect_inner_shadow_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectStroke:
        result_node = MakeEffectDrawNode(
            _full_target_quad,
            _effect_stroke_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;

    }
    return result_node;
}
