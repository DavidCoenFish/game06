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
	    // width, height of our render target, then the size of the knot
        float _texture_size_knot_size[4];
		// width, height of the data texture, then a pixel delta x, delta y to get the knot centered on the render target
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
        //_knot_shader_constant_buffer = _knot_shader->MakeShaderConstantBuffer(&in_draw_system);
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
        //_data_shader_constant_buffer = _data_shader->MakeShaderConstantBuffer(&in_draw_system);
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
        //_fill_knot_shader_constant_buffer = _fill_knot_shader->MakeShaderConstantBuffer(&in_draw_system);
    }
}

DscUi::CelticKnot::~CelticKnot()
{
	//nop
}

DscDag::NodeToken DscUi::CelticKnot::MakeDrawNode(
	std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_quad_pos_uv,
	DscDag::DagCollection& in_dag_collection,
	DscRender::DrawSystem& in_draw_system,
    DscRenderResource::RenderTargetPool& in_render_target_pool,
	DscDag::NodeToken in_frame_node,
	DscDag::NodeToken in_render_target_viewport_size,
	DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_component_resource_group
	)
{
    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(in_component_resource_group);
    DSC_ASSERT(nullptr != owner, "invalid state");

	DscDag::NodeToken celtic_knot_size = DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TCelticKnotSize);

	// make a data texture (red:0 or 1 to indicate each knot grid is solid or not) size = in_ui_render_target_node.viewport size / knot size
	DscDag::NodeToken data_texture_node = nullptr;
	{
		DscDag::NodeToken data_texture_size_node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
				const int32 celtic_knot_size = DscDag::GetValueType<int32>(in_input_array[0]);
				const DscCommon::VectorInt2& render_target_viewport_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
				out_value.Set(
					(render_target_viewport_size.GetX() / celtic_knot_size) + 1,
					(render_target_viewport_size.GetY() / celtic_knot_size) + 1
					);
				},
				owner);
			DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "data_texture_size_node"));
		DscDag::LinkIndexNodes(0, celtic_knot_size, data_texture_size_node);
		DscDag::LinkIndexNodes(1, in_render_target_viewport_size, data_texture_size_node);


		DscDag::NodeToken data_texture_render_target_node = MakeNode::MakeUiRenderTargetNode(
                in_draw_system, 
                in_render_target_pool, 
                in_dag_collection, 
                component_clear_colour,
                in_render_request_size_node,
                owner
                );
	}

	// ensure we have a celtic knot source texture (red:knot_1111, green:knot_1100, blue:knot:1000)
	DscDag::NodeToken celtic_knot_source_texture_node = nullptr;
	{


	}

    std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_full_quad_pos_uv;
    std::weak_ptr<DscRenderResource::Shader> weak_shader = _fill_knot_shader;
    DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
        DSC_ASSERT(nullptr != frame, "invalid state");
        auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
        DSC_ASSERT(nullptr != ui_render_target, "invalid state");
        auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
        const int32 celtic_knot_size = DscDag::GetValueType<int32>(in_input_array[3]);

        std::shared_ptr<DscRenderResource::Shader> shader = weak_shader.lock();
        DSC_ASSERT(nullptr != shader, "invalid state");

		// data texture
        DscUi::UiRenderTarget* const data_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[4]);
        shader->SetShaderResourceViewHandle(0, data_texture->GetTexture());

		// knot source texture
        DscUi::UiRenderTarget* const knot_souce_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[5]);
        shader->SetShaderResourceViewHandle(1, knot_souce_texture->GetTexture());

        auto& buffer = shader_buffer->GetConstant<TFillKnotConstantBuffer>(0);
		buffer._texture_size_knot_size[0] = static_cast<float>();

        if (true == ui_render_target->ActivateRenderTarget(*frame))
        {
            frame->SetShader(weak_shader.lock(), shader_buffer);
            frame->Draw(weak_geometry.lock());
            frame->SetRenderTarget(nullptr);
        }

        out_value = ui_render_target.get();
    },
        owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "celtic knot fill draw"));

    auto shader_buffer = _fill_knot_shader->MakeShaderConstantBuffer(&in_draw_system);
    auto shader_buffer_node = in_dag_collection.CreateValue(
        shader_buffer,
        DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
        owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "celtic knot fill shader constant"));

    DscDag::LinkIndexNodes(0, in_frame_node, result_node);
    DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
    DscDag::LinkIndexNodes(2, shader_buffer_node, result_node);
    DscDag::LinkIndexNodes(3, celtic_knot_size, result_node);
    DscDag::LinkIndexNodes(4, data_texture_node, result_node);
    DscDag::LinkIndexNodes(5, celtic_knot_source_texture_node, result_node);

	return result_node;
}

