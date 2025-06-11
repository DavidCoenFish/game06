#include "glyph_atlas_texture.h"

#include "glyph.h"
#include "glyph_atlas_row.h"
#include <dsc_render\draw_system.h>
#include <dsc_render_resource\shader_resource_partial_upload.h>

DscText::GlyphAtlasTexture::GlyphAtlasTexture(DscRender::DrawSystem& in_draw_system, const int32 in_texture_dimention)
    : _texture_dimention(in_texture_dimention)
    , _icon_lowest_pos_y(in_texture_dimention)
{
    // build d3dx12 texture
    {
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D, //D3D12_RESOURCE_DIMENSION Dimension;
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
            static_cast<UINT64>(_texture_dimention), //UINT64 Width;
            static_cast<UINT>(_texture_dimention), //UINT Height;
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
            &in_draw_system,
            in_draw_system.MakeHeapWrapperCbvSrvUav(),
            desc,
            shader_resource_view_desc,
            std::vector<uint8_t>()
            );
    }
}

DscText::GlyphAtlasTexture::~GlyphAtlasTexture()
{
	// nop
}

std::unique_ptr<DscText::Glyph> DscText::GlyphAtlasTexture::AddIcon(const int32 in_width, const int32 in_height, const uint8_t* const in_data_4b)
{
	DSC_ASSERT((int)in_width <= _texture_dimention, "invalid param");
	DSC_ASSERT((int)in_height <= _texture_dimention, "invalid param");

	if ((0 == in_width) || (0 == in_height))
	{
		return nullptr;
	}

	GlyphAtlasRow* found_row = nullptr;
	const int desired_height = in_height + (0 != (in_height & 0x03) ? 4 - (in_height & 0x03) : 0);
	for (auto iter = _array_icon_row.begin(); iter != _array_icon_row.end(); ++iter)
	{
		GlyphAtlasRow& row = **iter;
		if ((desired_height == row.GetHeight()) &&
			(row.GetTextureHighestX() + (int)in_width <= _texture_dimention))
		{
			if (row.GetTextureHighestX() + (int)in_width == _texture_dimention)
			{
				_array_glyph_row_full.push_back(std::move(*iter));
				_array_glyph_row.erase(iter);
			}

			found_row = &row;
			break;
		}
	}

	// make a new row
	if (nullptr == found_row)
	{
		// Check icon is not overlapping font
		const int new_max_pos_y = _icon_lowest_pos_y - in_height;
		if ((new_max_pos_y < _text_highest_pos_y[0]) ||
			(new_max_pos_y < _text_highest_pos_y[1]) ||
			(new_max_pos_y < _text_highest_pos_y[2]) ||
			(new_max_pos_y < _text_highest_pos_y[3]))
		{
			return nullptr;
		}

		_icon_lowest_pos_y = new_max_pos_y;

		auto temp = std::make_unique<GlyphAtlasRow>(-1, desired_height, _icon_lowest_pos_y);

		if ((int)in_width == _texture_dimention)
		{
			_array_icon_row_full.push_back(std::move(temp));
		}
		else
		{
			_array_icon_row.push_back(std::move(temp));
		}

		found_row = temp.get();
	}

	DscCommon::VectorFloat4 uv = DscCommon::VectorFloat4(
		(float)found_row->GetTextureHighestX() / (float)_texture_dimention,
		(float)(found_row->GetTexturePosY() + in_height) / (float)_texture_dimention,
		(float)(found_row->GetTextureHighestX() + in_width) / (float)_texture_dimention,
		(float)found_row->GetTexturePosY() / (float)_texture_dimention
	);

	// add cell to end of row
	auto cell = std::make_unique<Glyph>(
		DscCommon::VectorInt2(in_width, in_height),
		DscCommon::VectorInt2(),
		uv,
		0xffffffff
		);

	auto& dest_data = _texture->GetData(true, found_row->GetTexturePosY(), found_row->GetTexturePosY() + desired_height);
	for (int32 y = 0; y < in_height; ++y)
	{
		for (int32 x = 0; x < in_width; ++x)
		{
			const int dest_data_index = ((((found_row->GetTexturePosY() + y) * _texture_dimention) + (found_row->GetTextureHighestX() + x)) * 4);
			const int buffer_index = (((y * in_width) + x) * 4);
			for (int32 b = 0; b < 4; ++b)
			{
				dest_data[dest_data_index + b] = in_data_4b[buffer_index + b];
			}
		}
	}

	found_row->IncrementTextureHighestX(in_width);
	return cell;
}

