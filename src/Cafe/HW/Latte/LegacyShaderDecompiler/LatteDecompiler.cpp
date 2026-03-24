#include "Cafe/HW/Latte/Core/LatteConst.h"
#include "Cafe/HW/Latte/Core/LatteShaderAssembly.h"
#include "Cafe/HW/Latte/ISA/RegDefines.h"
#include "Cafe/HW/Latte/Core/Latte.h"
#include "Cafe/HW/Latte/Core/LatteDraw.h"
#include "Cafe/HW/Latte/Core/LatteShader.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerInternal.h"
#include "Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerInstructions.h"
#include "Cafe/HW/Latte/Core/FetchShader.h"
#include "Cafe/HW/Latte/Core/LattePerformanceMonitor.h"
#include "Cafe/HW/Latte/Renderer/Renderer.h"
#include "util/helpers/helpers.h"
#include <cstring>

// Declaración de funciones externas (Firma exacta de tus .cpp)
void LatteDecompiler_analyze(LatteDecompilerShaderContext* shaderContext, LatteDecompilerShader* shader);
void LatteDecompiler_emitGLSLShader(LatteDecompilerShaderContext* shaderContext, LatteDecompilerShader* shader);

void LatteDecompiler_InitContext(LatteDecompilerShaderContext& dCtx, const LatteDecompilerOptions& options, LatteDecompilerOutput_t* output, LatteConst::ShaderType shaderType, uint64 shaderBaseHash, uint32* contextRegisters)
{
    memset(&dCtx, 0, sizeof(LatteDecompilerShaderContext));
    dCtx.output = output;
    dCtx.shaderType = shaderType;
    dCtx.options = &options;
    dCtx.shaderBaseHash = shaderBaseHash;
    dCtx.contextRegisters = contextRegisters;
    dCtx.contextRegistersNew = (LatteContextRegister*)contextRegisters;
}

static void _LatteDecompiler_DoWork(LatteDecompilerShaderContext* shaderContext, uint8* programData, uint32 programSize)
{
    LatteDecompilerShader shader{}; 
    shader.programCode = programData;
    shader.programSize = programSize;
    
    // CORRECCIÓN CLAVE: Convertimos el enum a uint32 para evitar el error del log 17:48
    shader.shaderType = (uint32)shaderContext->shaderType;

    LatteDecompiler_analyze(shaderContext, &shader);
    LatteDecompiler_emitGLSLShader(shaderContext, &shader);
}

void LatteDecompiler_DecompileVertexShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, struct LatteFetchShader* fetchShader, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
    performanceMonitor.gpuTime_shaderCreate.beginMeasuring();
    LatteDecompilerShaderContext shaderContext;
    LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Vertex, shaderBaseHash, contextRegisters);
    _LatteDecompiler_DoWork(&shaderContext, programData, programSize);
    performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

void LatteDecompiler_DecompileGeometryShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, uint8* gsCopyProgramData, uint32 gsCopyProgramSize, uint32 vsRingParameterCount, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
    performanceMonitor.gpuTime_shaderCreate.beginMeasuring();
    LatteDecompilerShaderContext shaderContext;
    LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Geometry, shaderBaseHash, contextRegisters);
    _LatteDecompiler_DoWork(&shaderContext, programData, programSize);
    performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

void LatteDecompiler_DecompilePixelShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
    performanceMonitor.gpuTime_shaderCreate.beginMeasuring();
    LatteDecompilerShaderContext shaderContext;
    LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Pixel, shaderBaseHash, contextRegisters);
    _LatteDecompiler_DoWork(&shaderContext, programData, programSize);
    performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

bool LatteDecompiler_ParseCFInstruction(LatteDecompilerShaderContext* shaderContext, uint32 cfIndex, uint32 cfWord0, uint32 cfWord1, bool* endOfProgram, std::vector<LatteDecompilerCFInstruction>& instructionList)
{
    return true; 
}
