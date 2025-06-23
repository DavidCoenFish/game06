#include "ui_manager.h"
#include "screen_quad.h"
#include "ui_component_canvas.h"
#include "ui_component_debug_fill.h"
#include "ui_component_fill.h"
#include "ui_dag_node_component.h"
#include "ui_enum.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_common\vector_float2.h>
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
    static const std::vector<D3D12_INPUT_ELEMENT_DESC> s_input_element_desc_array({
        D3D12_INPUT_ELEMENT_DESC
        {
            "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        },
        // can not just 
        D3D12_INPUT_ELEMENT_DESC
        {
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        }
        });

    DscDag::NodeToken MakeNodeGetClearColour(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_ui_component)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DSC_ASSERT(ui_component != nullptr, "invalid state");

            value = ui_component->GetComponent().GetClearColour();
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetChildAvaliableSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, DscDag::NodeToken in_parent_avaliable_size, DscDag::NodeToken in_child_index)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            int32 child_index = DscDag::DagCollection::GetValueType<int32>(in_input_array[2]);

            value = ui_component->GetComponent().GetChildAvaliableSize(parent_avaliable_size, child_index);
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_parent_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_child_index, node);
        
        return node;
    }

    DscDag::NodeToken MakeNodeConvertAvaliableSizeToDesiredSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_ui_component, DscDag::NodeToken in_avaliable_size)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value = ui_component->GetComponent().ConvertAvaliableSizeToDesiredSize(avaliable_size);
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_avaliable_size, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetChildGeometrySize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, DscDag::NodeToken in_child_avaliable_size, DscDag::NodeToken in_child_desired_size)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 child_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            DscCommon::VectorInt2 child_desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

            value = ui_component->GetComponent().GetChildGeometrySize(child_desired_size, child_avaliable_size);
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_parent_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_child_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_child_desired_size, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetChildGeometryOffset(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, DscDag::NodeToken in_parent_avaliable_size, DscDag::NodeToken in_parent_child_index)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            int32 child_index = DscDag::DagCollection::GetValueType<int32>(in_input_array[2]);

            value = ui_component->GetComponent().GetChildGeometryOffset(parent_avaliable_size, child_index);
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_parent_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_parent_child_index, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetParentChildIndex(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_ui_component)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);

            value = ui_component->GetComponent().GetParentChildIndex();
        });
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, node);

        return node;
    }

    //DscDag::NodeToken MakeNodeCalculateRenderSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_desired_size, DscDag::NodeToken in_geometry_size)
    //{
    //    DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
    //        DscCommon::VectorInt2 desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
    //        DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

    //        value = DscCommon::VectorInt2(
    //            std::max(desired_size.GetX(), geometry_size.GetX()),
    //            std::max(desired_size.GetY(), geometry_size.GetY())
    //            );
    //    });

    //    DscDag::DagCollection::LinkIndexNodes(0, in_desired_size, node);
    //    DscDag::DagCollection::LinkIndexNodes(1, in_geometry_size, node);

    //    return node;
    //}


    DscDag::NodeToken MakeNodeCalculateRenderTarget(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_desired_size, DscRenderResource::RenderTargetPool* in_render_target_pool, DscRender::DrawSystem* const in_draw_system, DscDag::NodeToken in_clear_colour)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([in_render_target_pool, in_draw_system](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscCommon::VectorInt2 desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            DscCommon::VectorFloat4 clear_colour = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[1]);

            std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> pool_texture = {};
            if (typeid(std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>) == value.type())
            {
                pool_texture = std::any_cast<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(value);
            }

            std::vector<DscRender::RenderTargetFormatData> target_format_data_array = {};
            target_format_data_array.push_back(
                DscRender::RenderTargetFormatData(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    true,
                    clear_colour
                )
            );
            pool_texture = in_render_target_pool->CheckOrMakeRenderTarget(
                pool_texture,
                in_draw_system,
                target_format_data_array,
                DscRender::RenderTargetDepthData(),
                desired_size
                );

            value = pool_texture;
        });

        DscDag::DagCollection::LinkIndexNodes(0, in_desired_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_clear_colour, node);

        return node;
    }

    DscDag::NodeToken MakeNodeUiPanelShaderConstant(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_render_size, DscDag::NodeToken in_geometry_offset, DscDag::NodeToken in_geometry_size, DscDag::NodeToken in_render_size, DscDag::NodeToken in_scroll_value)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscCommon::VectorInt2 parent_render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            DscCommon::VectorInt2 geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);
            DscCommon::VectorInt2 render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
            DscCommon::VectorFloat2 scroll_value = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(in_input_array[4]);

            DscUi::TUiPanelShaderConstantBuffer constant_buffer = {};
            // geometry is in range [-1 ... 1]
            constant_buffer._pos_size[0] = (static_cast<float>(geometry_offset.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f) - 1.0f;
            constant_buffer._pos_size[1] = (static_cast<float>(geometry_offset.GetY()) / static_cast<float>(parent_render_size.GetY()) * 2.0f) - 1.0f;
            constant_buffer._pos_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f;
            constant_buffer._pos_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(parent_render_size.GetY()) * 2.0f;
            // atd::abs to allow ping point anim of scroll as it range [-1 ... 1] => [0 .. 1]
            constant_buffer._uv_size[0] = static_cast<float>(render_size.GetX() - geometry_size.GetX()) * std::abs(scroll_value.GetX()) / static_cast<float>(render_size.GetX());
            constant_buffer._uv_size[1] = static_cast<float>(render_size.GetY() - geometry_size.GetY()) * std::abs(scroll_value.GetY()) / static_cast<float>(render_size.GetY());
            constant_buffer._uv_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(render_size.GetX());
            constant_buffer._uv_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(render_size.GetY());
            value = constant_buffer;
        });

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_render_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_geometry_offset, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(3, in_render_size, node);
        DscDag::DagCollection::LinkIndexNodes(4, in_scroll_value, node);

        return node;
    }

    DscDag::NodeToken MakeNodePixelTraversal(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_geometry_size, DscDag::NodeToken in_render_size)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            DscCommon::VectorInt2 render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value = DscCommon::VectorInt2(
                std::max(0, render_size.GetX() - geometry_size.GetX()),
                std::max(0, render_size.GetY() - geometry_size.GetY())
                );
        });

        DscDag::DagCollection::LinkIndexNodes(0, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_render_size, node);

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

    //_ui_panel_geometry
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        _ui_panel_geometry = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            s_input_element_desc_array,
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
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
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
            DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
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


}

