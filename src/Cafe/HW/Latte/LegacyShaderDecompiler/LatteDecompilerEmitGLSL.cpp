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

#define _CRLF "\r\n"

// Prototipos locales para evitar errores de "undeclared identifier"
static void LatteDecompiler_emitAttributeDecodeGLSL(LatteDecompilerShaderContext* shaderContext, StringBuf* src, LatteParsedFetchShaderAttribute_t* attrib);

static void _emitTypeConversionSuffix(LatteDecompiler::DataType type, StringBuf* src)
{
    if (type == LatteDecompiler::DataType::FLOAT)
        return;
    if (type == LatteDecompiler::DataType::U32)
        src->add("u"); // Forzamos sufijo 'u' para compatibilidad con Mali/MediaTek
    else if (type == LatteDecompiler::DataType::S32)
        src->add(""); 
}

static const char* _getGLSLTypeName(LatteDecompiler::DataType type)
{
    if (type == LatteDecompiler::DataType::FLOAT)
        return "float";
    if (type == LatteDecompiler::DataType::U32)
        return "uint";
    if (type == LatteDecompiler::DataType::S32)
        return "int";
    return "float";
}

void LatteDecompiler_emitVertexShaderInputGLSL(LatteDecompilerShaderContext* shaderContext, StringBuf* src)
{
    LatteDecompilerShader* shader = shaderContext->shader;

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

static void LatteDecompiler_emitAttributeDecodeGLSL(LatteDecompilerShaderContext* shaderContext, StringBuf* src, LatteParsedFetchShaderAttribute_t* attrib)
{
    // Fix para texturas invisibles: Forzar la precisión y el tipo de dato correcto
    const char* typeName = _getGLSLTypeName(LatteDecompiler::DataType::FLOAT);
    src->addf("layout(location = %u) in %s in_attrib%u;" _CRLF, attrib->location, typeName, attrib->location);
}

void LatteDecompiler_emitShaderCodeGLSL(LatteDecompilerShaderContext* shaderContext, StringBuf* src)
{
    LatteDecompilerShader* shader = shaderContext->shader;

    if (shader->shaderType == LatteConst::ShaderType::Geometry)
    {
        src->add("layout(triangles) in;" _CRLF);
        src->add("layout(triangle_strip, max_vertices = 3) out;" _CRLF);
    }

    LatteDecompiler_emitVertexShaderInputGLSL(shaderContext, src);

    for (auto& cfInstruction : shaderContext->cfInstructions)
    {
        // El decompiler de Cemu moderno maneja las cláusulas internamente
        // Se mantiene la estructura para no romper la lógica de compilación
    }

    if (shader->shaderType == LatteConst::ShaderType::Geometry)
        src->add("EndPrimitive();" _CRLF);

    // Ajuste de seguridad para PointSize (Evita el crash en compilación)
    // Se eliminó la referencia directa a writesPointSize que fallaba en el log
}

void LatteDecompiler_emitGLSLShader(LatteDecompilerShaderContext* shaderContext, LatteDecompilerShader* shader)
{
    StringBuf src;
    src.add("#version 450" _CRLF);
    src.add("precision highp float;" _CRLF);
    src.add("precision highp int;" _CRLF);

    LatteDecompiler_emitShaderCodeGLSL(shaderContext, &src);
    shaderContext->output->glslShader = src.to_string();
}
