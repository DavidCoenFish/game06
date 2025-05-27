#pragma once
#include <dsc_common/common.h>

namespace DscRender
{
	class HeapWrapper;
	class DrawSystem;

	class HeapWrapperItem
	{
	public:
		static std::shared_ptr < HeapWrapperItem > Factory(
			ID3D12Device2* const in_device,
			const std::shared_ptr < HeapWrapper >& in_heap_wrapper,
			const int in_length = 1
		);
		HeapWrapperItem(
			const std::shared_ptr < HeapWrapper >& in_heap_wrapper,
			const int in_index,
			const int in_length
		);
		~HeapWrapperItem();
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleFrame();
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle();
		ID3D12DescriptorHeap* const GetHeap();
		const int GetLength() const
		{
			return _length;
		};

	private:
		std::shared_ptr < HeapWrapper > _heap_wrapper;
		int _index;
		int _length;
	};
} //namespace DscRender
