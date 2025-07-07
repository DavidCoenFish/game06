#include "ui_manager.h"
#include "screen_quad.h"
#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_param.h"
#include "ui_input_state.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_common\vector_float2.h>
#include <dsc_dag\dag_array_helper.h>
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
#include <dsc_render_resource\shader_resource.h>
#include <dsc_render_resource\shader_resource_info.h>
#include <dsc_text\text_manager.h>
#include <dsc_text\text_run.h>

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
        case DscUi::TUiComponentType::TDebugGrid:
            return DscUi::TUiDrawType::TDebugGrid;
        case DscUi::TUiComponentType::TFill:
            return DscUi::TUiDrawType::TFill;
        case DscUi::TUiComponentType::TImage:
            return DscUi::TUiDrawType::TImage;
        case DscUi::TUiComponentType::TCanvas:
            return DscUi::TUiDrawType::TUiPanel;
        case DscUi::TUiComponentType::TText:
            return DscUi::TUiDrawType::TText;
        case DscUi::TUiComponentType::TStack:
            return DscUi::TUiDrawType::TUiPanel;
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
        case DscUi::TUiEffectType::TEffectTint:
            return DscUi::TUiDrawType::TEffectTint;
        }
        return DscUi::TUiDrawType::TCount;
    }

    DscDag::NodeToken MakeUiRenderTargetNode(
        DscRender::DrawSystem& in_draw_system,
        DscRenderResource::RenderTargetPool& in_render_target_pool,
        DscDag::DagCollection& in_dag_collection,
        DscDag::NodeToken in_clear_colour,
        DscDag::NodeToken in_request_size_node,
        DscUi::UiComponentResourceNodeGroup& in_component_resource_group
        )
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
            }, 
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "ui render target"));

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
        const int32 in_input_texture_count,
        DscUi::UiComponentResourceNodeGroup& in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
    {
        DSC_ASSERT(nullptr != in_geometry, "invalid param");
        DSC_ASSERT(nullptr != in_shader, "invalid param");

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

                if (true == ui_render_target->ActivateRenderTarget(*frame))
                {
                    frame->SetShader(shader, shader_buffer);
                    frame->Draw(weak_geometry.lock());
                    frame->SetRenderTarget(nullptr);
                }
                out_value = ui_render_target.get();
            },
            &in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name + "Draw"));

        auto shader_buffer = in_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &in_component_resource_group
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

    std::shared_ptr<DscRenderResource::Shader> CreateEffectShader(
        DscRender::DrawSystem& in_draw_system,
        DscCommon::FileSystem& in_file_system,
        const std::string& in_vertex_shader_name,
        const std::string& in_pixel_shader_name,
        const bool in_use_data_sampler,
        const int32 in_texture_count = 1
        )
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", in_vertex_shader_name)))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", in_pixel_shader_name)))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            DscUi::ScreenQuad::GetInputElementDesc(),
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
                DscUi::TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        for (int32 index = 0; index < in_texture_count; ++index)
        {
            array_shader_resource_info.push_back(
                // data sampiler if expecting to be reading source texture at 1:1 scale (no bilinear smear)
                in_use_data_sampler ?
                DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                    nullptr,
                    D3D12_SHADER_VISIBILITY_PIXEL
                ) : 
                DscRenderResource::ShaderResourceInfo::FactorySampler(
                    nullptr,
                    D3D12_SHADER_VISIBILITY_PIXEL
                )
            );
        }
        return std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    DscUi::UiComponentResourceNodeGroup MakeComponentResourceGroup(
        DscDag::DagCollection& in_dag_collection,
        const DscUi::UiManager::TComponentConstructionHelper& in_construction_helper,
        DscDag::NodeToken in_ui_scale,
        DscDag::NodeToken in_avaliable_size
        )
    {
        DscUi::UiComponentResourceNodeGroup component_resource_group;
        component_resource_group.SetNodeToken(
            DscUi::TUiComponentResourceNodeGroup::TClearColour,
            in_dag_collection.CreateValue(
                in_construction_helper._clear_colour,
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                &component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "clear colour")));

        if (true == in_construction_helper._has_scroll)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::THasManualScrollX,
                in_dag_collection.CreateValue(
                    in_construction_helper._has_manual_scroll_x,
                    DscDag::CallbackOnValueChange<bool>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "has manual scroll x")));

            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TManualScrollX,
                in_dag_collection.CreateValue(
                    0.0f,
                    DscDag::CallbackOnValueChange<float>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "manual scroll x")));

            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::THasManualScrollY,
                in_dag_collection.CreateValue(
                    in_construction_helper._has_manual_scroll_y,
                    DscDag::CallbackOnValueChange<bool>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "has manual scroll y")));

            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TManualScrollY,
                in_dag_collection.CreateValue(
                    0.0f,
                    DscDag::CallbackOnValueChange<float>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "manual scroll y")));
        }

        if (true == in_construction_helper._has_fill)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TFillColour,
                in_dag_collection.CreateValue(
                    in_construction_helper._fill,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "fill colour")));
        }

        if (nullptr != in_construction_helper._texture)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TTexture,
                in_dag_collection.CreateValue(
                    in_construction_helper._texture,
                    DscDag::CallbackOnSetValue<std::shared_ptr<DscRenderResource::ShaderResource>>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "texture")));
        }

        if (nullptr != in_construction_helper._text_run)
        {
            DSC_ASSERT(nullptr != in_construction_helper._text_manager, "invalid state");
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TText,
                in_dag_collection.CreateValue(
                    DscUi::TUiComponentTextData({ in_construction_helper._text_run , in_construction_helper._text_manager }),
                    DscDag::CallbackOnSetValue<DscUi::TUiComponentTextData>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "text")));
        }

        if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
        {
            const int32 scale_width_low_threashhold = in_construction_helper._scale_width_low_threashhold;
            const float scale_factor = in_construction_helper._scale_factor;
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TUiScale,
                in_dag_collection.CreateCalculate<float>([scale_width_low_threashhold, scale_factor](float& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
                    const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
                    if (scale_width_low_threashhold < avaliable_size.GetX())
                    {
                        ui_scale = (1.0f + (static_cast<float>(avaliable_size.GetX() - scale_width_low_threashhold) * scale_factor));
                    }

                    value = ui_scale;
                },
                & component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "ui scale from width")));
            DscDag::DagCollection::LinkIndexNodes(0, in_ui_scale, component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale));
            DscDag::DagCollection::LinkIndexNodes(1, in_avaliable_size, component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale));
        }
        else
        {
            component_resource_group.SetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale, in_ui_scale);
        }

        if (true == in_construction_helper._has_child_slot_data)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TChildSlotSize,
                in_dag_collection.CreateValue(
                    in_construction_helper._child_size,
                    DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "child slot size")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot,
                in_dag_collection.CreateValue(
                    in_construction_helper._child_pivot,
                    DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "child slot pivot")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach,
                in_dag_collection.CreateValue(
                    in_construction_helper._attach_point,
                    DscDag::CallbackOnValueChange<DscUi::VectorUiCoord2>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "child slot parent attach")));
        }

        if (true == in_construction_helper._has_padding)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TPaddingLeft,
                in_dag_collection.CreateValue(
                    in_construction_helper._padding_left,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "padding left")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TPaddingTop,
                in_dag_collection.CreateValue(
                    in_construction_helper._padding_top,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "padding top")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TPaddingRight,
                in_dag_collection.CreateValue(
                    in_construction_helper._padding_right,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "padding right")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TPaddingBottom,
                in_dag_collection.CreateValue(
                    in_construction_helper._padding_bottom,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "padding bottom")));
        }

        if (true == in_construction_helper._has_gap)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TGap,
                in_dag_collection.CreateValue(
                    in_construction_helper._gap,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "gap")));
        }

        if (DscUi::TUiFlow::TCount != in_construction_helper._flow_direction)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TFlow,
                in_dag_collection.CreateValue(
                    in_construction_helper._flow_direction,
                    DscDag::CallbackOnValueChange<DscUi::TUiFlow>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "flow direction")));
        }

        if (true == in_construction_helper._has_child_stack_data)
        {
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TChildStackSize,
                in_dag_collection.CreateValue(
                    in_construction_helper._stack_size,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "child stack size")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TChildStackPivot,
                in_dag_collection.CreateValue(
                    in_construction_helper._stack_pivot,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "child stack size")));
            component_resource_group.SetNodeToken(
                DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach,
                in_dag_collection.CreateValue(
                    in_construction_helper._stack_parent_attach_point,
                    DscDag::CallbackOnValueChange<DscUi::UiCoord>::Function,
                    &component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "child stack size")));
        }

        return component_resource_group;
    }

    DscDag::NodeToken MakeAvaliableSize(
        DscDag::DagCollection& in_dag_collection,
        DscDag::NodeToken in_parent_avaliable_size,
        DscDag::NodeToken in_ui_scale,
        const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
        const DscUi::UiComponentResourceNodeGroup& in_parent_component_resource_group,
        DscUi::UiNodeGroup& in_owner_group
    )
    {
        DscDag::NodeToken node = in_parent_avaliable_size;
        if (nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow)) // slot child
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                    const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                    const DscUi::TUiFlow flow = DscDag::DagCollection::GetValueType<DscUi::TUiFlow>(in_input_array[2]);
                    DscCommon::VectorInt2 result = {};

                    if (3 < in_input_array.size())
                    {
                        const DscUi::UiCoord& child_size = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                        switch(flow)
                        {
                        default:
                            DSC_ASSERT_ALWAYS("invalid switch case");
                            break;
                        case DscUi::TUiFlow::THorizontal:
                            result[1] = child_size.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);
                            break;
                        case DscUi::TUiFlow::TVertical:
                            result[0] = child_size.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                            break;
                        }
                    }
                    else
                    {
                        switch (flow)
                        {
                        default:
                            DSC_ASSERT_ALWAYS("invalid switch case");
                            break;
                        case DscUi::TUiFlow::THorizontal:
                            result[1] = parent_avaliable_size.GetY();
                            break;
                        case DscUi::TUiFlow::TVertical:
                            result[0] = parent_avaliable_size.GetX();
                            break;
                        }
                    }

                    value = result;
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "avaliable size child stack"));

            DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
            DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
            DscDag::DagCollection::LinkIndexNodes(2, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow), node);
            if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackSize))
            {
                DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackSize), node);
            }
        }
        else if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize)) // canvas child
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                    const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                    const DscUi::VectorUiCoord2& child_slot_size = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[2]);

                    value = child_slot_size.EvalueUICoord(parent_avaliable_size, ui_scale);
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "avaliable size child slot"));

            DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
            DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
            DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize), node);
        }
        else if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)) // padding child
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                    const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                    const DscUi::UiCoord& left = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[2]);
                    const DscUi::UiCoord& top = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                    const DscUi::UiCoord& right = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[4]);
                    const DscUi::UiCoord& bottom = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[5]);

                    const int32 width_taken_by_padding = left.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale) +
                        right.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                    const int32 height_taken_by_padding = top.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale) +
                        bottom.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);

                    value.Set(
                        parent_avaliable_size.GetX() - width_taken_by_padding,
                        parent_avaliable_size.GetY() - height_taken_by_padding
                    );
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "avaliable size padding"));

            DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
            DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
            DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft), node);
            DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingTop), node);
            DscDag::DagCollection::LinkIndexNodes(4, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingRight), node);
            DscDag::DagCollection::LinkIndexNodes(5, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingBottom), node);
        }

        return node;
    }

    DscDag::NodeToken MakeDesiredSize(
        const DscUi::TUiComponentType in_component_type,
        const bool in_desired_size_from_children_max,
        DscDag::DagCollection & in_dag_collection,
        DscDag::NodeToken in_ui_scale,
        DscDag::NodeToken in_avaliable_size,
        DscDag::NodeToken in_array_child_node_group,
        const DscUi::UiComponentResourceNodeGroup& in_resource_node_group,
        DscUi::UiNodeGroup & in_owner_group
    )
    {
        DSC_UNUSED(in_component_type);
        DSC_UNUSED(in_resource_node_group);
        DSC_UNUSED(in_ui_scale);
        DscDag::NodeToken node = in_avaliable_size;

        if (true == in_desired_size_from_children_max)
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const std::vector<DscUi::UiNodeGroup>& array_child_node_group = DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_input_array[0]);
                    DscCommon::VectorInt2 max_size = {};
                    for (const auto& item : array_child_node_group)
                    {
                        const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType< DscCommon::VectorInt2>(item.GetNodeToken(DscUi::TUiNodeGroup::TGeometryOffset));
                        const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType< DscCommon::VectorInt2>(item.GetNodeToken(DscUi::TUiNodeGroup::TGeometrySize));

                        max_size.Set(
                            std::max(max_size.GetX(), geometry_offset.GetX() + geometry_size.GetX()),
                            std::max(max_size.GetY(), geometry_offset.GetY() + geometry_size.GetY())
                        );
                    }

                    value = max_size;
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "desired size max children"));

            DscDag::DagCollection::LinkIndexNodes(0, in_array_child_node_group, node);
        }
        else if (nullptr != in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TText))        //if text, get text bounds size
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                    const DscUi::TUiComponentTextData& text_data = DscDag::DagCollection::GetValueType<DscUi::TUiComponentTextData>(in_input_array[1]);
                    const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[2]);

                    DscText::TextRun* const text_run_raw = text_data._text_run.get();
                    if (nullptr != text_run_raw)
                    {
                        text_run_raw->SetWidthLimit(
                            text_run_raw->GetWidthLimitEnabled(),
                            avaliable_size.GetX()
                        );
                        text_run_raw->SetUIScale(ui_scale);

                        value = text_run_raw->GetTextBounds();
                    }
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "desired size text"));

            DscDag::DagCollection::LinkIndexNodes(0, in_avaliable_size, node);
            DscDag::DagCollection::LinkIndexNodes(1, in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TText), node);
            DscDag::DagCollection::LinkIndexNodes(2, in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale), node);
        }

        return node;
    }

    DscDag::NodeToken MakeRenderRequestSize(
        DscDag::DagCollection& in_dag_collection,
        DscDag::NodeToken in_desired_size,
        DscDag::NodeToken in_geometry_size_size,
        DscUi::UiNodeGroup& in_owner_group
        )
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscCommon::VectorInt2& desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

                value.Set(
                    std::max(desired_size.GetX(), geometry_size.GetX()),
                    std::max(desired_size.GetY(), geometry_size.GetY())
                    );
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "avaliable size child slot"));

        DscDag::DagCollection::LinkIndexNodes(0, in_desired_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_geometry_size_size, node);
        return node;
    }

    DscDag::NodeToken MakeGeometrySize(
        DscDag::DagCollection& in_dag_collection,
        const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
        const DscUi::UiComponentResourceNodeGroup& in_parent_component_resource_group,
        DscDag::NodeToken in_desired_size,
        DscDag::NodeToken in_avaliable_size,
        DscUi::UiNodeGroup& in_owner_group
    )
    {
        DSC_UNUSED(in_dag_collection);
        DSC_UNUSED(in_owner_group);
        DscDag::NodeToken node = in_desired_size;
        if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize))
        {
            // if we have a child slot, presume we are behaving like a canvas that has the geometry size the same as the avaliable size
            node = in_avaliable_size;
        }
        else if (nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow))
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscUi::TUiFlow flow = DscDag::DagCollection::GetValueType<DscUi::TUiFlow>(in_input_array[0]);
                    const DscCommon::VectorInt2& desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
                    const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

                    switch (flow)
                    {
                    default:
                        value = {}; //.Set(0, 0);
                        DSC_ASSERT_ALWAYS("invalid switch case");
                        break;
                    case DscUi::TUiFlow::THorizontal:
                        value.Set(
                            desired_size.GetX(),
                            avaliable_size.GetY()
                        );
                    case DscUi::TUiFlow::TVertical:
                        value.Set(
                            avaliable_size.GetX(),
                            desired_size.GetY()
                        );
                    }
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "geometry size flow child"));

            DscDag::DagCollection::LinkIndexNodes(0, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow), node);
            DscDag::DagCollection::LinkIndexNodes(1, in_desired_size, node);
            DscDag::DagCollection::LinkIndexNodes(2, in_avaliable_size, node);

        }

        DSC_ASSERT(nullptr != node, "invalid state");
        return node;
    }

    DscDag::NodeToken MakeGeometryOffset(
        DscDag::DagCollection& in_dag_collection,
        const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
        const DscUi::UiComponentResourceNodeGroup& in_parent_component_resource_group,
        DscDag::NodeToken in_parent_avaliable_size,
        DscDag::NodeToken in_ui_scale,
        DscDag::NodeToken in_geometry_size,
        DscDag::NodeToken in_parent_array_child_ui_node_group,
        DscUi::UiNodeGroup& in_owner_group
    )
    {
        DSC_UNUSED(in_parent_array_child_ui_node_group);

        DscDag::NodeToken node = nullptr;
        if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize))
        {
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                    const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

                    const DscUi::VectorUiCoord2& child_slot_pivot = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[3]);
                    const DscUi::VectorUiCoord2& child_slot_parent_attach = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);

                    const DscCommon::VectorInt2 pivot_point = child_slot_pivot.EvalueUICoord(geometry_size, ui_scale);
                    const DscCommon::VectorInt2 attach_point = child_slot_parent_attach.EvalueUICoord(parent_avaliable_size, ui_scale);

                    value.Set(
                        attach_point.GetX() - pivot_point.GetX(),
                        attach_point.GetY() - pivot_point.GetY()
                    );
                },
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "geometry offset child slot"));

            DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
            DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
            DscDag::DagCollection::LinkIndexNodes(2, in_geometry_size, node); // should this be avaliable size or geometry size
            DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot), node);
            DscDag::DagCollection::LinkIndexNodes(4, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach), node);
        }
        else if (nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow))
        {
            // this is a bit bad, we don't have an easy way of knowing what array child index we are of our parent (in a fashion resiliant to array size change) 
            // we could use the geoemtry size node as a token to identify ourself since we have that node on hand...
            //TUiNodeGroup::TGeometrySize
            node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                    const DscDag::NodeToken self_geometry_size = in_input_array[0];
                    const std::vector<DscUi::UiNodeGroup>& parent_child_array = DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_input_array[1]);
                    const DscUi::UiCoord& gap = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[2]);
                    const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
                    const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[4]);
                    const DscUi::TUiFlow flow = DscDag::DagCollection::GetValueType<DscUi::TUiFlow>(in_input_array[5]);

                    value = {};
                    int32 trace = 0;
                    for (const auto& item : parent_child_array)
                    {
                        if (self_geometry_size == item.GetNodeToken(DscUi::TUiNodeGroup::TGeometrySize))
                        {
                            break;
                        }
                        const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(item.GetNodeToken(DscUi::TUiNodeGroup::TGeometrySize));
                        switch (flow)
                        {
                        default:
                            break;
                        case DscUi::TUiFlow::THorizontal:
                            trace += geometry_size.GetX();
                            trace += gap.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                            break;
                        case DscUi::TUiFlow::TVertical:
                            trace += geometry_size.GetY();
                            trace += gap.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);
                            break;
                        }
                    }

                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(self_geometry_size);
                    switch (flow)
                    {
                    default:
                        break;
                    case DscUi::TUiFlow::THorizontal:
                    {
                        int32 offset = 0;
                        if (7 < in_input_array.size())
                        { 
                            const DscUi::UiCoord& pivot = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[6]);
                            const DscUi::UiCoord& parent_attach = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[7]);
                            offset = parent_attach.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale) -
                                pivot.Evaluate(geometry_size.GetY(), geometry_size.GetX(), ui_scale);
                        }
                        value.Set(trace, offset);
                    }
                    break;
                    case DscUi::TUiFlow::TVertical:
                    {
                        int32 offset = 0;
                        if (7 < in_input_array.size())
                        {
                            const DscUi::UiCoord& pivot = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[6]);
                            const DscUi::UiCoord& parent_attach = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[7]);
                            offset = parent_attach.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale) -
                                pivot.Evaluate(geometry_size.GetX(), geometry_size.GetY(), ui_scale);
                        }
                        value.Set(offset, trace);
                    }
                    break;
                    }

                    return;
                },
                & in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "geometry offset child stack"));
            DscDag::DagCollection::LinkIndexNodes(0, in_geometry_size, node);
            DscDag::DagCollection::LinkIndexNodes(1, in_parent_array_child_ui_node_group, node);
            DSC_ASSERT(nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TGap), "parent stack is obliged to have a gap node");
            DscDag::DagCollection::LinkIndexNodes(2, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TGap), node);
            DscDag::DagCollection::LinkIndexNodes(3, in_parent_avaliable_size, node);
            DscDag::DagCollection::LinkIndexNodes(4, in_ui_scale, node);
            DscDag::DagCollection::LinkIndexNodes(5, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow), node);
            if ((nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackPivot)) &&
                (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach)))
            {
                DscDag::DagCollection::LinkIndexNodes(6, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackPivot), node);
                DscDag::DagCollection::LinkIndexNodes(7, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach), node);
            }
        }
        else
        {
            node = in_dag_collection.CreateValue<DscCommon::VectorInt2>(
                DscCommon::VectorInt2::s_zero,
                DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function,
                &in_owner_group
                DSC_DEBUG_ONLY(DSC_COMMA "geometry offset zero"));
        }

        return node;
    }

    DscDag::NodeToken MakeNodePixelTraversal(
        DscDag::DagCollection& in_dag_collection, 
        DscDag::NodeToken in_geometry_size, 
        DscDag::NodeToken in_render_request_size,
        DscUi::UiNodeGroup& in_owner_group
        )
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                DscCommon::VectorInt2 render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

                value.Set(
                    std::max(0, render_size.GetX() - geometry_size.GetX()),
                    std::max(0, render_size.GetY() - geometry_size.GetY())
                );
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "pixel traversal"));

        DscDag::DagCollection::LinkIndexNodes(0, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_render_request_size, node);

        return node;
    }

    /// note: the automatic scroll, to do pingpoing without state, traverses a range of [-1 ... 1] and is passed through an abs() function
    /// otherwise would need state of going up or going down and swap at [0, 1]
    /// this works, but manual scroll needs to clamp it's range [0 ... 1] else in negative domain, will be pingpong to positive....
    constexpr float s_scroll_pixels_per_second = 32.0f;
    constexpr float s_wrap_threashold = 1.0f; // was considering 1.25f for threashold and 2.5 for ping pong step, but only pauses at one end of the anim
    constexpr float s_wrap_step_ping_pong = 2.0f;
    DscDag::NodeToken MakeNodeScrollValue(
        DscDag::DagCollection& in_dag_collection,
        const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
        DscDag::NodeToken in_time_delta,
        DscDag::NodeToken in_pixel_traversal_node,
        DscUi::UiNodeGroup& in_owner_group
    )
    {
        DscDag::NodeToken condition_x = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                // if manual scroll is false and the 0 < pixel traversal, return true
                const bool has_manual_scroll = DscDag::DagCollection::GetValueType<bool>(in_input_array[0]);
                const DscCommon::VectorInt2& pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

                out_value = ((false == has_manual_scroll) && (0 < pixel_traversal.GetX()));
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll condition x"));
        DscDag::DagCollection::LinkIndexNodes(0, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::THasManualScrollX), condition_x);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, condition_x);

        DscDag::NodeToken condition_y = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                // if manual scroll is false and the 0 < pixel traversal, return true
                const bool has_manual_scroll = DscDag::DagCollection::GetValueType<bool>(in_input_array[0]);
                const DscCommon::VectorInt2& pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

                out_value = ((false == has_manual_scroll) && (0 < pixel_traversal.GetY()));
            },
            & in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll condition y"));
        DscDag::DagCollection::LinkIndexNodes(0, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::THasManualScrollY), condition_y);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, condition_y);

        DscDag::NodeToken tick_scroll_x = in_dag_collection.CreateCalculate<float>([](float& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const float time_delta = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
                const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
                const int32 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetX();

                if (0 < pixel_traversal)
                {
                    // the max of the divisor is to avoid things bouncing too quickly on small values of pixel_traversal
                    out_value += time_delta_clamped * s_scroll_pixels_per_second / std::max(s_scroll_pixels_per_second, static_cast<float>(pixel_traversal));
                    while (s_wrap_threashold < out_value)
                    {
                        out_value -= s_wrap_step_ping_pong; // pingpong, consumer of the value applies std::abs
                    }
                }
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll condition x"));
        DscDag::DagCollection::LinkIndexNodes(0, in_time_delta, tick_scroll_x);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, tick_scroll_x);

        DscDag::NodeToken tick_scroll_y = in_dag_collection.CreateCalculate<float>([](float& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const float time_delta = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
                const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
                const int32 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetY();

                if (0 < pixel_traversal)
                {
                    // the max of the divisor is to avoid things bouncing too quickly on small values of pixel_traversal
                    out_value += time_delta_clamped * s_scroll_pixels_per_second / std::max(s_scroll_pixels_per_second, static_cast<float>(pixel_traversal));
                    while (s_wrap_threashold < out_value)
                    {
                        out_value -= s_wrap_step_ping_pong; // pingpong, consumer of the value applies std::abs
                    }
                }
            },
            & in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll condition y"));
        DscDag::DagCollection::LinkIndexNodes(0, in_time_delta, tick_scroll_y);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, tick_scroll_y);

        // something to hold the scroll value output
        DscDag::NodeToken scroll_x = in_dag_collection.CreateValue(
            0.0f, 
            DscDag::CallbackOnValueChange<float>::Function,
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll x"));
        DscDag::NodeToken scroll_y = in_dag_collection.CreateValue(
            0.0f,
            DscDag::CallbackOnValueChange<float>::Function,
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll y"));

        in_dag_collection.CreateCondition(
            condition_x,
            tick_scroll_x, 
            in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TManualScrollX),
            scroll_x, 
            scroll_x,
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "conditional scroll x")
        );
        in_dag_collection.CreateCondition(
            condition_y,
            tick_scroll_y,
            in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TManualScrollY),
            scroll_y,
            scroll_y,
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "conditional scroll y")
        );

        DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscCommon::VectorFloat2>([](DscCommon::VectorFloat2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const float x = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
                const float y = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);

                value.Set(x, y);
            },
            &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll"));
        DscDag::DagCollection::LinkIndexNodes(0, scroll_x, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, scroll_y, result_node);

        return result_node;
    }

    DscDag::NodeToken MakeScreenSpace(
        DscDag::DagCollection& in_dag_collection,
        DscDag::NodeToken in_parent_screen_space,
        DscDag::NodeToken in_parent_render_request_size,
        DscDag::NodeToken in_geometry_size,
        DscDag::NodeToken in_geometry_offset,
        DscDag::NodeToken in_render_request_size,
        DscDag::NodeToken in_scroll,
        DscUi::UiNodeGroup& in_owner_group
    )
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscUi::ScreenSpace>([](DscUi::ScreenSpace& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscUi::ScreenSpace& parent_screen_space = DscDag::DagCollection::GetValueType<DscUi::ScreenSpace>(in_input_array[0]);
                const DscCommon::VectorInt2& parent_render_request_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
                const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);
                const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
                const DscCommon::VectorInt2& render_request_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[4]);
                const DscCommon::VectorFloat2& scroll = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(in_input_array[5]);

                const DscCommon::VectorFloat2 parent_size(
                    parent_screen_space._screen_space[2] - parent_screen_space._screen_space[0],
                    parent_screen_space._screen_space[3] - parent_screen_space._screen_space[1]
                    );
                const DscCommon::VectorFloat2 parent_scale(
                    parent_size.GetX() / static_cast<float>(parent_render_request_size.GetX()),
                    parent_size.GetY() / static_cast<float>(parent_render_request_size.GetY())
                );

                const DscCommon::VectorFloat2 geometry_pivot(
                    static_cast<float>(geometry_offset.GetX()) - (static_cast<float>(render_request_size.GetX() - geometry_size.GetX()) * std::abs(scroll.GetX())),
                    static_cast<float>(geometry_offset.GetY()) - (static_cast<float>(render_request_size.GetY() - geometry_size.GetY()) * std::abs(scroll.GetY()))
                    );

                value._screen_space.Set(
                    parent_screen_space._screen_space[0] + (geometry_pivot[0] * parent_scale[0]),
                    parent_screen_space._screen_space[1] + (geometry_pivot[1] * parent_scale[1]),
                    parent_screen_space._screen_space[0] + ((geometry_pivot[0] + static_cast<float>(render_request_size.GetX())) * parent_scale[0]),
                    parent_screen_space._screen_space[1] + ((geometry_pivot[1] + static_cast<float>(render_request_size.GetY())) * parent_scale[1])
                    );

            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "pixel traversal"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_screen_space, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_render_request_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_geometry_offset, node);
        DscDag::DagCollection::LinkIndexNodes(3, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(4, in_render_request_size, node);
        DscDag::DagCollection::LinkIndexNodes(5, in_scroll, node);

        return node;
    }

    void TraverseHierarchyInput(
        const DscUi::UiInputParam::TouchData& in_touch,
        DscDag::NodeToken in_screen_space,
        DscDag::NodeToken in_array_child_ui_node_group,
        DscDag::NodeToken in_component_resources
    )
    {
        const float x = static_cast<float>(in_touch._root_relative_pos.GetX());
        const float y = static_cast<float>(in_touch._root_relative_pos.GetY());
        const DscUi::ScreenSpace& screen_space = DscDag::DagCollection::GetValueType<DscUi::ScreenSpace>(in_screen_space);

        // bounds check
        bool outside = false;
        if ((x < screen_space._screen_space[0]) || (screen_space._screen_space[2] < x))
        {
            outside = true;
        }
        if ((y < screen_space._screen_space[1]) || (screen_space._screen_space[3] < y))
        {
            outside = true;
        }

        const DscUi::UiComponentResourceNodeGroup& resource_group = DscDag::DagCollection::GetValueType< DscUi::UiComponentResourceNodeGroup>(in_component_resources);
        if (nullptr != resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TEffectParamArray))
        {
            const std::vector<DscDag::NodeToken>& effect_param_array = DscDag::DagCollection::GetValueType<std::vector<DscDag::NodeToken>>(resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TEffectParamArray));
            int32 trace = 0;
            for (const auto& node : effect_param_array)
            {
                if (0 != (trace & 1))
                {
                    DscDag::DagCollection::SetValueType(node, DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, outside ? 0.5f : 1.0f));
                }
                trace += 1;
            }
        }

        const auto& array_children = DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_array_child_ui_node_group);
        for (const auto& child : array_children)
        {
            TraverseHierarchyInput(
                in_touch,
                child.GetNodeToken(DscUi::TUiNodeGroup::TScreenSpace),
                child.GetNodeToken(DscUi::TUiNodeGroup::TArrayChildUiNodeGroup),
                child.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources)
                );
        }
    }


} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection)
{
    _dag_resource = DscDagRender::DagResource::Factory(&in_draw_system, &in_dag_collection);
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

    //_full_quad_pos_uv
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

        _full_quad_pos_uv = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            ScreenQuad::GetInputElementDesc(),
            vertex_raw_data,
            4
            );
    }

    //_full_quad_pos
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);

        _full_quad_pos = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            s_input_element_desc_array,
            vertex_raw_data,
            2
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
                TDebugGridConstantBuffer(),
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

    // _fill_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "fill_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "fill_ps.cso")))
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
        _fill_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );
    }

    _effect_round_corner_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_round_corner_vs.cso",
        "effect_round_corner_ps.cso",
        true
        );

    _effect_drop_shadow_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_drop_shadow_vs.cso",
        "effect_drop_shadow_ps.cso",
        false
    );

    _effect_inner_shadow_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_inner_shadow_vs.cso",
        "effect_inner_shadow_ps.cso",
        false
    );

    _effect_stroke_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_stroke_vs.cso",
        "effect_stroke_ps.cso",
        true
    );

    _effect_tint_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_tint_vs.cso",
        "effect_tint_ps.cso",
        true
    );
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

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentDebugGrid()
{
    return TComponentConstructionHelper({ TUiComponentType::TDebugGrid});
}

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentFill(const DscCommon::VectorFloat4& in_colour)
{
    TComponentConstructionHelper result({ TUiComponentType::TFill });
    result._fill = in_colour;
    result._has_fill = true;
    return result;
}

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentImage(const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture)
{
    TComponentConstructionHelper result({ TUiComponentType::TImage });
    result._texture = in_texture;
    return result;
}

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentCanvas()
{
    TComponentConstructionHelper result({ TUiComponentType::TCanvas });
    return result;
}

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentText(
    const std::shared_ptr<DscText::TextRun>& in_text_run,
    DscText::TextManager* const in_text_manager,
    const bool in_has_scroll
)
{
    TComponentConstructionHelper result({ TUiComponentType::TText });
    result._text_run = in_text_run;
    result._text_manager = in_text_manager;
    result._has_scroll = in_has_scroll;
    return result;
}

