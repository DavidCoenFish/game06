#include "frame.h"
#include "render_target_texture.h"
#include "shader.h"
#include "shader_constant_buffer.h"
#include "geometry_generic.h"
#include "unordered_access.h"

#include <dsc_common/log_system.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render/resource_list.h>

std::unique_ptr<DscRenderResource::Frame> DscRenderResource::Frame::CreateNewFrame(DscRender::DrawSystem& in_draw_system)
{
	return std::make_unique<Frame>(in_draw_system);
}

DscRenderResource::Frame::Frame(DscRender::DrawSystem& in_draw_system)
	: _draw_system(in_draw_system)
	, _command_list(nullptr)
	, _render_target(nullptr)
{
	//LOG_MESSAGE_RENDER("DscRenderResource::Frame start");

	_draw_system.Prepare(_command_list);
	_resource_list = _draw_system.MakeResourceList();
	DSC_ASSERT(nullptr != _resource_list, "Make resource list failed");
	return;
}

DscRenderResource::Frame::~Frame()
{
	SetRenderTarget(nullptr);
	_draw_system.Present();
	_draw_system.FinishResourceList(_resource_list);
	//LOG_MESSAGE_RENDER("DscRenderResource::Frame end");

	return;
}

ID3D12GraphicsCommandList* DscRenderResource::Frame::GetCommandList()
{
	return _command_list;
}

const int DscRenderResource::Frame::GetBackBufferIndex()
{
	return _draw_system.GetBackBufferIndex();
}

void DscRenderResource::Frame::SetRenderTarget(
	DscRender::IRenderTarget* const in_render_target,
	const bool in_allow_clear
	)
{
	if (_render_target == in_render_target)
	{
		return;
	}

	if ((in_render_target != nullptr) && 
		(_render_target != nullptr))
	{
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, "new in_render_target:%p\n", in_render_target->GetDebugToken());

		// cross fade was updating the child fade amount after the child had draw, moved caculate earlier as a parent dependency, but that is not a nice solution
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, "not strictly an error, but do we have Ui nodes drawing during the draw of parent, missing render node update?\n");
	}

	if (_render_target)
	{
		// DebugToken can be null for things like the backbuffer
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, "SetRenderTarget end:%p\n", _render_target->GetDebugToken());
		_render_target->EndRender(_command_list);
	}

	_render_target = in_render_target;
	if (_render_target)
	{
		// backbuffer render resource is not a IResource, but RenderTargetTexture is, deal upstream
		//_resource_list->AddResource(_render_target);
		_render_target->StartRender(_command_list, in_allow_clear);

		// DebugToken can be null for things like the backbuffer
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, "SetRenderTarget start:%p\n", _render_target->GetDebugToken());
	}

	return;
}

void DscRenderResource::Frame::SetRenderTargetTexture(
	const std::shared_ptr<RenderTargetTexture>& in_render_target,
	const bool in_allow_clear
	)
{
	//std::shared_ptr<RenderTargetTexture> resource(in_render_target);
	AddFrameResource(in_render_target);
	SetRenderTarget(in_render_target.get(), in_allow_clear);
	return;
}


void DscRenderResource::Frame::AddFrameResource(
	const std::shared_ptr<DscRender::IResource>& in_resource
	)
{
	_resource_list->AddResource(in_resource);
	return;
}

void DscRenderResource::Frame::SetShader(
	const std::shared_ptr<Shader>& in_shader,
	const std::shared_ptr<ShaderConstantBuffer>& in_shader_constant_buffer
	)
{
	in_shader->SetActive(
		_command_list,
		in_shader_constant_buffer.get()
		);
	_resource_list->AddResource(in_shader);
	_resource_list->AddResource(in_shader_constant_buffer);

	return;
}

void DscRenderResource::Frame::Draw(
	const std::shared_ptr<GeometryGeneric>& in_geometry
	)
{
	in_geometry->Draw(_command_list);
	_resource_list->AddResource(in_geometry);

	return;
}

void DscRenderResource::Frame::Dispatch(
	uint32_t in_thread_group_count_x,
	uint32_t in_thread_group_count_y,
	uint32_t in_thread_group_count_z
	)
{
	_command_list->Dispatch(
		in_thread_group_count_x,
		in_thread_group_count_y,
		in_thread_group_count_z
		);
	return;
}

//void DscRenderResource::Frame::ClearUAV(
//	const std::shared_ptr<UnorderedAccess>& in_unordered_access
//)
//{
//	in_unordered_access->Clear(_command_list);
//	_resource_list->AddResource(in_unordered_access);
//	return;
//}

void DscRenderResource::Frame::ResourceBarrier(
	DscRender::IResource* const in_resource,
	D3D12_RESOURCE_STATES in_after_state
	)
{
	in_resource->OnResourceBarrier(_command_list, in_after_state);
	return;
}

//void DscRenderResource::Frame::UpdateGeometryGeneric(
//	const std::shared_ptr<GeometryGeneric>& in_geometry,
//	const std::vector<uint8_t>& in_vertex_data_raw
//	)
//{
//	_draw_system.UpdateGeometryGeneric(
//		_command_list,
//		in_geometry.get(),
//		in_vertex_data_raw
//		);
//	_resource_list->AddResource(in_geometry);
//	return;
//}

void DscRenderResource::Frame::UpdateRenderTargetSize(
	std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target,
	const DscCommon::VectorInt2& in_size
)
{
	in_render_target->Resize(
		_command_list,
		_draw_system.GetD3dDevice(),
		in_size
		);
	_resource_list->AddResource(in_render_target);
	return;
}


#if defined(_DEBUG)

//template <>
//const std::string DscCommon::DebugPrint::PrintType<DscRenderResource::Frame*>(DscRenderResource::Frame* in_value)
//{
//	return std::string();
//}

#endif //#if defined(_DEBUG)