std::unique_ptr<DscText::Glyph> DscText::GlyphAtlasTexture::AddGlyph(const int32 in_width, const int32 in_height, 
	const int32 in_bearing_x,
	const int32 in_bearing_y,
	const uint8_t* const in_data_1b)
{
	DSC_ASSERT((int)in_width <= _texture_dimention, "invalid param");
	DSC_ASSERT((int)in_height <= _texture_dimention, "invalid param");

	if ((0 == in_width) || (0 == in_height))
	{
		return nullptr;
	}

	GlyphAtlasRow* found_row = nullptr;
	const int desired_height = in_height + (0 != (in_height & 0x03) ? 4 - (in_height & 0x03) : 0);
	for (auto iter = _array_glyph_row.begin(); iter != _array_glyph_row.end(); ++iter)
	{
		GlyphAtlasRow& row = **iter;
		if ((desired_height == row.GetHeight()) &&
			(row.GetTextureHighestX() + (int)in_width <= _texture_dimention))
		{
			if (row.GetTextureHighestX() + (int)in_width == _texture_dimention)
			{
				_array_glyph_row_full.push_back(std::move(*iter));
				_array_glyph_row.erase(iter);
			}

			found_row = &row;
			break;
		}
	}

	// make a new row
	if (nullptr == found_row)
	{
		int mask_index = 0;
		if ((_text_highest_pos_y[0] <= _text_highest_pos_y[1]) &&
			(_text_highest_pos_y[0] <= _text_highest_pos_y[2]) &&
			(_text_highest_pos_y[0] <= _text_highest_pos_y[3]))
		{
			mask_index = 0;
		}
		else if ((_text_highest_pos_y[1] <= _text_highest_pos_y[2]) &&
			(_text_highest_pos_y[1] <= _text_highest_pos_y[3]))
		{
			mask_index = 1;
		}
		else if (_text_highest_pos_y[2] <= _text_highest_pos_y[3])
		{
			mask_index = 2;
		}
		else
		{
			mask_index = 3;
		}

		if (_icon_lowest_pos_y < _text_highest_pos_y[mask_index] + desired_height)
		{
			return nullptr;
		}

		auto temp = std::make_unique<GlyphAtlasRow>(mask_index, desired_height, _text_highest_pos_y[mask_index]);
		_text_highest_pos_y[mask_index] += desired_height;

		found_row = temp.get();

		if ((int)in_width == _texture_dimention)
		{
			_array_glyph_row_full.push_back(std::move(temp));
		}
		else
		{
			_array_glyph_row.push_back(std::move(temp));
		}
	}

	DscCommon::VectorFloat4 uv = DscCommon::VectorFloat4(
		(float)found_row->GetTextureHighestX() / (float)_texture_dimention,
		(float)(found_row->GetTexturePosY() + in_height) / (float)_texture_dimention,
		(float)(found_row->GetTextureHighestX() + in_width) / (float)_texture_dimention,
		(float)found_row->GetTexturePosY() / (float)_texture_dimention
	);

	const int mask = 0xff << (found_row->GetMaskIndex() * 8);

	// add cell to end of row
	auto cell = std::make_unique<Glyph>(
		DscCommon::VectorInt2(in_width, in_height),
		DscCommon::VectorInt2(in_bearing_x, in_bearing_y),
		uv,
		mask
		);

	auto& dest_data = _texture->GetData(true, found_row->GetTexturePosY(), found_row->GetTexturePosY() + desired_height);
	for (int32 y = 0; y < in_height; ++y)
	{
		for (int32 x = 0; x < in_width; ++x)
		{
			const int dest_data_index = ((((found_row->GetTexturePosY() + y) * _texture_dimention) + (found_row->GetTextureHighestX() + x)) * 4) + found_row->GetMaskIndex();
			const int buffer_index = ((y * in_width) + x);
			dest_data[dest_data_index] = in_data_1b[buffer_index];
		}
	}

	found_row->IncrementTextureHighestX(in_width);
	return cell;
}

void DscText::GlyphAtlasTexture::ClearAllIconUsage()
{
	_icon_lowest_pos_y = _texture_dimention;
	_array_icon_row.resize(0);
	_array_icon_row_full.resize(0);
}

void DscText::GlyphAtlasTexture::ClearAllGlyphUsage()
{
	_text_highest_pos_y[0] = 0;
	_text_highest_pos_y[1] = 0;
	_text_highest_pos_y[2] = 0;
	_text_highest_pos_y[3] = 0;
	_array_glyph_row.resize(0);
	_array_glyph_row_full.resize(0);
}

std::shared_ptr<DscRender::HeapWrapperItem> DscText::GlyphAtlasTexture::GetHeapWrapperItem() const
{
    if (nullptr != _texture)
    {
        return _texture->GetHeapWrapperItem();
    }
    return nullptr;
}

std::shared_ptr<DscRender::IResource> DscText::GlyphAtlasTexture::GetResource()
{
	return _texture;
}

void DscText::GlyphAtlasTexture::UploadTexture(
    DscRender::DrawSystem* const in_draw_system,
    ID3D12GraphicsCommandList* const in_command_list
    )
{
    if (nullptr != _texture)
    {
        return _texture->UploadDataIfDirty(in_draw_system, in_command_list);
    }
    return;
}
