#include "Cafe/HW/Latte/Core/LatteConst.h"
#include "Cafe/HW/Latte/Core/LatteShaderAssembly.h"
#include "Cafe/HW/Latte/ISA/RegDefines.h"
#include "Cafe/HW/Latte/Core/Latte.h"
#include "Cafe/HW/Latte/Core/LatteDraw.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.h"
#include "Cafe/HW/Latte/Core/FetchShader.h"
#include "Cafe/HW/Latte/Renderer/Renderer.h"
#include "util/helpers/StringBuf.h"

#define _CRLF	"\r\n"

void _readLittleEndianAttributeU32x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x3(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xyz, 0u);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xy, 0u, 0u);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.x, 0u, 0u, 0u);" _CRLF, attributeInputIndex);
}

void _readBigEndianAttributeU16x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.zw = attrDecoder.xy >> 16u;" _CRLF);
	src->add("attrDecoder.xy &= 0xFFFFu;" _CRLF);
	src->add("attrDecoder.x = ((attrDecoder.x & 0xFFu) << 8u) | (attrDecoder.x >> 8u);" _CRLF);
	src->add("attrDecoder.y = ((attrDecoder.y & 0xFFu) << 8u) | (attrDecoder.y >> 8u);" _CRLF);
	src->add("attrDecoder.z = ((attrDecoder.z & 0xFFu) << 8u) | (attrDecoder.z >> 8u);" _CRLF);
	src->add("attrDecoder.w = ((attrDecoder.w & 0xFFu) << 8u) | (attrDecoder.w >> 8u);" _CRLF);
}

void _readBigEndianAttributeU16x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.x = attrDataSem{}.x;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.y = attrDecoder.x >> 16u;" _CRLF);
	src->add("attrDecoder.x &= 0xFFFFu;" _CRLF);
	src->add("attrDecoder.x = ((attrDecoder.x & 0xFFu) << 8u) | (attrDecoder.x >> 8u);" _CRLF);
	src->add("attrDecoder.y = ((attrDecoder.y & 0xFFu) << 8u) | (attrDecoder.y >> 8u);" _CRLF);
}

void _readBigEndianAttributeU16x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.x = attrDataSem{}.x & 0xFFFFu;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = ((attrDecoder.x & 0xFFu) << 8u) | (attrDecoder.x >> 8u);" _CRLF);
}

void _readBigEndianAttributeU32x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = ((attrDecoder.x & 0xFFu) << 24u) | ((attrDecoder.x & 0xFF00u) << 8u) | ((attrDecoder.x & 0xFF0000u) >> 8u) | (attrDecoder.x >> 24u);" _CRLF);
	src->add("attrDecoder.y = ((attrDecoder.y & 0xFFu) << 24u) | ((attrDecoder.y & 0xFF00u) << 8u) | ((attrDecoder.y & 0xFF0000u) >> 8u) | (attrDecoder.y >> 24u);" _CRLF);
}

void _readBigEndianAttributeU32x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.x = attrDataSem{}.x;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = ((attrDecoder.x & 0xFFu) << 24u) | ((attrDecoder.x & 0xFF00u) << 8u) | ((attrDecoder.x & 0xFF0000u) >> 8u) | (attrDecoder.x >> 24u);" _CRLF);
}

