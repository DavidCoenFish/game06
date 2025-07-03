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

std::unique_ptr<DscUi::UiRenderTarget> DscUi::UiManager::MakeUiRenderTarget(
    DscRender::IRenderTarget* const in_render_target,
    const bool in_allow_clear_on_draw
)
{
    return std::make_unique<DscUi::UiRenderTarget>(in_render_target, in_allow_clear_on_draw);
}

std::unique_ptr<DscUi::UiRenderTarget> DscUi::UiManager::MakeUiRenderTarget(
    const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
    const bool in_allow_clear_on_draw
)
{
    return std::make_unique<DscUi::UiRenderTarget>(in_render_target_texture, in_allow_clear_on_draw);
}

DscUi::UiRootNodeGroup DscUi::UiManager::MakeRootNode(
    const TUiComponentType in_type,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<UiRenderTarget>&& in_ui_render_target
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
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
        in_type,
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

    result.SetNodeToken(TUiRootNodeGroup::TUiRenderTarget, in_dag_collection.CreateValueUnique(
        std::move(in_ui_render_target)
        DSC_DEBUG_ONLY(DSC_COMMA "ui render target")));

    result.SetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize, in_dag_collection.CreateValue(
        DscCommon::VectorInt2::s_zero,
        DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "render target size")));

    result.SetNodeToken(TUiRootNodeGroup::TScreenSpaceSize, in_dag_collection.CreateValue(
        DscCommon::VectorFloat4::s_zero,
        DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function
        DSC_DEBUG_ONLY(DSC_COMMA "screen space size")));

    result.SetNodeToken(TUiRootNodeGroup::TDrawNode, MakeDrawNode(
        in_type,
        in_draw_system,
        in_dag_collection,
        result.GetNodeToken(TUiRootNodeGroup::TFrame),
        result.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget),
        result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize)
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
        ));

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
        auto render_target = DscDag::DagCollection::GetUniqueValueType<UiRenderTarget>(node);
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

DscDag::NodeToken DscUi::UiManager::MakeDrawNode(
    const TUiComponentType in_type,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_render_target_viewport_size_node
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    DscDag::NodeToken result_node = {};
    switch (in_type)
    {
    default:
        break;
    case TUiComponentType::TGridFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_full_target_quad = _full_target_quad;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _debug_grid_shader;
        result_node = in_dag_collection.CreateCalculate<std::nullptr_t>([weak_full_target_quad, weak_shader](std::nullptr_t&, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetUniqueValueType<UiRenderTarget>(in_input_array[1]);
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
        }
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name + "Draw"));

        auto shader_buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(3, in_render_target_viewport_size_node, result_node);
    }
    }
    return result_node;
}

std::shared_ptr<DscRenderResource::Shader> DscUi::UiManager::GetEffectShader(const DscUi::TEffect in_effect)
{
    switch (in_effect)
    {
    default:
        DSC_ASSERT_ALWAYS("missing switch condition");
        break;
    case TEffect::TDropShadow:
        return _effect_drop_shadow_shader;
    case TEffect::TRoundedCorner:
        return _effect_round_corner_shader;
    case TEffect::TStroke:
        return _effect_stroke_shader;
    }
    return nullptr;
}

std::shared_ptr<DscRenderResource::GeometryGeneric> DscUi::UiManager::GetEffectGeometry(const DscUi::TEffect)
{
    return _full_target_quad;
}

