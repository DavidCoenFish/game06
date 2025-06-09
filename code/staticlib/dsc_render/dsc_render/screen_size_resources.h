#pragma once
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
	class RenderTargetBackBuffer;
	struct RenderTargetFormatData;
	struct RenderTargetDepthData;

	class ScreenSizeResources
	{
	public:
		ScreenSizeResources() = delete;
		ScreenSizeResources& operator=(const ScreenSizeResources&) = delete;
		ScreenSizeResources(const ScreenSizeResources&) = delete;

		ScreenSizeResources(
			DrawSystem* const in_draw_system,
			const Microsoft::WRL::ComPtr<ID3D12Device>& in_device,
			const Microsoft::WRL::ComPtr<IDXGIFactory6>& in_dxgi_factory,
			const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& in_command_queue,
			const HWND in_hwnd,
			const UINT64 in_fence_value,
			const unsigned int in_back_buffer_count,
			const DscCommon::VectorInt2& in_size,
			const bool in_allow_tearing,
			const RenderTargetFormatData& in_target_format_data,
			const RenderTargetDepthData& in_target_depth_data
		);
		~ScreenSizeResources();
		const UINT64 GetFenceValue();
		void SetFenceValue(const UINT64 in_value);
		void Prepare(ID3D12GraphicsCommandList*& in_command_list);
		// Void Clear();
		const bool Present(
			HRESULT& in_hr,
			const Microsoft::WRL::ComPtr < ID3D12CommandQueue >& in_command_queue
		);

		IRenderTarget* GetRenderTargetBackBuffer();
		void UpdateBackBufferIndex();
		const int GetBackBufferIndex() const
		{
			return _back_buffer_index;
		}

		const DscCommon::VectorInt2 GetSize() const
		{
			return _size;
		}

		void WaitForGpu(const Microsoft::WRL::ComPtr < ID3D12CommandQueue >& in_command_queue) noexcept;
		void MoveToNextFrame(const Microsoft::WRL::ComPtr < ID3D12CommandQueue >& in_command_queue);

	private:
		static const size_t MAX_BACK_BUFFER_COUNT = 3;
		DscCommon::VectorInt2 _size;
		bool _allow_tearing;
		int _back_buffer_count;
		int _back_buffer_index;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _command_list;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _command_allocators[MAX_BACK_BUFFER_COUNT];
		UINT64 _fence_values[MAX_BACK_BUFFER_COUNT];
		Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
		Microsoft::WRL::Wrappers::Event _fence_event;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> _swap_chain;
		std::unique_ptr<RenderTargetBackBuffer> _render_target_back_buffer[MAX_BACK_BUFFER_COUNT];

	};
} //namespace DscRender
