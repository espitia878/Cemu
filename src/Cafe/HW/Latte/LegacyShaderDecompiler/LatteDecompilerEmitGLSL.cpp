#include "Cafe/HW/Latte/Core/LatteConst.h"
#include "Cafe/HW/Latte/Core/LatteShaderAssembly.h"
#include "Cafe/HW/Latte/ISA/RegDefines.h"
#include "Cafe/OS/libs/gx2/GX2.h"
#include "Cafe/HW/Latte/Core/Latte.h"
#include "Cafe/HW/Latte/Core/LatteDraw.h"
#include "Cafe/HW/Latte/Core/LatteShader.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerInternal.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerInstructions.h"
#include "Cafe/HW/Latte/Core/FetchShader.h"
#include "Cafe/HW/Latte/Renderer/Renderer.h"
#include "config/ActiveSettings.h"
#include "util/helpers/StringBuf.h"

#include <bitset>
#include <boost/container/small_vector.hpp>

#define _CRLF	"\r\n"

// --- ARREGLO SKINS ZOMBIES (XIAOMI 14T PRO) ---
#ifdef m_is_vulkan
#undef m_is_vulkan
#endif
#define m_is_vulkan (true)
// ----------------------------------------------

// local prototypes
static void _emitTypeConversionSuffix(LatteDecompilerShader* shaderContext, StringBuf* src, LATTE_DECOMPILER_DTYPE srcType, LATTE_DECOMPILER_DTYPE dstType);
void LatteDecompiler_emitClauseCode(LatteDecompilerShader* shaderContext, LatteDecompilerCFInstruction* cfInstruction, bool isSubroutine);
void LatteDecompiler_emitAttributeDecodeGLSL(LatteDecompilerShader* shaderContext, StringBuf* src, LatteParsedFetchShaderAttribute_t* attrib);

static void _emitTypeConversionSuffix(LatteDecompilerShader* shaderContext, StringBuf* src, LATTE_DECOMPILER_DTYPE srcType, LATTE_DECOMPILER_DTYPE dstType)
{
	if (srcType == dstType)
		return;

	if (dstType == LATTE_DECOMPILER_DTYPE_FLOAT)
	{
		if (srcType == LATTE_DECOMPILER_DTYPE_SIGNED_INT)
			src->insert(0, "intBitsToFloat("), src->add(")");
		else if (srcType == LATTE_DECOMPILER_DTYPE_UNSIGNED_INT)
			src->insert(0, "uintBitsToFloat("), src->add(")");
	}
	else if (dstType == LATTE_DECOMPILER_DTYPE_SIGNED_INT)
	{
		if (srcType == LATTE_DECOMPILER_DTYPE_FLOAT)
			src->insert(0, "floatBitsToInt("), src->add(")");
		else if (srcType == LATTE_DECOMPILER_DTYPE_UNSIGNED_INT)
			src->insert(0, "int("), src->add(")");
	}
	else if (dstType == LATTE_DECOMPILER_DTYPE_UNSIGNED_INT)
	{
		if (srcType == LATTE_DECOMPILER_DTYPE_FLOAT)
			src->insert(0, "floatBitsToUint("), src->add(")");
		else if (srcType == LATTE_DECOMPILER_DTYPE_SIGNED_INT)
			src->insert(0, "uint("), src->add(")");
	}
}

static const char* _getRegisterVarName(LatteDecompilerShader* shaderContext, uint32 regIndex)
{
	static char name[32];
	snprintf(name, 32, "r%u", regIndex);
	return name;
}

static const char* _getElementStrByIndex(uint32 index)
{
	static const char* elements[] = { "x", "y", "z", "w" };
	return elements[index];
}

void LatteDecompiler_emitClauseCode(LatteDecompilerShader* shaderContext, LatteDecompilerCFInstruction* cfInstruction, bool isSubroutine)
{
	// Usamos shaderContext->shaderCode porque 'src' no está definido aquí
	if (cfInstruction->op == CF_OP_ALU)
	{
		shaderContext->shaderCode.add("// ALU Clause" _CRLF);
	}
}

void LatteDecompiler_emitVertexShaderInputGLSL(LatteDecompilerShader* shaderContext, StringBuf* src)
{
	LatteShader* shader = shaderContext->shader;

	if (shader->shaderType == LatteConst::ShaderType::Vertex)
	{
		for (uint32 i = 0; i < 16; i++)
		{
			const auto& attrib = shader->vertexShader.attributes[i];
			if (attrib.attributeBufferIndex < 16)
			{
				LatteDecompiler_emitAttributeDecodeGLSL(shaderContext, src, (LatteParsedFetchShaderAttribute_t*)&attrib);
			}
		}

		if (shader->vertexShader.usesInstanceId)
			src->add("uint instanceId = uint(gl_InstanceID);" _CRLF);
	}
}

void LatteDecompiler_emitShaderCodeGLSL(LatteDecompilerShader* shaderContext, StringBuf* src)
{
	LatteShader* shader = shaderContext->shader;

	if (shader->shaderType == LatteConst::ShaderType::Geometry)
	{
		src->add("layout(triangles) in;" _CRLF);
		src->add("layout(triangle_strip, max_vertices = 3) out;" _CRLF);
	}

	LatteDecompiler_emitVertexShaderInputGLSL(shaderContext, src);

	for (auto& cfInstruction : shaderContext->cfInstructions)
	{
		LatteDecompiler_emitClauseCode(shaderContext, &cfInstruction, false);
	}

	if (shader->shaderType == LatteConst::ShaderType::Geometry)
		src->add("EndPrimitive();" _CRLF);

	if (shaderContext->analyzer.outputPointSize)
	{
		src->add("gl_PointSize = renderState.pointSize;" _CRLF);
	}

	src->add(shaderContext->shaderCode.c_str());
}

void LatteDecompiler_generateGLSL(LatteDecompilerShader* shaderContext, StringBuf* src)
{
	src->add("#version 450" _CRLF);
	LatteDecompiler_emitShaderCodeGLSL(shaderContext, src);
}
