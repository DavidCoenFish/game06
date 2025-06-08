#include "glyph_atlas_texture.h"

#include "glyph_atlas_row.h"
#include <dsc_render\draw_system.h>
#include <dsc_render_resource\shader_resource_partial_upload.h>

DscText::GlyphAtlasTexture::GlyphAtlasTexture(DscRender::DrawSystem& drawSystem)
{
    D3D12_RESOURCE_DESC desc = {
        D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
        D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
        16, //UINT64 Width;
        16, //UINT Height;
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

	_texture = std::make_unique<DscRenderResource::ShaderResourcePartialUpload>(
        &drawSystem,
        drawSystem.MakeHeapWrapperCbvSrvUav(),
        desc,
        shader_resource_view_desc,
        std::vector<uint8_t>()
        );
}

DscText::GlyphAtlasTexture::~GlyphAtlasTexture()
{
	// nop
}

