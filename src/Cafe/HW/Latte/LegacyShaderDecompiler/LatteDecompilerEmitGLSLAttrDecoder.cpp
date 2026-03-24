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

// --- Helpers de lectura ---
void _readLittleEndianAttributeU32x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x3(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xyz, 0);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xy, 0, 0);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU32x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.x, 0, 0, 0);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU16x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = uvec4(attrDataSem{}.xy, 0, 0);" _CRLF, attributeInputIndex);
}

void _readLittleEndianAttributeU16x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, attributeInputIndex);
}

void _readBigEndianAttributeU32x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, attributeInputIndex);
	src->add("attrDecoder = (attrDecoder>>24)|((attrDecoder>>8)&0xFF00u)|((attrDecoder<<8)&0xFF0000u)|((attrDecoder<<24));" _CRLF);
}

void _readBigEndianAttributeU32x3(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xyz = attrDataSem{}.xyz;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.xyz = (attrDecoder.xyz>>24)|((attrDecoder.xyz>>8)&0xFF00u)|((attrDecoder.xyz<<8)&0xFF0000u)|((attrDecoder.xyz<<24));" _CRLF);
	src->add("attrDecoder.w = 0u;" _CRLF);
}

void _readBigEndianAttributeU32x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.xy = (attrDecoder.xy>>24)|((attrDecoder.xy>>8)&0xFF00u)|((attrDecoder.xy<<8)&0xFF0000u)|((attrDecoder.xy<<24));" _CRLF);
	src->add("attrDecoder.zw = uvec2(0);" _CRLF);
}

void _readBigEndianAttributeU32x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.x = attrDataSem{}.x;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = (attrDecoder.x>>24)|((attrDecoder.x>>8)&0xFF00u)|((attrDecoder.x<<8)&0xFF0000u)|((attrDecoder.x<<24));" _CRLF);
	src->add("attrDecoder.yzw = uvec3(0);" _CRLF);
}

void _readBigEndianAttributeU16x1(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.x = ((attrDecoder.x>>8)&0xFFu)|((attrDecoder.x<<8)&0xFF00u);" _CRLF);
	src->add("attrDecoder.yzw = uvec3(0);" _CRLF);
}

void _readBigEndianAttributeU16x2(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xy = attrDataSem{}.xy;" _CRLF, attributeInputIndex);
	src->add("attrDecoder.xy = ((attrDecoder.xy>>8)&0xFFu)|((attrDecoder.xy<<8)&0xFF00u);" _CRLF);
	src->add("attrDecoder.zw = uvec2(0);" _CRLF);
}

void _readBigEndianAttributeU16x4(LatteDecompilerShader* shaderContext, StringBuf* src, uint32 attributeInputIndex)
{
	src->addFmt("attrDecoder.xyzw = attrDataSem{}.xyzw;" _CRLF, attributeInputIndex);
	src->add("attrDecoder = ((attrDecoder>>8)&0xFFu)|((attrDecoder<<8)&0xFF00u);" _CRLF);
}

