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
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xyz,0);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xy,0,0);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.x,0,0,0);" _CRLF, attributeInputIndex);
}

void _readBigEndianAttributeU32x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, attributeInputIndex);
	src->add("attrDecoder = (attrDecoder << 24) | ((attrDecoder << 8) & 0x00FF0000) | ((attrDecoder >> 8) & 0x0000FF00) | (attrDecoder >> 24);" _CRLF);
}

void _readBigEndianAttributeU32x3(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xyz = attrDataSem{}.xyz;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.xyz = (attrDecoder.xyz << 24) | ((attrDecoder.xyz << 8) & 0x00FF0000) | ((attrDecoder.xyz >> 8) & 0x0000FF00) | (attrDecoder.xyz >> 24);" _CRLF);
	src->add("attrDecoder.w = 0;" _CRLF);
}

void _readBigEndianAttributeU32x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.xy = (attrDecoder.xy << 24) | ((attrDecoder.xy << 8) & 0x00FF0000) | ((attrDecoder.xy >> 8) & 0x0000FF00) | (attrDecoder.xy >> 24);" _CRLF);
	src->add("attrDecoder.zw = uvec2(0);" _CRLF);
}

void _readBigEndianAttributeU32x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.x = attrDataSem{}.x;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = (attrDecoder.x << 24) | ((attrDecoder.x << 8) & 0x00FF0000) | ((attrDecoder.x >> 8) & 0x0000FF00) | (attrDecoder.x >> 24);" _CRLF);
	src->add("attrDecoder.yzw = uvec3(0);" _CRLF);
}

void _readBigEndianAttributeU16x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, attributeInputIndex);
	src->add("attrDecoder = ((attrDecoder << 8) & 0xFF00FF00) | ((attrDecoder >> 8) & 0x00FF00FF);" _CRLF);
}

void _readBigEndianAttributeU16x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.xy = ((attrDecoder.xy << 8) & 0xFF00FF00) | ((attrDecoder.xy >> 8) & 0x00FF00FF);" _CRLF);
	src->add("attrDecoder.zw = uvec2(0);" _CRLF);
}

void _readBigEndianAttributeU16x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.x = attrDataSem{}.x;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = ((attrDecoder.x << 8) & 0xFF00FF00) | ((attrDecoder.x >> 8) & 0x00FF00FF);" _CRLF);
	src->add("attrDecoder.yzw = uvec3(0);" _CRLF);
}

void LatteDecompiler_emitAttributeDecodeGLSL(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	const FetchShaderAttrib* attrib = &shaderContext->fetchShader->attribs[attributeInputIndex];
	using namespace LatteConst;

	if (attrib->endian == ENDIAN_LITTLE)
	{
		if (attrib->format == FMT_32_32_32_32 && attrib->nfa == 4)
			_readLittleEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32_32_32 && attrib->nfa == 3)
			_readLittleEndianAttributeU32x3(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32_32 && attrib->nfa == 2)
			_readLittleEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32 && attrib->nfa == 1)
			_readLittleEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
		else
		{
			// CORRECCIÓN: Bypass de seguridad
			cemu_assert_debug("Little endian format bypass");
		}
	}
	else if (attrib->endian == ENDIAN_BIG || attrib->endian == ENDIAN_8IN32)
	{
		if (attrib->format == FMT_32_32_32_32 && attrib->nfa == 4)
			_readBigEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32_32_32 && attrib->nfa == 3)
			_readBigEndianAttributeU32x3(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32_32 && attrib->nfa == 2)
			_readBigEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_32 && attrib->nfa == 1)
			_readBigEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
		else if (attrib->format == FMT_16_16_16_16 && attrib->nfa == 4 && attrib->isSigned == 0)
		{
			_readBigEndianAttributeU16x4(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xy = floatBitsToUint(vec2(float(attrDecoder.x & 0xFFFF), float(attrDecoder.x >> 16)));" _CRLF);
			src->add("attrDecoder.zw = floatBitsToUint(vec2(float(attrDecoder.y & 0xFFFF), float(attrDecoder.y >> 16)));" _CRLF);
		}
		else if (attrib->format == FMT_16_16_16_16 && attrib->nfa == 4 && attrib->isSigned != 0)
		{
			_readBigEndianAttributeU16x4(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xy = floatBitsToUint(vec2(float(int(attrDecoder.x << 16) >> 16), float(int(attrDecoder.x) >> 16)));" _CRLF);
			src->add("attrDecoder.zw = floatBitsToUint(vec2(float(int(attrDecoder.y << 16) >> 16), float(int(attrDecoder.y) >> 16)));" _CRLF);
		}
		else if (attrib->format == FMT_16_16 && attrib->nfa == 2 && attrib->isSigned == 0)
		{
			_readBigEndianAttributeU16x2(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xy = floatBitsToUint(vec2(float(attrDecoder.x & 0xFFFF), float(attrDecoder.x >> 16)));" _CRLF);
			src->add("attrDecoder.zw = uvec2(0);" _CRLF);
		}
		else if (attrib->format == FMT_16_16 && attrib->nfa == 2 && attrib->isSigned != 0)
		{
			_readBigEndianAttributeU16x2(shaderContext, src, attributeInputIndex);
			src->add("if( (attrDecoder.x&0x8000) != 0 ) attrDecoder.x |= 0xFFFF0000;" _CRLF);
			src->add("if( (attrDecoder.y&0x8000) != 0 ) attrDecoder.y |= 0xFFFF0000;" _CRLF);
			src->add("attrDecoder.xy = floatBitsToUint(vec2(float(int(attrDecoder.x)), float(int(attrDecoder.y))));" _CRLF);
			src->add("attrDecoder.zw = uvec2(0);" _CRLF);
		}
		else if (attrib->format == FMT_16 && attrib->nfa == 1 && attrib->isSigned == 0)
		{
			_readBigEndianAttributeU16x1(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.x = floatBitsToUint(float(attrDecoder.x & 0xFFFF));" _CRLF);
			src->add("attrDecoder.yzw = uvec3(0);" _CRLF);
		}
		else
		{
			// CORRECCIÓN: Bypass de seguridad
			cemu_assert_debug("Big endian format bypass");
		}
	}
}