void _emitAttributeDecoderGLSL(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex, const LatteFetchShaderAttribute* attrib)
{
	// GX2_ENDIAN_SWAP_DEFAULT (64 bit swap) is not supported for now (not used by any game anyway)
	if (attrib->endianSwap == GX2_ENDIAN_SWAP_8_IN_16 || attrib->endianSwap == GX2_ENDIAN_SWAP_8_IN_32)
	{
		if (attrib->format == FMT_16_16_16_16 && attrib->nfa == 4 && attrib->isSigned == 0)
		{
			_readBigEndianAttributeU16x4(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder = floatBitsToUint(vec4(float(attrDecoder.x), float(attrDecoder.y), float(attrDecoder.z), float(attrDecoder.w)));" _CRLF);
		}
		else if (attrib->format == FMT_16_16 && attrib->nfa == 2 && attrib->isSigned == 0)
		{
			_readBigEndianAttributeU16x2(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xy = floatBitsToUint(vec2(float(attrDecoder.x), float(attrDecoder.y)));" _CRLF);
			src->add("attrDecoder.zw = uvec2(0u);" _CRLF);
		}
		else if (attrib->format == FMT_16_16 && attrib->nfa == 2 && attrib->isSigned != 0)
		{
			_readBigEndianAttributeU16x2(shaderContext, src, attributeInputIndex);
			src->add("if( (attrDecoder.x & 0x8000u) != 0u ) attrDecoder.x |= 0xFFFF0000u;" _CRLF);
			src->add("if( (attrDecoder.y & 0x8000u) != 0u ) attrDecoder.y |= 0xFFFF0000u;" _CRLF);
			src->add("attrDecoder.xy = floatBitsToUint(vec2(float(int(attrDecoder.x)), float(int(attrDecoder.y))));" _CRLF);
			src->add("attrDecoder.zw = uvec2(0u);" _CRLF);
		}
		else if (attrib->format == FMT_16 && attrib->nfa == 1 && attrib->isSigned == 0)
		{
			_readBigEndianAttributeU16x1(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.x = floatBitsToUint(float(attrDecoder.x));" _CRLF);
			src->add("attrDecoder.yzw = uvec3(0u);" _CRLF);
		}
		else if (attrib->format == FMT_32_32 && attrib->nfa == 2)
		{
			_readBigEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.zw = uvec2(0u);" _CRLF);
		}
		else if (attrib->format == FMT_32 && attrib->nfa == 1)
		{
			_readBigEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.yzw = uvec3(0u);" _CRLF);
		}
		else
		{
			// Fallback
			_readLittleEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		}
	}
	else
	{
		if (attrib->format == FMT_32_32_32_32 && attrib->nfa == 4)
			_readLittleEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32_32_32 && attrib->nfa == 3)
			_readLittleEndianAttributeU32x3(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32_32 && attrib->nfa == 2)
			_readLittleEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32 && attrib->nfa == 1)
			_readLittleEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_10_10_10_2 && attrib->nfa == 4 && attrib->isSigned == 0)
		{
			src->addFmt("attrDecoder.x = (attrDataSem{}.x >> 0u) & 0x3FFu;" _CRLF, attributeInputIndex);
			src->addFmt("attrDecoder.y = (attrDataSem{}.x >> 10u) & 0x3FFu;" _CRLF, attributeInputIndex);
			src->addFmt("attrDecoder.z = (attrDataSem{}.x >> 20u) & 0x3FFu;" _CRLF, attributeInputIndex);
			src->addFmt("attrDecoder.w = (attrDataSem{}.x >> 30u) & 0x3u;" _CRLF, attributeInputIndex);
			src->add("attrDecoder = floatBitsToUint(vec4(float(attrDecoder.x), float(attrDecoder.y), float(attrDecoder.z), float(attrDecoder.w)));" _CRLF);
		}
		else if (attrib->format == FMT_8_8_8_8 && attrib->nfa == 4 && attrib->isSigned == 0)
		{
			src->addFmt("attrDecoder.x = (attrDataSem{}.x >> 0u) & 0xFFu;" _CRLF, attributeInputIndex);
			src->addFmt("attrDecoder.y = (attrDataSem{}.x >> 8u) & 0xFFu;" _CRLF, attributeInputIndex);
			src->addFmt("attrDecoder.z = (attrDataSem{}.x >> 16u) & 0xFFu;" _CRLF, attributeInputIndex);
			src->addFmt("attrDecoder.w = (attrDataSem{}.x >> 24u) & 0xFFu;" _CRLF, attributeInputIndex);
			src->add("attrDecoder = floatBitsToUint(vec4(float(attrDecoder.x), float(attrDecoder.y), float(attrDecoder.z), float(attrDecoder.w)));" _CRLF);
		}
		else
		{
			_readLittleEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		}
	}
}
