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
#include <dsc_render_resource\render_target_texture.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\shader_pipeline_state_data.h>
#include <dsc_render_resource\shader_resource_info.h>

namespace
{
    //DscDag::NodeToken MakeChildAvaliableSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, const int32 in_child_index)
    //{
    //
    //}

    DscDag::NodeToken MakeNodeCalculateDesiredSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_ui_component, DscDag::NodeToken in_avaliable_size)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value = ui_component->GetComponent().CalculateDesiredSize(avaliable_size);
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_avaliable_size, node);

        return node;
    }

    DscDag::NodeToken MakeNodeCalculateRenderTarget(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_desired_size, const DscCommon::VectorFloat4& in_clear_colour, DscRenderResource::RenderTargetPool* in_render_target_pool, DscRender::DrawSystem* const in_draw_system)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([in_render_target_pool, in_draw_system, in_clear_colour](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscCommon::VectorInt2 desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);

            std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> pool_texture = {};
            if (typeid(std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>) == value.type())
            {
                pool_texture = std::any_cast<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(value);
            }

            if (nullptr != pool_texture)
            {
                if (true == pool_texture->AdjustForSize(desired_size))
                {
                    return;
                }
            }

            std::vector<DscRender::RenderTargetFormatData> target_format_data_array = {};
            target_format_data_array.push_back(
                DscRender::RenderTargetFormatData(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    true,
                    in_clear_colour
                )
            );
            pool_texture = in_render_target_pool->MakeOrReuseRenderTarget(
                in_draw_system,
                target_format_data_array,
                DscRender::RenderTargetDepthData(),
                desired_size
                );

            value = pool_texture;
        });

        DscDag::DagCollection::LinkIndexNodes(0, in_desired_size, node);

        return node;
    }

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

        // loop over in_input to ensure that they have rendered
        for (const auto& item : in_input_set)
        {
            item->GetValue();
        }

        // prep for ui component to draw
        frame->SetRenderTarget(render_target, clear_on_draw);
        if (nullptr != ui_component)
        {
            ui_component->GetComponent().Draw(*frame, render_target->GetViewportSize());
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

DscDag::NodeToken DscUi::UiManager::MakeUiNode(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    IUiComponent& in_parent_component,
    DscDag::NodeToken in_root_node,
    const int32 in_parent_child_index
) 
{
    DscDag::NodeToken frame = in_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TFrame));

    DscDag::NodeToken ui_component = nullptr;
    {
        auto node = std::make_unique<UiDagNodeComponent>(std::move(in_component));
        ui_component = in_dag_collection.AddCustomNode(std::move(node));
    }
    DscDag::NodeToken avaliable_size = in_parent_component.GetChildAvalableSizeNode(in_parent_child_index);
    DscDag::NodeToken desired_size = MakeNodeCalculateDesiredSize(in_dag_collection, ui_component, avaliable_size);
    DscDag::NodeToken render_target_pool_texture = MakeNodeCalculateRenderTarget(in_dag_collection, desired_size, in_component->GetClearColour(), _render_target_pool.get(), &in_draw_system);

    DscDag::NodeToken ui_node = in_dag_collection.CreateCalculate([](std::any&, std::set<DscDag::NodeToken>& in_input_set, std::vector<DscDag::NodeToken>& in_input_array) {
        DscRenderResource::Frame* frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[static_cast<int32>(UiNodeInputIndex::TFrame)]);
        UiDagNodeComponent* ui_component = dynamic_cast<UiDagNodeComponent*>(in_input_array[static_cast<int32>(UiNodeInputIndex::TUiComponent)]);
        auto render_target_pool_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(in_input_array[static_cast<int32>(UiNodeInputIndex::TRenderTargetPoolTexture)]);
        DscRender::IRenderTarget* render_target = render_target_pool_texture->_render_target_texture.get();

        // loop over in_input to ensure that they have rendered
        for (const auto& item : in_input_set)
        {
            item->GetValue();
        }

        // prep for ui component to draw
        frame->SetRenderTarget(render_target, true);

        if (nullptr != ui_component)
        {
            ui_component->GetComponent().Draw(*frame, render_target->GetViewportSize());
        }
    });

    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TFrame), frame, ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TUiComponent), ui_component, ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TRenderTargetPoolTexture), render_target_pool_texture, ui_node);
    

    return ui_node;
}

DscUi::IUiComponent& DscUi::UiManager::GetComponentFromUiRootNode(DscDag::NodeToken in_ui_root_node) 
{
    UiDagNodeComponent* component = dynamic_cast<UiDagNodeComponent*>(in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TUiComponent)));
    return component->GetComponent();
}

DscUi::IUiComponent& DscUi::UiManager::GetComponentFromUiNode(DscDag::NodeToken in_ui_node)
{
    UiDagNodeComponent* component = dynamic_cast<UiDagNodeComponent*>(in_ui_node->GetIndexInput(static_cast<int32>(UiNodeInputIndex::TUiComponent)));
    return component->GetComponent();
}

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