DscUi::UiManager::~UiManager()
{
    //nop
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentDebugFill(DscRender::DrawSystem& in_draw_system, const int32 in_parent_child_index)
{
    auto buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);

    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentDebugFill>(
        in_parent_child_index,
        _debug_grid_shader,
        buffer,
        _full_target_quad
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentFill(DscRender::DrawSystem& in_draw_system, const DscCommon::VectorFloat4& in_background_colour, const int32 in_parent_child_index)
{
    (void*)&in_draw_system;
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentFill>(
        in_parent_child_index,
        in_background_colour
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentCanvas(DscRender::DrawSystem& in_draw_system, const DscCommon::VectorFloat4& in_background_colour, const int32 in_parent_child_index)
{
    (void*)&in_draw_system;
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentCanvas>(
        in_parent_child_index,
        _ui_panel_shader,
        _ui_panel_geometry,
        in_background_colour
        );
    return result;
}

DscUi::UiManager::ResultNodeData DscUi::UiManager::MakeUiRootNode(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component
)
{
    DscDag::NodeToken frame = in_dag_collection.CreateValue(std::any((DscRenderResource::Frame*)nullptr), DscDag::TValueChangeCondition::TNever DSC_DEBUG_ONLY(DSC_COMMA "frame"));
    DscDag::NodeToken render_target = in_dag_collection.CreateValue(std::any((DscRender::IRenderTarget*)nullptr), DscDag::TValueChangeCondition::TNever  DSC_DEBUG_ONLY(DSC_COMMA "render target"));
    DscDag::NodeToken force_draw = in_dag_collection.CreateValue(std::any(0), DscDag::TValueChangeCondition::TOnSet DSC_DEBUG_ONLY(DSC_COMMA "force draw"));
    DscDag::NodeToken clear_on_draw = in_dag_collection.CreateValue(std::any(false), DscDag::TValueChangeCondition::TNever DSC_DEBUG_ONLY(DSC_COMMA "clear on draw"));
    DscDag::NodeToken scroll_value = in_dag_collection.CreateValue(std::any(DscCommon::VectorFloat2()), DscDag::TValueChangeCondition::TOnValueChange DSC_DEBUG_ONLY(DSC_COMMA "scroll value"));
    DscDag::NodeToken ui_component = nullptr;
    IUiComponent* ui_component_raw = in_component.get();
    {
        auto node = std::make_unique<UiDagNodeComponent>(std::move(in_component));
        ui_component = in_dag_collection.AddCustomNode(std::move(node));
    }
    DscDag::NodeToken avaliable_size = in_dag_collection.CreateValue(std::any(DscCommon::VectorInt2()), DscDag::TValueChangeCondition::TOnValueChange DSC_DEBUG_ONLY(DSC_COMMA "avaliable size"));
    DscDag::NodeToken desired_size = MakeNodeConvertAvaliableSizeToDesiredSize(in_dag_collection, ui_component, avaliable_size);
    DscDag::NodeToken geometry_offset = in_dag_collection.CreateValue(std::any(DscCommon::VectorInt2()), DscDag::TValueChangeCondition::TOnValueChange DSC_DEBUG_ONLY(DSC_COMMA "geometry offset"));
    DscDag::NodeToken geometry_size = avaliable_size;
    DscDag::NodeToken pixel_traversal_node = MakeNodePixelTraversal(in_dag_collection, geometry_size, geometry_size);

    auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);

    DscDag::NodeToken ui_panel_shader_constant = MakeNodeUiPanelShaderConstant(in_dag_collection, geometry_size, geometry_offset, geometry_size, geometry_size, scroll_value);

    DscDag::NodeToken master_ui_root_node = in_dag_collection.CreateCalculate([](std::any&, std::set<DscDag::NodeToken>& in_input_set, std::vector<DscDag::NodeToken>& in_input_array) {
        DscRenderResource::Frame* frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TFrame)]);
        const bool clear_on_draw = DscDag::DagCollection::GetValueType<bool>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TAllowClearOnDraw)]);
        DscRender::IRenderTarget* render_target = DscDag::DagCollection::GetValueType<DscRender::IRenderTarget*>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TRenderTarget)]);
        UiDagNodeComponent* ui_component = dynamic_cast<UiDagNodeComponent*>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TUiComponent)]);
        const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[static_cast<int32>(UiRootNodeInputIndex::TUiScale)]);
        (void*)&ui_scale;

        //ensure this is calculated by fetching it, so that future dirty triggers bubble up to the render node
        in_input_array[static_cast<int32>(UiRootNodeInputIndex::TShaderConstant)]->GetValue();

        // loop over in_input to ensure that they have rendered
        for (const auto& item : in_input_set)
        {
            item->GetValue();
        }

        // prep for ui component to draw
        frame->SetRenderTarget(render_target, clear_on_draw);
        if ((nullptr != ui_component) && (nullptr != render_target))
        {
            ui_component->GetComponent().Draw(*frame, *render_target);
        }
    });

    DSC_ASSERT(nullptr != _dag_resource, "invalid state");

    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TFrame), frame, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TRenderTarget), render_target, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TDeviceRestore), _dag_resource->GetDagNodeRestored(), master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TUiScale), _dag_node_ui_scale, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TForceDraw), force_draw, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TAllowClearOnDraw), clear_on_draw, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TUiComponent), ui_component, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TShaderConstant), ui_panel_shader_constant, master_ui_root_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiRootNodeInputIndex::TAvaliableSize), avaliable_size, master_ui_root_node);

    ui_component_raw->SetNode(master_ui_root_node, desired_size, pixel_traversal_node, scroll_value, ui_panel_shader_constant);

    return ResultNodeData({
        master_ui_root_node,
        ui_component,
        avaliable_size,
        desired_size
        });
}