DscUi::UiManager::TComponentConstructionHelper DscUi::UiManager::MakeComponentStack(
    const TUiFlow in_flow_direction,
    const UiCoord& in_gap,
    const bool in_desired_size_from_children_max,
    const bool in_has_scroll
)
{
    TComponentConstructionHelper result({ TUiComponentType::TStack });
    result._flow_direction = in_flow_direction;
    result._has_gap = true;
    result._gap = in_gap;
    result._desired_size_from_children_max = in_desired_size_from_children_max;
    result._has_scroll = in_has_scroll;
    return result;
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
        DscDag::CallbackNever<DscRenderResource::Frame*>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "frame")));

    result.SetNodeToken(TUiRootNodeGroup::TTimeDelta, in_dag_collection.CreateValue<float>(
        0.0f,
        DscDag::CallbackNoZero<float>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "time delta")));

    result.SetNodeToken(TUiRootNodeGroup::TUiScale, in_dag_collection.CreateValue(
        1.0f,
        DscDag::CallbackOnValueChange<float>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui scale")));

    result.SetNodeToken(TUiRootNodeGroup::TInputState, in_dag_collection.CreateValue(
        UiInputState(),
        DscDag::CallbackOnSetValue<UiInputState>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "input state")));

    result.SetNodeToken(TUiRootNodeGroup::TUiComponentType, in_dag_collection.CreateValue(
        in_construction_helper._component_type,
        DscDag::CallbackOnValueChange<TUiComponentType>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui component type")));

    result.SetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue<std::vector<UiNodeGroup>>(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    result.SetNodeToken(TUiRootNodeGroup::TForceDraw, in_dag_collection.CreateValue<bool>(
        false,
        DscDag::CallbackNoZero<bool>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "force draw")));

    result.SetNodeToken(TUiRootNodeGroup::TUiRenderTarget, in_dag_collection.CreateValue(
        in_ui_render_target,
        DscDag::CallbackOnSetValue<std::shared_ptr<DscUi::UiRenderTarget>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui render target")));

    result.SetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize, in_dag_collection.CreateValue(
        DscCommon::VectorInt2::s_zero,
        DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "render target size")));

    result.SetNodeToken(TUiRootNodeGroup::TScreenSpace, in_dag_collection.CreateValue(
        DscUi::ScreenSpace(),
        DscDag::CallbackOnValueChange<DscUi::ScreenSpace>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "screen space")));

    UiComponentResourceNodeGroup component_resource_node_group;
    component_resource_node_group.SetNodeToken(TUiComponentResourceNodeGroup::TClearColour, in_dag_collection.CreateValue(
        in_construction_helper._clear_colour,
        DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "clear colour")));
    component_resource_node_group.SetNodeToken(TUiComponentResourceNodeGroup::TUiScale, result.GetNodeToken(TUiRootNodeGroup::TUiScale));

    auto draw_node = MakeDrawStack(
        in_construction_helper,
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        result,
        result.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget),
        result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize),
        result.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group
        DSC_DEBUG_ONLY(DSC_COMMA "root draw")
    );
    // if force draw is true, we just need to re apply the last draw step, even if nothing else has changed
    DscDag::DagCollection::LinkNodes(result.GetNodeToken(TUiRootNodeGroup::TForceDraw), draw_node);
    DscDag::DagCollection::LinkNodes(result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize), draw_node);

    result.SetNodeToken(TUiRootNodeGroup::TDrawNode, draw_node);

    component_resource_node_group.Validate();

    result.SetNodeToken(TUiRootNodeGroup::TUiComponentResources, in_dag_collection.CreateValue<DscUi::UiComponentResourceNodeGroup>(
        component_resource_node_group,
        DscDag::CallbackNever<DscUi::UiComponentResourceNodeGroup>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "component resource node group")));

    result.Validate();

    return result;
}

