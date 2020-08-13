/* Copyright (c) 2017-2020 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "texture_decoder.hpp"
#include "global_managers.hpp"
#include "logging.hpp"

namespace Granite
{
static VkFormat compressed_format_to_decoded_format(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
		return VK_FORMAT_R8G8B8A8_SRGB;

	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case VK_FORMAT_BC4_UNORM_BLOCK:
		return VK_FORMAT_R8_UNORM;
	case VK_FORMAT_BC5_UNORM_BLOCK:
		return VK_FORMAT_R8G8_UNORM;

	case VK_FORMAT_BC4_SNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
	case VK_FORMAT_EAC_R11_SNORM_BLOCK:
		LOGE("SNORM formats are not supported.\n");
		return VK_FORMAT_UNDEFINED;

	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		return VK_FORMAT_R16G16B16A16_SFLOAT;

	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		return VK_FORMAT_R8G8B8A8_SRGB;

	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		return VK_FORMAT_R16_SFLOAT;
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		return VK_FORMAT_R16G16_SFLOAT;

	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
		return VK_FORMAT_R8G8B8A8_SRGB;

	case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;

	default:
		return VK_FORMAT_UNDEFINED;
	}
}

static VkFormat compressed_format_to_payload_format(VkFormat format)
{
	auto block_size = Vulkan::TextureFormatLayout::format_block_size(format, VK_IMAGE_ASPECT_COLOR_BIT);

	if (block_size == 4)
		return VK_FORMAT_R32_UINT;
	else if (block_size == 8)
		return VK_FORMAT_R32G32_UINT;
	else if (block_size == 16)
		return VK_FORMAT_R32G32B32A32_UINT;
	else
		return VK_FORMAT_UNDEFINED;
}

static VkFormat to_storage_format(VkFormat format, VkFormat orig_format = VK_FORMAT_UNDEFINED)
{
	switch (format)
	{
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_R8G8B8A8_UNORM:
		if (orig_format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK ||
		    orig_format == VK_FORMAT_BC1_RGBA_SRGB_BLOCK ||
		    orig_format == VK_FORMAT_BC1_RGB_SRGB_BLOCK ||
		    orig_format == VK_FORMAT_BC1_RGB_UNORM_BLOCK ||
		    orig_format == VK_FORMAT_BC2_SRGB_BLOCK ||
		    orig_format == VK_FORMAT_BC2_UNORM_BLOCK ||
		    orig_format == VK_FORMAT_BC3_SRGB_BLOCK ||
		    orig_format == VK_FORMAT_BC3_UNORM_BLOCK)
			return VK_FORMAT_R8G8B8A8_UNORM;
		else
			return VK_FORMAT_R8G8B8A8_UINT;

	case VK_FORMAT_R8_UNORM:
		if (orig_format == VK_FORMAT_BC4_UNORM_BLOCK)
			return VK_FORMAT_R8_UNORM;
		else
			return VK_FORMAT_R8_UINT;

	case VK_FORMAT_R8G8_UNORM:
		if (orig_format == VK_FORMAT_BC5_UNORM_BLOCK)
			return VK_FORMAT_R8G8_UNORM;
		else
			return VK_FORMAT_R8G8_UINT;

	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_SFLOAT:
		return format;

	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return VK_FORMAT_R16G16B16A16_UINT;

	default:
		return VK_FORMAT_UNDEFINED;
	}
}

struct ASTCQuantizationMode
{
	uint8_t bits, trits, quints;
};

static void build_astc_unquant_weight_lut(uint8_t *lut, size_t range, const ASTCQuantizationMode &mode)
{
	for (size_t i = 0; i < range; i++)
	{
		auto &v = lut[i];

		if (!mode.quints && !mode.trits)
		{
			switch (mode.bits)
			{
			case 1:
				v = i * 63;
				break;

			case 2:
				v = i * 0x15;
				break;

			case 3:
				v = i * 9;
				break;

			case 4:
				v = (i << 2) | (i >> 2);
				break;

			case 5:
				v = (i << 1) | (i >> 4);
				break;

			default:
				v = 0;
				break;
			}
		}
		else if (mode.bits == 0)
		{
			if (mode.trits)
				v = 32 * i;
			else
				v = 16 * i;
		}
		else
		{
			unsigned b = (i >> 1) & 1;
			unsigned c = (i >> 2) & 1;
			unsigned A, B, C, D;

			A = 0x7f * (i & 1);
			D = i >> mode.bits;
			B = 0;

			if (mode.trits)
			{
				static const unsigned Cs[3] = { 50, 23, 11 };
				C = Cs[mode.bits - 1];
				if (mode.bits == 2)
					B = 0x45 * b;
				else if (mode.bits == 3)
					B = 0x21 * b + 0x42 * c;
			}
			else
			{
				static const unsigned Cs[2] = { 28, 13 };
				C = Cs[mode.bits - 1];
				if (mode.bits == 2)
					B = 0x42 * b;
			}

			unsigned unq = D * C + B;
			unq ^= A;
			unq = (A & 0x20) | (unq >> 2);
		}

		// Expand [0, 63] to [0, 64].
		if (mode.bits != 0 && v > 32)
			v++;
	}
}

static void build_astc_unquant_endpoint_lut(uint8_t *lut, size_t range, const ASTCQuantizationMode &mode)
{
	for (size_t i = 0; i < range; i++)
	{
		auto &v = lut[i];

		if (!mode.quints && !mode.trits)
		{
			// Bit-replication.
			switch (mode.bits)
			{
			case 1:
				v = i * 0xff;
				break;

			case 2:
				v = i * 0x55;
				break;

			case 3:
				v = (i << 5) | (i << 2) | (i >> 1);
				break;

			case 4:
				v = i * 0x11;
				break;

			case 5:
				v = (i << 3) | (i >> 2);
				break;

			case 6:
				v = (i << 2) | (i >> 4);
				break;

			case 7:
				v = (i << 1) | (i >> 6);
				break;

			default:
				v = i;
				break;
			}
		}
		else
		{
			unsigned A, B, C, D;
			unsigned b = (i >> 1) & 1;
			unsigned c = (i >> 2) & 1;
			unsigned d = (i >> 3) & 1;
			unsigned e = (i >> 4) & 1;
			unsigned f = (i >> 5) & 1;

			B = 0;
			D = i >> mode.bits;
			A = (i & 1) * 0x1ff;

			if (mode.trits)
			{
				static const unsigned Cs[6] = { 204, 93, 44, 22, 11, 5 };
				C = Cs[mode.bits - 1];

				switch (mode.bits)
				{
				case 2:
					B = b * 0x116;
					break;

				case 3:
					B = b * 0x85 + c * 0x10a;
					break;

				case 4:
					B = b * 0x41 + c * 0x82 + d * 0x104;
					break;

				case 5:
					B = b * 0x20 + c * 0x40 + d * 0x81 + e * 0x102;
					break;

				case 6:
					B = b * 0x10 + c * 0x20 + d * 0x40 + e * 0x80 + f * 0x101;
					break;
				}
			}
			else
			{
				static const unsigned Cs[5] = { 113, 54, 26, 13, 6 };
				C = Cs[mode.bits - 1];

				switch (mode.bits)
				{
				case 2:
					B = b * 0x10c;
					break;

				case 3:
					B = b * 0x82 + c * 0x105;
					break;

				case 4:
					B = b * 0x40 + c * 0x81 + d * 0x102;
					break;

				case 5:
					B = b * 0x20 + c * 0x40 + d * 0x80 + e * 0x101;
					break;
				}
			}

			unsigned unq = D * C + B;
			unq ^= A;
			unq = (A & 0x80) | (unq >> 2);
			v = uint8_t(unq);
		}
	}
}

static unsigned astc_value_range(const ASTCQuantizationMode &mode)
{
	unsigned value_range = 1u << mode.bits;
	if (mode.trits)
		value_range *= 3;
	if (mode.quints)
		value_range *= 5;
	return value_range;
}

static void setup_astc_lut_color_endpoint(Vulkan::CommandBuffer &cmd)
{
	// In order to decode color endpoints, we need to convert available bits and number of values
	// into a format of (bits, trits, quints). A simple LUT texture is a reasonable approach for this.
	// Decoders are expected to have some form of LUT to deal with this ...
	static const ASTCQuantizationMode potential_modes[] = {
		{ 8, 0, 0 },
		{ 6, 1, 0 },
		{ 5, 0, 1 },
		{ 7, 0, 0 },
		{ 5, 1, 0 },
		{ 4, 0, 1 },
		{ 6, 0, 0 },
		{ 4, 1, 0 },
		{ 3, 0, 1 },
		{ 5, 0, 0 },
		{ 3, 1, 0 },
		{ 2, 0, 1 },
		{ 4, 0, 0 },
		{ 2, 1, 0 },
		{ 1, 0, 1 },
		{ 3, 0, 0 },
		{ 1, 1, 0 },
		// Are these valid?
		{ 2, 0, 0 },
		{ 1, 0, 0 },
	};

	constexpr size_t num_modes = sizeof(potential_modes) / sizeof(potential_modes[0]);
	uint8_t unquant_lut_offsets[num_modes];
	size_t unquant_offset = 0;

	uint8_t unquant_lut[2048];

	for (size_t i = 0; i < num_modes; i++)
	{
		auto value_range = astc_value_range(potential_modes[i]);
		unquant_lut_offsets[i] = unquant_offset;
		build_astc_unquant_endpoint_lut(unquant_lut + unquant_offset, value_range, potential_modes[i]);
		unquant_offset += value_range;
	}

	uint16_t lut[16][128][4];

	// We can have a maximum of 4 partitions and 4 component pairs.
	for (unsigned pairs = 0; pairs < 16; pairs++)
	{
		for (unsigned remaining = 0; remaining < 128; remaining++)
		{
			bool found_mode = false;
			for (auto &mode : potential_modes)
			{
				unsigned num_values = pairs * 2;
				unsigned total_bits = mode.bits * num_values + mode.quints * 7 * ((num_values + 2) / 3) +
				                      mode.trits * 8 * ((num_values + 4) / 5);

				if (total_bits <= remaining)
				{
					found_mode = true;
					lut[pairs][remaining][0] = mode.bits;
					lut[pairs][remaining][1] = mode.trits;
					lut[pairs][remaining][2] = mode.quints;
					lut[pairs][remaining][3] = unquant_lut_offsets[&mode - potential_modes];
					break;
				}
			}

			if (!found_mode)
				memset(lut[pairs][remaining], 0, sizeof(lut[pairs][remaining]));
		}
	}

	{
		Vulkan::BufferCreateInfo info = {};
		info.size = sizeof(lut);
		info.domain = Vulkan::BufferDomain::Device;
		info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		auto lut_buffer = cmd.get_device().create_buffer(info, lut);

		Vulkan::BufferViewCreateInfo view_info = {};
		view_info.buffer = lut_buffer.get();
		view_info.format = VK_FORMAT_R16G16B16A16_UINT;
		view_info.range = sizeof(lut);
		auto lut_view = cmd.get_device().create_buffer_view(view_info);
		cmd.set_buffer_view(1, 0, *lut_view);
	}

	{
		Vulkan::BufferCreateInfo info = {};
		info.size = unquant_offset;
		info.domain = Vulkan::BufferDomain::Device;
		info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		auto unquant_buffer = cmd.get_device().create_buffer(info, unquant_lut);

		Vulkan::BufferViewCreateInfo view_info = {};
		view_info.buffer = unquant_buffer.get();
		view_info.format = VK_FORMAT_R8_UINT;
		view_info.range = unquant_offset;

		auto unquant_view = cmd.get_device().create_buffer_view(view_info);
		cmd.set_buffer_view(1, 1, *unquant_view);
	}
}

static void setup_astc_lut_weights(Vulkan::CommandBuffer &cmd)
{
	static const ASTCQuantizationMode weight_modes[] = {
		{ 0, 0, 0 }, // Invalid
		{ 0, 0, 0 }, // Invalid
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 2, 0, 0 },
		{ 0, 0, 1 },
		{ 1, 1, 0 },
		{ 3, 0, 0 },
		{ 0, 0, 0 }, // Invalid
		{ 0, 0, 0 }, // Invalid
		{ 1, 0, 1 },
		{ 2, 1, 0 },
		{ 4, 0, 0 },
		{ 2, 0, 1 },
		{ 3, 1, 0 },
		{ 5, 0, 0 },
	};

	constexpr size_t num_modes = sizeof(weight_modes) / sizeof(weight_modes[0]);
	size_t unquant_offset = 0;
	uint8_t unquant_lut[2048];

	uint8_t lut[num_modes][4];

	for (size_t i = 0; i < num_modes; i++)
	{
		auto value_range = astc_value_range(weight_modes[i]);
		lut[i][0] = weight_modes[i].bits;
		lut[i][1] = weight_modes[i].trits;
		lut[i][2] = weight_modes[i].quints;
		lut[i][3] = unquant_offset;
		build_astc_unquant_weight_lut(unquant_lut + unquant_offset, value_range, weight_modes[i]);
		unquant_offset += value_range;
	}

	assert(unquant_offset <= 256);

	{
		Vulkan::BufferCreateInfo info = {};
		info.size = sizeof(lut);
		info.domain = Vulkan::BufferDomain::Device;
		info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		auto lut_buffer = cmd.get_device().create_buffer(info, lut);

		Vulkan::BufferViewCreateInfo view_info = {};
		view_info.buffer = lut_buffer.get();
		view_info.format = VK_FORMAT_R8G8B8A8_UINT;
		view_info.range = sizeof(lut);
		auto lut_view = cmd.get_device().create_buffer_view(view_info);
		cmd.set_buffer_view(1, 2, *lut_view);
	}

	{
		Vulkan::BufferCreateInfo info = {};
		info.size = unquant_offset;
		info.domain = Vulkan::BufferDomain::Device;
		info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		auto unquant_buffer = cmd.get_device().create_buffer(info, unquant_lut);

		Vulkan::BufferViewCreateInfo view_info = {};
		view_info.buffer = unquant_buffer.get();
		view_info.format = VK_FORMAT_R8_UINT;
		view_info.range = unquant_offset;

		auto unquant_view = cmd.get_device().create_buffer_view(view_info);
		cmd.set_buffer_view(1, 3, *unquant_view);
	}
}

static void setup_astc_luts(Vulkan::CommandBuffer &cmd)
{
	setup_astc_lut_color_endpoint(cmd);
	setup_astc_lut_weights(cmd);
}

static bool set_compute_decoder(Vulkan::CommandBuffer &cmd, VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
		cmd.set_program("builtin://shaders/decode/s3tc.comp");
		break;

	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
		cmd.set_program("builtin://shaders/decode/rgtc.comp");
		break;

	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		cmd.set_program("builtin://shaders/decode/etc2.comp");
		break;

	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		cmd.set_program("builtin://shaders/decode/eac.comp");
		break;

	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		cmd.set_program("builtin://shaders/decode/bc6.comp");
		break;

	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
		cmd.set_program("builtin://shaders/decode/bc7.comp");
		break;

	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
	case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
	case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
		setup_astc_luts(cmd);
		cmd.set_program("builtin://shaders/decode/astc.comp");
		break;

	default:
		return false;
	}

	return true;
}

static void dispatch_kernel_eac(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat format)
{
	struct Push
	{
		uint32_t width, height;
	} push;

	push.width = width;
	push.height = height;
	cmd.push_constants(&push, 0, sizeof(push));

	cmd.set_specialization_constant_mask(1);
	cmd.set_specialization_constant(0, uint32_t(format == VK_FORMAT_EAC_R11G11_UNORM_BLOCK ? 2 : 1));

	width = (width + 7) / 8;
	height = (height + 7) / 8;
	cmd.dispatch(width, height, 1);
}

static void dispatch_kernel_bc6(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat format)
{
	struct Push
	{
		uint32_t width, height;
	} push;

	push.width = width;
	push.height = height;
	cmd.push_constants(&push, 0, sizeof(push));

	cmd.set_specialization_constant_mask(1);
	cmd.set_specialization_constant(0, uint32_t(format == VK_FORMAT_BC6H_SFLOAT_BLOCK));

	width = (width + 7) / 8;
	height = (height + 7) / 8;
	cmd.dispatch(width, height, 1);
}

static void dispatch_kernel_bc7(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat)
{
	struct Push
	{
		uint32_t width, height;
	} push;

	push.width = width;
	push.height = height;
	cmd.push_constants(&push, 0, sizeof(push));

	width = (width + 7) / 8;
	height = (height + 7) / 8;
	cmd.dispatch(width, height, 1);
}

static void dispatch_kernel_etc2(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat format)
{
	struct Push
	{
		uint32_t width, height;
	} push;

	push.width = width;
	push.height = height;
	cmd.push_constants(&push, 0, sizeof(push));

	cmd.set_specialization_constant_mask(1);
	switch (format)
	{
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		cmd.set_specialization_constant(0, uint32_t(0));
		break;

	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
		cmd.set_specialization_constant(0, uint32_t(1));
		break;

	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		cmd.set_specialization_constant(0, uint32_t(8));
		break;

	default:
		break;
	}

	width = (width + 7) / 8;
	height = (height + 7) / 8;
	cmd.dispatch(width, height, 1);
}

static void dispatch_kernel_rgtc(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat format)
{
	struct Push
	{
		uint32_t width, height;
	} push;

	push.width = width;
	push.height = height;
	cmd.push_constants(&push, 0, sizeof(push));

	cmd.set_specialization_constant_mask(1);
	cmd.set_specialization_constant(0, uint32_t(format == VK_FORMAT_BC5_UNORM_BLOCK));

	width = (width + 7) / 8;
	height = (height + 7) / 8;
	cmd.dispatch(width, height, 1);
}

static void dispatch_kernel_s3tc(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat format)
{
	struct Push
	{
		uint32_t width, height;
	} push;

	push.width = width;
	push.height = height;
	cmd.push_constants(&push, 0, sizeof(push));

	cmd.set_specialization_constant_mask(3);

	switch (format)
	{
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		cmd.set_specialization_constant(0, uint32_t(0));
		break;

	default:
		cmd.set_specialization_constant(0, uint32_t(1));
		break;
	}

	switch (format)
	{
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		cmd.set_specialization_constant(1, uint32_t(1));
		break;

	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
		cmd.set_specialization_constant(1, uint32_t(2));
		break;

	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
		cmd.set_specialization_constant(1, uint32_t(3));
		break;

	default:
		break;
	}

	width = (width + 7) / 8;
	height = (height + 7) / 8;
	cmd.dispatch(width, height, 1);
}

static void dispatch_kernel(Vulkan::CommandBuffer &cmd, uint32_t width, uint32_t height, VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
		dispatch_kernel_s3tc(cmd, width, height, format);
		break;

	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
		dispatch_kernel_rgtc(cmd, width, height, format);
		break;

	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		dispatch_kernel_etc2(cmd, width, height, format);
		break;

	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		dispatch_kernel_eac(cmd, width, height, format);
		break;

	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		dispatch_kernel_bc6(cmd, width, height, format);
		break;

	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
		dispatch_kernel_bc7(cmd, width, height, format);
		break;

	default:
		break;
	}
}

Vulkan::ImageHandle decode_compressed_image(Vulkan::CommandBuffer &cmd, const Vulkan::TextureFormatLayout &layout,
                                            const VkComponentMapping &swizzle)
{
	auto &device = cmd.get_device();

	// For EXTENDED_USAGE_BIT.
	if (!device.get_device_features().supports_maintenance_2)
		return {};

	uint32_t block_width, block_height;
	Vulkan::TextureFormatLayout::format_block_dim(layout.get_format(), block_width, block_height);
	if (block_width == 1 || block_height == 1)
	{
		LOGE("Not a compressed format.\n");
		return {};
	}

	auto image_info = Vulkan::ImageCreateInfo::immutable_image(layout);
	image_info.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.format = compressed_format_to_decoded_format(layout.get_format());
	image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
	                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	image_info.flags = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
	image_info.swizzle = swizzle;
	if (image_info.format == VK_FORMAT_UNDEFINED)
		return {};
	auto decoded_image = device.create_image(image_info);

	auto staging = device.create_image_staging_buffer(layout);
	for (auto &blit : staging.blits)
	{
		blit.bufferRowLength = (blit.bufferRowLength + block_width - 1) / block_width;
		blit.bufferImageHeight = (blit.bufferImageHeight + block_height - 1) / block_height;
		blit.imageExtent.width = (blit.imageExtent.width + block_width - 1) / block_width;
		blit.imageExtent.height = (blit.imageExtent.height + block_height - 1) / block_height;
	}

	image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.format = compressed_format_to_payload_format(layout.get_format());
	if (image_info.format == VK_FORMAT_UNDEFINED)
		return {};
	image_info.swizzle = { VK_COMPONENT_SWIZZLE_R,
		                   VK_COMPONENT_SWIZZLE_G,
		                   VK_COMPONENT_SWIZZLE_B,
		                   VK_COMPONENT_SWIZZLE_A };
	image_info.misc = Vulkan::IMAGE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT |
	                  Vulkan::IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_GRAPHICS_BIT |
	                  Vulkan::IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT;
	image_info.width = (image_info.width + block_width - 1) / block_width;
	image_info.height = (image_info.height + block_height - 1) / block_height;
	image_info.initial_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	auto uploaded_image = device.create_image_from_staging_buffer(image_info, &staging);

	Vulkan::ImageViewCreateInfo view_info;
	view_info.image = decoded_image.get();
	view_info.view_type = VK_IMAGE_VIEW_TYPE_2D;
	view_info.levels = 1;
	view_info.layers = 1;
	view_info.format = to_storage_format(compressed_format_to_decoded_format(layout.get_format()), layout.get_format());

	Vulkan::ImageViewCreateInfo input_view_info;
	input_view_info.image = uploaded_image.get();
	input_view_info.view_type = VK_IMAGE_VIEW_TYPE_2D;
	input_view_info.levels = 1;
	input_view_info.layers = 1;

	cmd.image_barrier(*decoded_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
	                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
	                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT);

	if (!set_compute_decoder(cmd, layout.get_format()))
	{
		LOGE("Failed to set the compute decoder.\n");
		return {};
	}

	for (unsigned level = 0; level < layout.get_levels(); level++)
	{
		uint32_t mip_width = layout.get_width(level);
		uint32_t mip_height = layout.get_height(level);

		for (unsigned layer = 0; layer < layout.get_layers(); layer++)
		{
			view_info.base_layer = input_view_info.base_layer = layer;
			view_info.base_level = input_view_info.base_level = level;
			auto storage_view = device.create_image_view(view_info);
			auto payload_view = device.create_image_view(input_view_info);

			cmd.set_storage_texture(0, 0, *storage_view);
			cmd.set_texture(0, 1, *payload_view);
			dispatch_kernel(cmd, mip_width, mip_height, layout.get_format());
		}
	}

	cmd.image_barrier(*decoded_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT,
	                  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_SHADER_READ_BIT);

	cmd.set_specialization_constant_mask(0);
	return decoded_image;
}
}