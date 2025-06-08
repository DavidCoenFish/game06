#pragma once
#include "dsc_render_resource.h"

#include <dsc_common/dsc_common.h>

namespace DscRender
{
	class HeapWrapperItem;
}

namespace DscRenderResource
{
	struct UnorderedAccessInfo
	{
	public:
		static std::shared_ptr<UnorderedAccessInfo> Factory(
			const std::shared_ptr<DscRender::HeapWrapperItem>& in_unordered_access_view_handle = nullptr,
			const D3D12_SHADER_VISIBILITY in_visiblity = D3D12_SHADER_VISIBILITY_ALL
		);

		explicit UnorderedAccessInfo(
			const std::shared_ptr<DscRender::HeapWrapperItem>& in_unordered_access_view_handle = nullptr,
			const D3D12_SHADER_VISIBILITY in_visiblity = D3D12_SHADER_VISIBILITY_ALL
		);
		void SetUnorderedAccessViewHandle(const std::shared_ptr<DscRender::HeapWrapperItem>& in_unordered_access_view_handle);
		const D3D12_SHADER_VISIBILITY GetVisiblity() const
		{
			return _visiblity;
		}

		void Activate(
			ID3D12GraphicsCommandList* const in_command_list,
			const int in_root_param_index
		);

	private:
		std::shared_ptr<DscRender::HeapWrapperItem> _unordered_access_view_handle;
		D3D12_SHADER_VISIBILITY _visiblity;
	};
}//namespace DscRenderResource
