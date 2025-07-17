#include "dsc_render_resource_png.h"
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/vector_int2.h>
#include <dsc_png/dsc_png.h>
#include <dsc_render/draw_system.h>
#include <dsc_render_resource/shader_resource.h>

std::shared_ptr<DscRenderResource::ShaderResource> DscRenderResourcePng::MakeShaderResource(
    DscCommon::FileSystem& in_file_system,
    DscRender::DrawSystem& in_draw_system,
    const std::string& in_file_path
)
{
    std::vector<uint8> data = {};
    int32 byte_per_pixel = 0;
    DscCommon::VectorInt2 size = {};

    DscPng::LoadPng(
        data,
        byte_per_pixel,
        size,
        in_file_system,
        in_file_path
    );
    DscPng::ForceRgba(
        data,
        byte_per_pixel,
        size
    );
    std::shared_ptr<DscRenderResource::ShaderResource> result = {};
    if (0 < data.size())
    {
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
            (UINT64)size.GetX(), //UINT64 Width;
            (UINT)size.GetY(), //UINT Height;
            1, //UINT16 DepthOrArraySize;
            1, //UINT16 MipLevels;
            DXGI_FORMAT_R8G8B8A8_UNORM, //DXGI_FORMAT Format;
            DXGI_SAMPLE_DESC{ 1, 0 }, //DXGI_SAMPLE_DESC SampleDesc;
            D3D12_TEXTURE_LAYOUT_UNKNOWN, //D3D12_TEXTURE_LAYOUT Layout;
            D3D12_RESOURCE_FLAG_NONE //D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE //D3D12_RESOURCE_FLAGS Flags;
        };
        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
        shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        shader_resource_view_desc.Format = desc.Format;
        shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        shader_resource_view_desc.Texture2D.MipLevels = 1;

        result = std::make_shared<DscRenderResource::ShaderResource>(
            &in_draw_system,
            in_draw_system.MakeHeapWrapperCbvSrvUav(), //const std::shared_ptr < HeapWrapperItem >&in_shader_resource,
            desc, //const D3D12_RESOURCE_DESC & in_desc,
            shader_resource_view_desc, //const D3D12_SHADER_RESOURCE_VIEW_DESC & in_shader_resource_view_desc,
            data //const std::vector<uint8_t>&in_data
            );
    }
    return result;
}