DscUi::UiNodeGroup DscUi::UiManager::ConvertRootNodeGroupToNodeGroup(
    DscDag::DagCollection& in_dag_collection,
    const UiRootNodeGroup& in_ui_root_node_group
)
{
    UiNodeGroup result;

    result.SetNodeToken(TUiNodeGroup::TDrawNode, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TDrawNode));
    result.SetNodeToken(TUiNodeGroup::TUiComponentType, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentType));
    result.SetNodeToken(TUiNodeGroup::TUiComponentResources, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources));
    result.SetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup));
    result.SetNodeToken(TUiNodeGroup::TAvaliableSize, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(TUiNodeGroup::TRenderRequestSize, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(TUiNodeGroup::TScreenSpace, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TScreenSpace));
    result.SetNodeToken(TUiNodeGroup::TGeometrySize, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(TUiNodeGroup::TGeometryOffset, 
        in_dag_collection.CreateValue(
            DscCommon::VectorInt2::s_zero,
            DscDag::CallbackNever<DscCommon::VectorInt2>::Function,
            &result
            DSC_DEBUG_ONLY(DSC_COMMA "geometry offset")));
    result.SetNodeToken(TUiNodeGroup::TScrollPos,
        in_dag_collection.CreateValue(
            DscCommon::VectorFloat2::s_zero,
            DscDag::CallbackNever<DscCommon::VectorFloat2>::Function,
            &result
            DSC_DEBUG_ONLY(DSC_COMMA "scroll pos")));

    result.Validate();

    return result;
}

DscUi::UiNodeGroup DscUi::UiManager::AddChildNode(
    const TComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const UiRootNodeGroup& in_root_node_group,
    const UiNodeGroup& in_parent,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
    )
{
    UiNodeGroup result;

    //TUiComponentResources
    result.SetNodeToken(TUiNodeGroup::TUiComponentResources, in_dag_collection.CreateValue<UiComponentResourceNodeGroup>(
        MakeComponentResourceGroup(
            in_dag_collection, 
            in_construction_helper,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize) // ok, this was ment to be the local avaliable size, but have some dependency order issues here
            ),
        DscDag::CallbackNever<DscUi::UiComponentResourceNodeGroup>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "component resource node group")));

    //TUiComponentType
    result.SetNodeToken(TUiNodeGroup::TUiComponentType, in_dag_collection.CreateValue(
        in_construction_helper._component_type,
        DscDag::CallbackOnValueChange<TUiComponentType>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui component type")));

    // TArrayChildUiNodeGroup
    result.SetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    //TUiPanelShaderConstantBuffer
    {
        auto panel_shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        result.SetNodeToken(TUiNodeGroup::TUiPanelShaderConstantBuffer, in_dag_collection.CreateValue(
            panel_shader_constant_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &result
            DSC_DEBUG_ONLY(DSC_COMMA "ui panel shader constant buffer")));
    }

    //TArrayChildUiNodeGroup
    result.SetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue<std::vector<UiNodeGroup>>(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    //calculate our avaliable size
    result.SetNodeToken(
        TUiNodeGroup::TAvaliableSize, 
        MakeAvaliableSize(
            in_dag_collection,
            in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize),
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(in_parent.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            result
        ));

    // more of a order of construction issue than a circular dependency
    if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
    {
        UiComponentResourceNodeGroup& component_resource = DscDag::DagCollection::GetValueNonConstRef<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources), false);
        DscDag::DagCollection::LinkIndexNodes(1, result.GetNodeToken(TUiNodeGroup::TAvaliableSize), component_resource.GetNodeToken(TUiComponentResourceNodeGroup::TUiScale));
    }

    // calculate our desired size (for stack, this is all the contents, for text, the text render size (if width limit, limit is the avaliable size width))
    DscDag::NodeToken desired_size = MakeDesiredSize(
        in_construction_helper._component_type,
        in_construction_helper._desired_size_from_children_max,
        in_dag_collection,
        in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
        result.GetNodeToken(TUiNodeGroup::TAvaliableSize),
        result.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
        result
        );

    //TGeometrySize
    result.SetNodeToken(TUiNodeGroup::TGeometrySize,
        MakeGeometrySize(
            in_dag_collection,
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(in_parent.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            desired_size,
            result.GetNodeToken(TUiNodeGroup::TAvaliableSize),
            result
        ));

    //TGeometryOffset (after TGeometrySize as canvas child uses geometry size for attach point)
    result.SetNodeToken(TUiNodeGroup::TGeometryOffset, 
        MakeGeometryOffset(
            in_dag_collection,
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(in_parent.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize),
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            result.GetNodeToken(TUiNodeGroup::TGeometrySize),
            in_parent.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup),
            result
        ));
        
    //TRenderRequestSize calculate our render request size (max desired and geometry size)
    result.SetNodeToken(TUiNodeGroup::TRenderRequestSize,
        MakeRenderRequestSize(
            in_dag_collection,
            desired_size,
            result.GetNodeToken(TUiNodeGroup::TGeometrySize),
            result
        ));

    //TScrollPos, // where is the geometry size quad is on the render target texture
    {
        const auto& resource_group = DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources));
        if ((nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::THasManualScrollX)) &&
            (nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::THasManualScrollY)) &&
            (nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TManualScrollX)) &&
            (nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TManualScrollY)))
        {
            DscDag::NodeToken pixel_traversal_node = MakeNodePixelTraversal(
                in_dag_collection,
                result.GetNodeToken(TUiNodeGroup::TGeometrySize),
                result.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
                result
            );

            result.SetNodeToken(TUiNodeGroup::TScrollPos,
                MakeNodeScrollValue(
                    in_dag_collection,
                    DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
                    in_root_node_group.GetNodeToken(TUiRootNodeGroup::TTimeDelta),
                    pixel_traversal_node,
                    result
                ));
        }
        else
        {
            result.SetNodeToken(TUiNodeGroup::TScrollPos, in_dag_collection.CreateValue(
                DscCommon::VectorFloat2(0, 0),
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat2>::Function,
                &result
                DSC_DEBUG_ONLY(DSC_COMMA "scroll pos")));
        }
    }

    //TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover
    result.SetNodeToken(TUiNodeGroup::TScreenSpace, MakeScreenSpace(
        in_dag_collection,
        in_parent.GetNodeToken(TUiNodeGroup::TScreenSpace),
        in_parent.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
        result.GetNodeToken(TUiNodeGroup::TGeometrySize),
        result.GetNodeToken(TUiNodeGroup::TGeometryOffset),
        result.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
        result.GetNodeToken(TUiNodeGroup::TScrollPos),
        result
        ));

    UiComponentResourceNodeGroup& component_resource_node_group = DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources), false);
    auto draw_node = MakeDrawStack(
        in_construction_helper, //TUiDrawType
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        in_root_node_group,
        nullptr,
        result.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
        result.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name + " draw")
    );
    result.SetNodeToken(TUiNodeGroup::TDrawNode, draw_node);

    result.Validate();

    // and now attach to parent
    {
        DscDag::ArrayHelper::PushBack(in_parent.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), result);

        // effect the child Update function to be called just before the parent Update/draw by setting our draw as input of the parent draw
        DscDag::DagCollection::LinkNodes(
            draw_node,
            in_parent.GetNodeToken(TUiNodeGroup::TDrawNode)
            );
    }

    return result;
}

