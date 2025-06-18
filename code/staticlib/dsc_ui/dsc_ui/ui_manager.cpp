#include "ui_manager.h"
#include "screen_quad.h"
#include "ui_component_debug_fill.h"
#include "ui_dag_node_component.h"
#include "ui_enum.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag_render\dag_resource.h>
#include <dsc_render\draw_system.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\constant_buffer_info.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\render_target_pool.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\shader_pipeline_state_data.h>
#include <dsc_render_resource\shader_resource_info.h>

namespace
{
} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection)
{
    _dag_resource = DscDagRender::DagResource::Factory(&in_draw_system, &in_dag_collection);
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

    _dag_node_ui_scale = in_dag_collection.CreateValue(std::any(1.0f));

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
                TSizeShaderConstantBuffer(),
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
    /*
    // _screen_quad_texture_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_ps.cso")))
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
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_ps.cso")))
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

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentDebugFill(DscRender::DrawSystem& in_draw_system)
{
    auto buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);

    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentDebugFill>(
        _debug_grid_shader,
        buffer,
        _full_target_quad
        );
    return result;
}

DscDag::NodeToken DscUi::UiManager::MakeUiRootNode(
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component
)
{
    DscDag::NodeToken frame = in_dag_collection.CreateValue(std::any((DscRenderResource::Frame*)nullptr), DscDag::TValueChangeCondition::TNever);
    DscDag::NodeToken render_target = in_dag_collection.CreateValue(std::any((DscRender::IRenderTarget*)nullptr), DscDag::TValueChangeCondition::TNever);
    DscDag::NodeToken force_draw = in_dag_collection.CreateValue(std::any(0), DscDag::TValueChangeCondition::TOnSet);
    DscDag::NodeToken clear_on_draw = in_dag_collection.CreateValue(std::any(false), DscDag::TValueChangeCondition::TNever);

    DscDag::NodeToken ui_component = nullptr;
    {
        auto node = std::make_unique<UiDagNodeComponent>(std::move(in_component));
        ui_component = in_dag_collection.AddCustomNode(std::move(node));
    }
    DscDag::NodeToken target_size = in_dag_collection.CreateValue(std::any(DscCommon::VectorInt2(0, 0)));

    DscDag::NodeToken master_ui_root_node = in_dag_collection.CreateCalculate([](std::any&, std::set<DscDag::NodeToken>& in_input_set, std::vector<DscDag::NodeToken>& in_input_array) {
        DscRenderResource::Frame* frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TFrame)]);
        const bool clear_on_draw = DscDag::DagCollection::GetValueType<bool>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TClearOnDraw)]);
        DscRender::IRenderTarget* render_target = DscDag::DagCollection::GetValueType<DscRender::IRenderTarget*>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TRenderTarget)]);
        UiDagNodeComponent* ui_component = dynamic_cast<UiDagNodeComponent*>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TUiComponent)]);

        // loop over in_input_set to ensure that they have rendered
        for (const auto& item : in_input_set)
        {
            item->GetValue();
        }

        // prep for ui component to draw
        frame->SetRenderTarget(render_target, clear_on_draw);
        if (nullptr != ui_component)
        {
            ui_component->GetComponent().Draw(*frame, render_target->GetSize());
        }
    });

    //
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TFrame), frame, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TForceDraw), force_draw, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TClearOnDraw), clear_on_draw, master_ui_root_node);
    DSC_ASSERT(nullptr != _dag_resource, "invalid state");
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TDeviceRestore), _dag_resource->GetDagNodeRestored(), master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TRenderTarget), render_target, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TTargetSize), target_size, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TUiScale), _dag_node_ui_scale, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TUiComponent), ui_component, master_ui_root_node);

    return master_ui_root_node;
}

//DscDag::NodeToken MakeUiNode(std::unique_ptr<IUiComponent>, DscDag::NodeToken in_parent, DscDag::NodeToken in_root_node)
void DscUi::UiManager::DrawUiSystem(
    DscRender::IRenderTarget* const in_render_target,
    const bool in_always_draw, // if this render target is shared, need to at least redraw the top level ui
    const bool in_clear_on_draw, // clear the top level render target before we draw to it
    DscDag::NodeToken in_ui_root_node,
    DscRenderResource::Frame& in_frame
)
{
    DSC_ASSERT(nullptr != in_ui_root_node, "invalid param");
    DSC_ASSERT(nullptr != in_render_target, "invalid param");
    if (true == in_always_draw)
    {
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TForceDraw));
        DscDag::DagCollection::SetValueType<int32>(node, 0);
    }
    {
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TClearOnDraw));
        DscDag::DagCollection::SetValueType<bool>(node, in_clear_on_draw);
    }
    {
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TFrame));
        DscDag::DagCollection::SetValueType(node, &in_frame);
    }
    {
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TRenderTarget));
        DscDag::DagCollection::SetValueType<DscRender::IRenderTarget*>(node, in_render_target);
    }
    {
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TTargetSize));
        DscDag::DagCollection::SetValueType<DscCommon::VectorInt2>(node, in_render_target->GetSize());
    }

    in_ui_root_node->GetValue();

    return;
}

//UiRootNode, commandList or Frame, time delta, current touch, array button presses)

//could be different per ui system? probably not however....
const float DscUi::UiManager::GetUiScale() const
{
    DSC_ASSERT(nullptr != _dag_node_ui_scale, "invalid state");
    const float result = DscDag::DagCollection::GetValueType<float>(_dag_node_ui_scale);
    return result;
}

void DscUi::UiManager::SetUiScale(const float in_ui_scale)
{
    DSC_ASSERT(nullptr != _dag_node_ui_scale, "invalid state");
    DscDag::DagCollection::SetValueType(_dag_node_ui_scale, in_ui_scale);
    return;
}