DscUi::UiManager::ResultNodeData DscUi::UiManager::MakeUiNode(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,

    DscDag::NodeToken in_parent_ui_component,
    DscDag::NodeToken in_parent_avaliable_size,
    DscDag::NodeToken in_parent_render_size, // parent desired size not in directly in the parent UiComponent, and not of a know offset in the node [root?node?]
    DscDag::NodeToken in_root_node
)
{
    DscDag::NodeToken frame = in_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TFrame));
    DscDag::NodeToken scroll_value = in_dag_collection.CreateValue(std::any(DscCommon::VectorFloat2()), DscDag::TValueChangeCondition::TOnValueChange);

    DscDag::NodeToken ui_component = nullptr;
    IUiComponent* ui_component_raw = in_component.get();
    {
        auto node = std::make_unique<UiDagNodeComponent>(std::move(in_component));
        ui_component = in_dag_collection.AddCustomNode(std::move(node));
    }
    DscDag::NodeToken parent_child_index = MakeNodeGetParentChildIndex(in_dag_collection, ui_component);
    DscDag::NodeToken avaliable_size = MakeNodeGetChildAvaliableSize(in_dag_collection, in_parent_ui_component, in_parent_avaliable_size, parent_child_index);
    DscDag::NodeToken desired_size = MakeNodeConvertAvaliableSizeToDesiredSize(in_dag_collection, ui_component, avaliable_size);
    DscDag::NodeToken clear_colour_node = MakeNodeGetClearColour(in_dag_collection, ui_component);

    DscDag::NodeToken geometry_offset = MakeNodeGetChildGeometryOffset(in_dag_collection, in_parent_ui_component, in_parent_avaliable_size, parent_child_index);
    DscDag::NodeToken geometry_size = MakeNodeGetChildGeometrySize(in_dag_collection, in_parent_ui_component, avaliable_size, desired_size);

    DscDag::NodeToken render_target_pool_texture = MakeNodeCalculateRenderTarget(in_dag_collection, desired_size, _render_target_pool.get(), &in_draw_system, clear_colour_node);

    DscDag::NodeToken pixel_traversal_size_node = MakeNodePixelTraversal(in_dag_collection, geometry_size, desired_size);
    DscDag::NodeToken shader_constant_node = MakeNodeUiPanelShaderConstant(in_dag_collection, in_parent_render_size, geometry_offset, geometry_size, desired_size, scroll_value);

    DscDag::NodeToken ui_node = in_dag_collection.CreateCalculate([](std::any& out_value, std::set<DscDag::NodeToken>& in_input_set, std::vector<DscDag::NodeToken>& in_input_array) {
        DscRenderResource::Frame* frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[static_cast<int32>(UiNodeInputIndex::TFrame)]);
        UiDagNodeComponent* ui_component = dynamic_cast<UiDagNodeComponent*>(in_input_array[static_cast<int32>(UiNodeInputIndex::TUiComponent)]);
        auto render_target_pool_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(in_input_array[static_cast<int32>(UiNodeInputIndex::TRenderTargetPoolTexture)]);
        DscRender::IRenderTarget* render_target = render_target_pool_texture->_render_target_texture.get();
        const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[static_cast<int32>(UiNodeInputIndex::TUiScale)]);
        (void*)&ui_scale;

        //ensure this is calculated by fetching it, so that future dirty triggers bubble up to the render node
        in_input_array[static_cast<int32>(UiNodeInputIndex::TShaderConstant)]->GetValue();

        // loop over in_input to ensure that they have rendered
        for (const auto& item : in_input_set)
        {
            item->GetValue();
        }

        frame->AddFrameResource(render_target_pool_texture->_render_target_texture);

        // prep for ui component to draw
        frame->SetRenderTarget(render_target, true);

        if ((nullptr != ui_component) && (nullptr != render_target))
        {
            ui_component->GetComponent().Draw(*frame, *render_target);
        }

        out_value = render_target_pool_texture->_render_target_texture->GetShaderResourceHeapWrapperItem(0);
    });

    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TFrame), frame, ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TDeviceRestore), _dag_resource->GetDagNodeRestored(), ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TUiScale), _dag_node_ui_scale, ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TUiComponent), ui_component, ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TRenderTargetPoolTexture), render_target_pool_texture, ui_node);
    DscDag::DagCollection::LinkIndexNodes(static_cast<int32>(UiNodeInputIndex::TShaderConstant), shader_constant_node, ui_node);

    ui_component_raw->SetNode(ui_node, desired_size, pixel_traversal_size_node, scroll_value, shader_constant_node);

    return ResultNodeData({
        ui_node,
        ui_component,
        avaliable_size,
        desired_size
        });
}