void DscUi::UiManager::Update(
    const UiRootNodeGroup& in_root_node_group,
    const float in_time_delta,
    const UiInputParam& in_input_param,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DSC_UNUSED(in_input_param);

    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TTimeDelta), in_time_delta);

    if (in_external_render_target_or_null)
    {
        DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
    }
    UpdateRootViewportSize(in_root_node_group);

    //todo: travers node hierarcy with the in_input_state updating a UiInputInternal to effect state/ button clicks/ rollover
    for (const auto& touch : in_input_param._touch_data_array)
    {
        TraverseHierarchyInput(
            touch,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TScreenSpace),
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup),
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources)
            );
    }
}

void DscUi::UiManager::Draw(
    const UiRootNodeGroup& in_root_node_group,
    DscDag::DagCollection& in_dag_collection,
    DscRenderResource::Frame& in_frame,
    const bool in_force_draw,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame), &in_frame);
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TForceDraw), in_force_draw);

    if (in_external_render_target_or_null)
    {
        DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
    }
    UpdateRootViewportSize(in_root_node_group);

    in_dag_collection.ResolveDirtyConditionNodes();

    in_root_node_group.GetNodeToken(TUiRootNodeGroup::TDrawNode)->Update();

    return;
}

void DscUi::UiManager::UpdateRootViewportSize(
    const UiRootNodeGroup& in_root_node_group
    )
{
    DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
    auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
    DSC_ASSERT(nullptr != render_target, "invalid state");

    if (nullptr != render_target)
    {
        const DscCommon::VectorInt2 viewport_size = render_target->GetViewportSize();
        DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize), viewport_size);

        DscUi::ScreenSpace screen_space({ DscCommon::VectorFloat4(
            0.0f,
            0.0f,
            static_cast<float>(viewport_size.GetX()),
            static_cast<float>(viewport_size.GetY())
        ), DscCommon::VectorFloat4(
            0.0f,
            0.0f,
            static_cast<float>(viewport_size.GetX()),
            static_cast<float>(viewport_size.GetY())
        ) });

        DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TScreenSpace), screen_space);
    }
    return;
}