void LatteDecompiler_emitAttributeDecodeGLSL(LatteDecompilerShader* shaderContext, StringBuf* src, LatteParsedFetchShaderAttribute_t* attrib)
{
	if (attrib->attributeBufferIndex >= Latte::GPU_LIMITS::NUM_VERTEX_BUFFERS)
	{
		src->add("attrDecoder = uvec4(0);" _CRLF);
		return;
	}

	uint32 attributeInputIndex = attrib->semanticId;

	// --- SWAP_U32 ---
	if( attrib->endianSwap == LatteConst::VertexFetchEndianMode::SWAP_U32 )
	{
		if( attrib->format == FMT_32_32_32_32_FLOAT && attrib->nfa == 2 )
		{
			_readBigEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		}
		else if( attrib->format == FMT_32_32_32_FLOAT && attrib->nfa == 2 )
		{
			_readBigEndianAttributeU32x3(shaderContext, src, attributeInputIndex);
		}
		else if( attrib->format == FMT_32_32_FLOAT && attrib->nfa == 2 )
		{
			_readBigEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
		}
		else if( attrib->format == FMT_32_FLOAT && attrib->nfa == 2 )
		{
			_readBigEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
		}
		else if( attrib->format == FMT_2_10_10_10 && attrib->nfa == 0 )
		{
			_readBigEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xyzw = uvec4((attrDecoder.x>>0)&0x3FFu,(attrDecoder.x>>10)&0x3FFu,(attrDecoder.x>>20)&0x3FFu,(attrDecoder.x>>30)&0x3u);" _CRLF);
			if (attrib->isSigned != 0)
			{
				src->add("if( (attrDecoder.x&0x200u) != 0u ) attrDecoder.x |= 0xFFFFFC00u;" _CRLF);
				src->add("if( (attrDecoder.y&0x200u) != 0u ) attrDecoder.y |= 0xFFFFFC00u;" _CRLF);
				src->add("if( (attrDecoder.z&0x200u) != 0u ) attrDecoder.z |= 0xFFFFFC00u;" _CRLF);
				src->add("attrDecoder.x = floatBitsToUint(max(float(int(attrDecoder.x))/511.0,-1.0));" _CRLF);
				src->add("attrDecoder.y = floatBitsToUint(max(float(int(attrDecoder.y))/511.0,-1.0));" _CRLF);
				src->add("attrDecoder.z = floatBitsToUint(max(float(int(attrDecoder.z))/511.0,-1.0));" _CRLF);
			}
			else
			{
				src->add("attrDecoder.x = floatBitsToUint(float(attrDecoder.x)/1023.0);" _CRLF);
				src->add("attrDecoder.y = floatBitsToUint(float(attrDecoder.y)/1023.0);" _CRLF);
				src->add("attrDecoder.z = floatBitsToUint(float(attrDecoder.z)/1023.0);" _CRLF);
			}
			src->add("attrDecoder.w = floatBitsToUint(float(attrDecoder.w)/3.0);" _CRLF);
		}
		else if( (attrib->format == FMT_32_32_32_32 || attrib->format == FMT_32_32_32 || attrib->format == FMT_32_32 || attrib->format == FMT_32) && attrib->nfa == 1 )
		{
			if (attrib->format == FMT_32_32_32_32) _readBigEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
			else if (attrib->format == FMT_32_32_32) _readBigEndianAttributeU32x3(shaderContext, src, attributeInputIndex);
			else if (attrib->format == FMT_32_32) _readBigEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
			else _readBigEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
		}
		else if( attrib->format == FMT_8_8_8_8 && attrib->nfa == 0 )
		{
			src->addFmt("attrDecoder.xyzw = attrDataSem{}.wzyx;" _CRLF, attributeInputIndex);
			if (attrib->isSigned != 0)
			{
				src->add("if( (attrDecoder.x&0x80u) != 0u ) attrDecoder.x |= 0xFFFFFF00u;" _CRLF); 
				src->add("if( (attrDecoder.y&0x80u) != 0u ) attrDecoder.y |= 0xFFFFFF00u;" _CRLF); 
				src->add("if( (attrDecoder.z&0x80u) != 0u ) attrDecoder.z |= 0xFFFFFF00u;" _CRLF); 
				src->add("if( (attrDecoder.w&0x80u) != 0u ) attrDecoder.w |= 0xFFFFFF00u;" _CRLF); 
				src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(max(vec4(ivec4(attrDecoder))/127.0,-1.0)));" _CRLF); 
			}
			else
			{
				src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(vec4(attrDecoder)/255.0));" _CRLF);
			}
		}
		else
		{
			src->add("// U32 Swap format not supported" _CRLF);
		}
	}
	// --- SWAP_NONE ---
	else if( attrib->endianSwap == LatteConst::VertexFetchEndianMode::SWAP_NONE )
	{
		if( attrib->format == FMT_32_32_32_32_FLOAT && attrib->nfa == 2 )
		{
			_readLittleEndianAttributeU32x4(shaderContext, src, attributeInputIndex);
		}
		else if (attrib->format == FMT_32_32_32_FLOAT && attrib->nfa == 2)
		{
			_readLittleEndianAttributeU32x3(shaderContext, src, attributeInputIndex);
		}
		else if (attrib->format == FMT_32_32_FLOAT && attrib->nfa == 2)
		{
			_readLittleEndianAttributeU32x2(shaderContext, src, attributeInputIndex);
		}
		else if (attrib->format == FMT_32 && attrib->nfa == 1 && attrib->isSigned == 0)
		{
			_readLittleEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
		}
		else if (attrib->format == FMT_2_10_10_10 && attrib->nfa == 0 && attrib->isSigned == 0)
		{
			_readLittleEndianAttributeU32x1(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xyzw = uvec4((attrDecoder.x>>0)&0x3FFu,(attrDecoder.x>>10)&0x3FFu,(attrDecoder.x>>20)&0x3FFu,(attrDecoder.x>>30)&0x3u);" _CRLF);
			src->add("attrDecoder.x = floatBitsToUint(max(float(int(attrDecoder.x))/1023.0,-1.0));" _CRLF);
			src->add("attrDecoder.y = floatBitsToUint(max(float(int(attrDecoder.y))/1023.0,-1.0));" _CRLF); // CORRECCIÓN: corregido 'attribDecoder' a 'attrDecoder'
			src->add("attrDecoder.z = floatBitsToUint(max(float(int(attrDecoder.z))/1023.0,-1.0));" _CRLF);
			src->add("attrDecoder.w = floatBitsToUint(float(attrDecoder.w)/3.0);" _CRLF);
		}
		else if (attrib->format == FMT_16_16_16_16 && (attrib->nfa == 0 || attrib->nfa == 2))
		{
			_readLittleEndianAttributeU16x4(shaderContext, src, attributeInputIndex);
			if (attrib->isSigned != 0)
			{
				src->add("if( (attrDecoder.x&0x8000u) != 0u ) attrDecoder.x |= 0xFFFF0000u;" _CRLF);
				src->add("if( (attrDecoder.y&0x8000u) != 0u ) attrDecoder.y |= 0xFFFF0000u;" _CRLF);
				src->add("if( (attrDecoder.z&0x8000u) != 0u ) attrDecoder.z |= 0xFFFF0000u;" _CRLF);
				src->add("if( (attrDecoder.w&0x8000u) != 0u ) attrDecoder.w |= 0xFFFF0000u;" _CRLF);
				if (attrib->nfa == 0)
					src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(max(vec4(ivec4(attrDecoder))/32767.0,-1.0)));" _CRLF);
				else
					src->add("attrDecoder.xyzw = floatBitsToUint(vec4(ivec4(attrDecoder)));" _CRLF);
			}
			else
			{
				src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(vec4(attrDecoder)/65535.0));" _CRLF);
			}
		}
		else if (attrib->format == FMT_16_16_16_16_FLOAT && attrib->nfa == 2)
		{
			_readLittleEndianAttributeU16x4(shaderContext, src, attributeInputIndex);
			src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(vec4(unpackHalf2x16(attrDecoder.x|(attrDecoder.y<<16)),unpackHalf2x16(attrDecoder.z|(attrDecoder.w<<16)))));" _CRLF);
		}
		else if (attrib->format == FMT_8_8_8_8 && attrib->nfa == 0)
		{
			src->addFmt("attrDecoder.xyzw = attrDataSem{}.xyzw;" _CRLF, attributeInputIndex);
			if (attrib->isSigned != 0)
			{
				src->add("if( (attrDecoder.x&0x80u) != 0u ) attrDecoder.x |= 0xFFFFFF00u;" _CRLF); 
				src->add("if( (attrDecoder.y&0x80u) != 0u ) attrDecoder.y |= 0xFFFFFF00u;" _CRLF); 
				src->add("if( (attrDecoder.z&0x80u) != 0u ) attrDecoder.z |= 0xFFFFFF00u;" _CRLF); 
				src->add("if( (attrDecoder.w&0x80u) != 0u ) attrDecoder.w |= 0xFFFFFF00u;" _CRLF); 
				src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(max(vec4(ivec4(attrDecoder))/127.0,-1.0)));" _CRLF); 
			}
			else
			{
				src->add("attrDecoder.xyzw = uvec4(floatBitsToUint(vec4(attrDecoder)/255.0));" _CRLF);
			}
		}
		else
		{
			src->add("// Endian None format not supported" _CRLF);
		}
	}
	else if( attrib->endianSwap == LatteConst::VertexFetchEndianMode::SWAP_U16 )
	{
		src->add("// U16 Swap not implemented" _CRLF);
	}
}
