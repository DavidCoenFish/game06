#include "celtic_knot.h"
#include "screen_quad.h"
#include "ui_enum.h"
#include "ui_render_target.h"
#include "make_node.h"

#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_dag\i_dag_owner.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_node_group.h>
#include <dsc_dag\debug_print.h>
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
        float _knot_tint[4];
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
	DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_component_resource_group,
	DscDag::NodeToken in_visible
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
                auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                const auto viewport_size = ui_render_target->GetViewportSize();
				out_value.Set(
					(viewport_size.GetX() / celtic_knot_size) + 1,
					(viewport_size.GetY() / celtic_knot_size) + 1
					);
				},
				owner);
			DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(data_texture_size_node, "data_texture_size_node"));
		DscDag::LinkIndexNodes(0, celtic_knot_size, data_texture_size_node);
		DscDag::LinkIndexNodes(1, in_ui_render_target_node, data_texture_size_node);

        auto clear_colour = in_dag_collection.CreateValueOnValueChange(
            DscCommon::VectorFloat4::s_zero,
            owner
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(clear_colour, "clear_colour"));

		DscDag::NodeToken data_texture_render_target_node = MakeNode::MakeUiRenderTargetNode(
                in_draw_system, 
                in_render_target_pool, 
                in_dag_collection, 
                clear_colour,
                data_texture_size_node,
                owner
                );

		// draw the data texture
		std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_full_quad_pos_uv;
		std::weak_ptr<DscRenderResource::Shader> weak_shader = _data_shader;
		data_texture_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
			DSC_ASSERT(nullptr != frame, "invalid state");
			auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
			DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);

			auto& buffer = shader_buffer->GetConstant<TDataConstantBuffer>(0);
			const auto viewport_size = ui_render_target->GetViewportSize();
			buffer._texture_size[0] = static_cast<float>(viewport_size.GetX());
			buffer._texture_size[1] = static_cast<float>(viewport_size.GetY());
			buffer._texture_size[2] = static_cast<float>(viewport_size.GetX());
			buffer._texture_size[3] = static_cast<float>(viewport_size.GetY());

			if (true == ui_render_target->ActivateRenderTarget(*frame))
			{
				frame->SetShader(weak_shader.lock(), shader_buffer);
				frame->Draw(weak_geometry.lock());
				frame->SetRenderTarget(nullptr);
			}

			out_value = ui_render_target.get();
		},
			owner);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(data_texture_node, "data texture draw"));

		auto shader_buffer = _data_shader->MakeShaderConstantBuffer(&in_draw_system);
		auto shader_buffer_node = in_dag_collection.CreateValue(
			shader_buffer,
			DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
			owner);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "data shader constant"));

		DscDag::LinkIndexNodes(0, in_frame_node, data_texture_node);
		DscDag::LinkIndexNodes(1, data_texture_render_target_node, data_texture_node);
		DscDag::LinkIndexNodes(2, shader_buffer_node, data_texture_node);
	}

	// ensure we have a celtic knot source texture (red:knot_1111, green:knot_1100, blue:knot:1000)
	DscDag::NodeToken celtic_knot_source_texture_node = nullptr;
	{
		DscDag::NodeToken celtic_knot_texture_size_node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
				const int32 celtic_knot_size = DscDag::GetValueType<int32>(in_input_array[0]);
				out_value.Set(
					celtic_knot_size,
					celtic_knot_size
					);
				},
				owner);
			DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(celtic_knot_texture_size_node, "celtic_knot_size_node"));
		DscDag::LinkIndexNodes(0, celtic_knot_size, celtic_knot_texture_size_node);
		
        auto clear_colour = in_dag_collection.CreateValueOnValueChange(
            DscCommon::VectorFloat4::s_zero,
            owner
            );
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(clear_colour, "clear_colour"));

		DscDag::NodeToken celtic_knot_texture_render_target_node = MakeNode::MakeUiRenderTargetNode(
                in_draw_system, 
                in_render_target_pool, 
                in_dag_collection, 
                clear_colour,
                celtic_knot_texture_size_node,
                owner
                );

		// draw the celtic_knot_source_texture
		std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_full_quad_pos_uv;
		std::weak_ptr<DscRenderResource::Shader> weak_shader = _knot_shader;
		celtic_knot_source_texture_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
			DSC_ASSERT(nullptr != frame, "invalid state");
			auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
			const auto viewport_size = ui_render_target->GetViewportSize();
			DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);

			auto& buffer = shader_buffer->GetConstant<TKnotConstantBuffer>(0);
			buffer._texture_size[0] = static_cast<float>(viewport_size.GetX());

			if (true == ui_render_target->ActivateRenderTarget(*frame))
			{
				frame->SetShader(weak_shader.lock(), shader_buffer);
				frame->Draw(weak_geometry.lock());
				frame->SetRenderTarget(nullptr);
			}

			out_value = ui_render_target.get();
		},
			owner);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(celtic_knot_source_texture_node, "celtic knot source texture draw"));

		auto shader_buffer = _knot_shader->MakeShaderConstantBuffer(&in_draw_system);
		auto shader_buffer_node = in_dag_collection.CreateValue(
			shader_buffer,
			DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
			owner);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "knot shader constant"));

		DscDag::LinkIndexNodes(0, in_frame_node, celtic_knot_source_texture_node);
		DscDag::LinkIndexNodes(1, celtic_knot_texture_render_target_node, celtic_knot_source_texture_node);
		DscDag::LinkIndexNodes(2, shader_buffer_node, celtic_knot_source_texture_node);
	}

	DscDag::NodeToken result_node = nullptr;
	{
		std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_full_quad_pos_uv;
		std::weak_ptr<DscRenderResource::Shader> weak_shader = _fill_knot_shader;
		result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
			DSC_ASSERT(nullptr != frame, "invalid state");
			auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
			DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			const bool visible = DscDag::GetValueType<bool>(in_input_array[7]);
			ui_render_target->SetEnabled(visible);
			if (true == visible)
			{
				auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
				const int32 celtic_knot_size = DscDag::GetValueType<int32>(in_input_array[3]);

				const DscCommon::VectorFloat4& celtic_knot_tint = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[4]);

				std::shared_ptr<DscRenderResource::Shader> shader = weak_shader.lock();
				DSC_ASSERT(nullptr != shader, "invalid state");

				// data texture
				DscUi::UiRenderTarget* const data_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[5]);
				shader->SetShaderResourceViewHandle(0, data_texture->GetTexture());

				// knot source texture
				DscUi::UiRenderTarget* const knot_souce_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[6]);
				shader->SetShaderResourceViewHandle(1, knot_souce_texture->GetTexture());

				auto& buffer = shader_buffer->GetConstant<TFillKnotConstantBuffer>(0);

				// width, height of our render target, then the size of the knot
				const auto viewport_size = ui_render_target->GetViewportSize();
				buffer._texture_size_knot_size[0] = static_cast<float>(viewport_size.GetX());
				buffer._texture_size_knot_size[1] = static_cast<float>(viewport_size.GetY());
				buffer._texture_size_knot_size[2] = static_cast<float>(celtic_knot_size);
				buffer._texture_size_knot_size[3] = static_cast<float>(knot_souce_texture->GetTextureSize().GetX());

				// width, height of the data texture, then a pixel delta x, delta y to get the knot centered on the render target
				const auto data_size = data_texture->GetTextureSize();
				buffer._data_size[0] = static_cast<float>(data_size.GetX());
				buffer._data_size[1] = static_cast<float>(data_size.GetY());
				buffer._data_size[2] = static_cast<float>((((viewport_size.GetX() / celtic_knot_size) * celtic_knot_size) - viewport_size.GetX()) / 2);
				buffer._data_size[3] = static_cast<float>((((viewport_size.GetY() / celtic_knot_size) * celtic_knot_size) - viewport_size.GetY()) / 2);

				buffer._knot_tint[0] = celtic_knot_tint.GetX();
				buffer._knot_tint[1] = celtic_knot_tint.GetY();
				buffer._knot_tint[2] = celtic_knot_tint.GetZ();
				buffer._knot_tint[3] = celtic_knot_tint.GetW();

				if (true == ui_render_target->ActivateRenderTarget(*frame))
				{
					frame->SetShader(weak_shader.lock(), shader_buffer);
					frame->Draw(weak_geometry.lock());
					frame->SetRenderTarget(nullptr);
				}
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
        DscDag::NodeToken celtic_knot_tint = DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TCelticKnotTint);
        DscDag::LinkIndexNodes(4, celtic_knot_tint, result_node);
		DscDag::LinkIndexNodes(5, data_texture_node, result_node);
		DscDag::LinkIndexNodes(6, celtic_knot_source_texture_node, result_node);
		DscDag::LinkIndexNodes(7, in_visible, result_node);

	}

	return result_node;
}