DscDag::NodeToken DscUi::UiManager::MakeDrawStack(
    const TComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array,
    const UiRootNodeGroup& in_root_node_group,
    DscDag::NodeToken in_last_render_target_or_null,
    DscDag::NodeToken in_render_request_size_node,
    DscDag::NodeToken in_child_array_node_or_null,
    UiComponentResourceNodeGroup& in_component_resource_group
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    std::vector<DscDag::NodeToken> array_draw_nodes;
    std::vector<DscDag::NodeToken> effect_param_array;
    DscDag::NodeToken last_draw_node = nullptr;

    {
        DscDag::NodeToken ui_render_target_node = nullptr;
        if ((0 == in_effect_array.size()) && (nullptr != in_last_render_target_or_null))
        {
            ui_render_target_node = in_last_render_target_or_null;
        }
        else
        {
            DscDag::NodeToken component_clear_colour = in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TClearColour);

            ui_render_target_node = MakeUiRenderTargetNode(
                in_draw_system, 
                *_render_target_pool, 
                in_dag_collection, 
                component_clear_colour,
                in_render_request_size_node,
                in_component_resource_group
                );
        }

        last_draw_node = MakeDrawNode(
            GetDrawTypeFromComponentType(in_construction_helper._component_type),
            &in_construction_helper,
            in_draw_system,
            in_dag_collection,
            array_draw_nodes,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame),
            ui_render_target_node,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            nullptr,
            nullptr,
            in_child_array_node_or_null,
            in_component_resource_group
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
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect clear colour"));

                // you could put this in the effect_param_array, but then the stride of the effect param could be weird
                // could also put it in in_component_resource_group? have it's own array of 

                ui_render_target_node = MakeUiRenderTargetNode(
                    in_draw_system, 
                    *_render_target_pool, 
                    in_dag_collection, 
                    effect_clear_colour, 
                    in_render_request_size_node,
                    in_component_resource_group
                    );
            }

            const auto& effect_data = in_effect_array[index];
            DscDag::NodeToken effect_param = in_dag_collection.CreateValue(
                effect_data._effect_param,
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                &in_component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "effect param"));
            effect_param_array.push_back(effect_param);
            DscDag::NodeToken effect_tint = in_dag_collection.CreateValue(
                effect_data._effect_param_tint,
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                &in_component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "effect tint"));
            effect_param_array.push_back(effect_tint);

            last_draw_node = MakeDrawNode(
                GetDrawTypeFromEffectType(effect_data._effect_type),
                nullptr,
                in_draw_system,
                in_dag_collection,
                array_draw_nodes,
                in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame),
                ui_render_target_node,
                in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
                effect_param,
                effect_tint,
                in_child_array_node_or_null,
                in_component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "effect draw")
            );
            array_draw_nodes.push_back(last_draw_node);
        }
    }

    if (0 < effect_param_array.size())
    {
        in_component_resource_group.SetNodeToken(TUiComponentResourceNodeGroup::TEffectParamArray,
            in_dag_collection.CreateValue(
                effect_param_array,
                DscDag::CallbackNever<std::vector<DscDag::NodeToken>>::Function,
                &in_component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "effect param array")
            ));
    }

    DSC_ASSERT(nullptr != last_draw_node, "invalid state");

    return last_draw_node;
}


