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

// --- Helper para lectura de atributos (Corregido para StringBuf de Cemu) ---
static void _emitReadAttribute(StringBuf* src, uint32 inputIndex)
{
	// Usamos {} que es el estándar de StringBuf en Cemu para evitar errores de formato
	src->addFmt("attrDecoder = attrDataSem{};" _CRLF, inputIndex);
}

// FUNCIÓN CORREGIDA PARA EL ERROR [233/528]
void LatteDecompiler_emitAttributeDecodeGLSL(LatteDecompilerShader* shaderContext, StringBuf* src, LatteParsedFetchShaderAttribute_t* attrib)
{
	uint32 attributeInputIndex = attrib->attributeBufferIndex;

	// 1. Reset de la variable temporal (u para unsigned, vital en Adreno/Xiaomi)
	src->add("attrDecoder = uvec4(0u);" _CRLF);

	// 2. Lógica de decodificación (Formatos de texturas/skins)
	if (attrib->format == FMT_32_32_32_32 && attrib->nfa == 0)
	{
		_emitReadAttribute(src, attributeInputIndex);
	}
	else if (attrib->format == FMT_32_32_32 && attrib->nfa == 0)
	{
		src->addFmt("attrDecoder = uvec4(attrDataSem{}.xyz, 0u);" _CRLF, attributeInputIndex);
	}
	else if (attrib->format == FMT_32_32 && attrib->nfa == 0)
	{
		src->addFmt("attrDecoder = uvec4(attrDataSem{}.xy, 0u, 0u);" _CRLF, attributeInputIndex);
	}
	else if (attrib->format == FMT_8_8_8_8 && attrib->nfa == 0)
	{
		_emitReadAttribute(src, attributeInputIndex);
		if (attrib->isSigned == 0)
			src->add("attrDecoder = floatBitsToUint(vec4(attrDecoder) / 255.0);" _CRLF);
		else
			src->add("attrDecoder = floatBitsToUint(max(vec4(ivec4(attrDecoder)) / 127.0, -1.0));" _CRLF);
	}
	else
	{
		_emitReadAttribute(src, attributeInputIndex);
	}

	// 3. Asignación final al registro rX (Donde X es el número del registro)
	src->addFmt("r{} = attrDecoder;" _CRLF, attrib->destReg);
}