DscUi::UiManager::ResultNodeData DscUi::UiManager::MakeUiNodeCanvasChild(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,

    DscDag::NodeToken in_parent_ui_component_node, // assert if not a UiComponentCanvas
    DscDag::NodeToken in_parent_avaliable_size,
    DscDag::NodeToken in_parent_render_size,
    DscDag::NodeToken in_ui_root_node,

    const VectorUiCoord2& in_child_size,
    const VectorUiCoord2& in_child_pivot,
    const VectorUiCoord2& in_attach_point
)
{
    IUiComponent* ui_component_raw = in_component.get();

    UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_parent_ui_component_node);
    UiComponentCanvas* parent_canvas = dynamic_cast<UiComponentCanvas*>(&ui_dag_node_component->GetComponent());

    auto result = MakeUiNode(
        in_draw_system,
        in_dag_collection,
        std::move(in_component),
        in_parent_ui_component_node,
        in_parent_avaliable_size,
        in_parent_render_size, // parent desired size not in directly in the parent UiComponent, and not of a know offset in the node [root?node?]
        in_ui_root_node
    );

    parent_canvas->AddChild(in_draw_system, in_child_size, in_child_pivot, in_attach_point, ui_component_raw);

    return result;
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
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TAllowClearOnDraw));
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
        DscDag::NodeToken node = in_ui_root_node->GetIndexInput(static_cast<int32>(UiRootNodeInputIndex::TAvaliableSize));
        DscDag::DagCollection::SetValueType<DscCommon::VectorInt2>(node, in_render_target->GetViewportSize());
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