DscDag::NodeToken DscUi::UiManager::MakeDrawNode(
    const TUiDrawType in_type,
    const TComponentConstructionHelper* const in_construction_helper_or_null,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::vector<DscDag::NodeToken>& in_array_input_stack,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_param_or_null,
    DscDag::NodeToken in_effect_tint_or_null,
    DscDag::NodeToken in_child_array_node_or_null,
    UiComponentResourceNodeGroup& in_component_resource_group
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
    case TUiDrawType::TDebugGrid:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _debug_grid_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");

            const DscCommon::VectorInt2 viewport_size = ui_render_target->GetViewportSize();

            auto& buffer = shader_buffer->GetConstant<TDebugGridConstantBuffer>(0);
            buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
            buffer._width_height[1] = static_cast<float>(viewport_size.GetY());

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(weak_shader.lock(), shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        auto shader_buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
    }
    break;
    case TUiDrawType::TFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");
            DscCommon::VectorFloat4 fill_colour = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[3]);

            auto& buffer = shader_buffer->GetConstant<TFillConstantBuffer>(0);
            buffer._colour[0] = fill_colour.GetX();
            buffer._colour[1] = fill_colour.GetY();
            buffer._colour[2] = fill_colour.GetZ();
            buffer._colour[3] = fill_colour.GetW();

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(weak_shader.lock(), shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        auto shader_buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
        DSC_ASSERT(nullptr != in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TFillColour), "invalid state");
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TFillColour), result_node);
    }
    break;
    case TUiDrawType::TImage:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _image_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_resource = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderResource>>(in_input_array[2]);

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                auto shader = weak_shader.lock();
                shader->SetShaderResourceViewHandle(0, shader_resource->GetHeapWrapperItem());
                frame->SetShader(shader);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DSC_ASSERT(nullptr != in_construction_helper_or_null, "invalid state");
        auto texture_node = in_dag_collection.CreateValue(
            in_construction_helper_or_null->_texture,
            DscDag::CallbackOnSetValue<std::shared_ptr<DscRenderResource::ShaderResource>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "fill colour"));
        in_component_resource_group.SetNodeToken(TUiComponentResourceNodeGroup::TTexture, texture_node);

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, texture_node, result_node);
    }
    break;
    case TUiDrawType::TUiPanel:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _ui_panel_geometry;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _ui_panel_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            const auto& ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            const std::vector<UiNodeGroup>& child_array = DscDag::DagCollection::GetValueType<std::vector<UiNodeGroup>>(in_input_array[2]);

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                auto shader = weak_shader.lock();
                auto geometry = weak_geometry.lock();
                const DscCommon::VectorInt2 parent_render_size = ui_render_target->GetViewportSize();

                for (const auto& child : child_array)
                {
                    DscUi::UiRenderTarget* child_render_target = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(child.GetNodeToken(TUiNodeGroup::TDrawNode));
                    DSC_ASSERT(nullptr != child_render_target, "invalid state");
                    auto child_texture = child_render_target->GetTexture();
                    if (nullptr == child_texture)
                    {
                        continue;
                    }

                    const auto& shader_constant_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(child.GetNodeToken(TUiNodeGroup::TUiPanelShaderConstantBuffer));

                    auto& buffer = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBuffer>(0);
                    //float _pos_size[4]; // _pos_x_y_size_width_height;
                    // geometry is in range [-1 ... 1], but we want the offset relative to top left
                    const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(child.GetNodeToken(TUiNodeGroup::TGeometryOffset));
                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(child.GetNodeToken(TUiNodeGroup::TGeometrySize));
                    buffer._pos_size[0] = (static_cast<float>(geometry_offset.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f) - 1.0f;
                    buffer._pos_size[1] = ((1.0f - static_cast<float>(geometry_offset.GetY()) / static_cast<float>(parent_render_size.GetY())) * 2.0f) - 1.0f;
                    buffer._pos_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f;
                    buffer._pos_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(parent_render_size.GetY()) * 2.0f;

                    //float _uv_size[4]; // _ui_x_y_size_width_height;
                    const DscCommon::VectorFloat2& scroll_value = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(child.GetNodeToken(TUiNodeGroup::TScrollPos));
                    const DscCommon::VectorInt2 render_viewport_size = child_render_target->GetViewportSize();
                    const DscCommon::VectorInt2 render_texture_size = child_render_target->GetTextureSize();
                    const float scroll_x = std::min(1.0f, std::max(0.0f, std::abs(scroll_value.GetX())));
                    buffer._uv_size[0] = static_cast<float>(render_viewport_size.GetX() - geometry_size.GetX()) * scroll_x / static_cast<float>(render_texture_size.GetX());
                    const float scroll_y = std::min(1.0f, std::max(0.0f, std::abs(scroll_value.GetY())));
                    buffer._uv_size[1] = static_cast<float>(render_viewport_size.GetY() - geometry_size.GetY()) * scroll_y / static_cast<float>(render_texture_size.GetY());
                    buffer._uv_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(render_texture_size.GetX());
                    buffer._uv_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(render_texture_size.GetY());

                    shader->SetShaderResourceViewHandle(0, child_texture);
                    frame->SetShader(shader, shader_constant_buffer);
                    frame->Draw(geometry);
                }
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DSC_ASSERT(nullptr != in_child_array_node_or_null, "invalid state");
        DscDag::DagCollection::LinkIndexNodes(2, in_child_array_node_or_null, result_node);
    }
    break;
    case TUiDrawType::TText:
    {
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                DscRenderResource::Frame& frame = *DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
                //DSC_ASSERT(nullptr != frame, "invalid state");
                const auto& ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                DSC_ASSERT(nullptr != ui_render_target, "invalid state");
                const TUiComponentTextData& text_data = DscDag::DagCollection::GetValueType<TUiComponentTextData>(in_input_array[2]);

                if (true == ui_render_target->ActivateRenderTarget(frame))
                {
                    DscText::TextRun* text_run_raw = text_data._text_run.get();
                    if (nullptr != text_run_raw)
                    {
                        text_run_raw->SetTextContainerSize(ui_render_target->GetViewportSize());

                        auto geometry = text_run_raw->GetGeometry(&frame.GetDrawSystem(), &frame);
                        DSC_ASSERT(nullptr != text_data._text_manager, "invalid state");
                        auto shader = text_data._text_manager->GetShader(&frame.GetDrawSystem(), &frame);
                        frame.SetShader(shader);
                        frame.Draw(geometry);
                    }
                }

                out_value = ui_render_target.get();
            },
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TText), result_node);

    }
    break;
    case TUiDrawType::TEffectCorner:
        result_node = MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_round_corner_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
    break;
    case TUiDrawType::TEffectDropShadow:
        result_node = MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_drop_shadow_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectInnerShadow:
        result_node = MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_inner_shadow_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectStroke:
        result_node = MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_stroke_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectTint:
        result_node = MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_tint_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;

    }
    return result_node;
}
