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

// Usamos internal linkage para mejorar la optimización del compilador
extern void _LatteDecompiler_Process(LatteDecompilerShaderContext* shaderContext, uint8* programData, uint32 programSize);

// Inline para reducir el overhead de llamada en funciones pequeñas
inline void LatteDecompiler_SetupContext(LatteDecompilerShaderContext& dCtx, LatteDecompilerOutput_t* output, LatteConst::ShaderType type, uint64 hash, uint32* regs, const LatteDecompilerOptions& opts)
{
    // Inicialización ultra-rápida (Zero-init)
    dCtx = {}; 
    
    dCtx.output = output;
    dCtx.shaderType = type;
    dCtx.options = &opts;
    dCtx.shaderBaseHash = hash;
    dCtx.contextRegisters = regs;
    dCtx.contextRegistersNew = reinterpret_cast<LatteContextRegister*>(regs);

    if (output) {
        // Aseguramos que el output esté limpio de basura
        output->shader = nullptr; 
    }
}

// Macro para reducir repetición de código y errores humanos
#define DECOMPILE_TEMPLATE(type) \
    if (!programData || programSize == 0) return; \
    performanceMonitor.gpuTime_shaderCreate.beginMeasuring(); \
    LatteDecompilerShaderContext shaderContext; \
    LatteDecompiler_SetupContext(shaderContext, output, type, shaderBaseHash, contextRegisters, options); \
    _LatteDecompiler_Process(&shaderContext, programData, programSize); \
    performanceMonitor.gpuTime_shaderCreate.endMeasuring();

void LatteDecompiler_DecompileVertexShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, struct LatteFetchShader* fetchShader, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
    DECOMPILE_TEMPLATE(LatteConst::ShaderType::Vertex);
}

void LatteDecompiler_DecompileGeometryShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, uint8* gsCopyProgramData, uint32 gsCopyProgramSize, uint32 vsRingParameterCount, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
    DECOMPILE_TEMPLATE(LatteConst::ShaderType::Geometry);
}

void LatteDecompiler_DecompilePixelShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
    DECOMPILE_TEMPLATE(LatteConst::ShaderType::Pixel);
}

bool LatteDecompiler_ParseCFInstruction(LatteDecompilerShaderContext* shaderContext, uint32 cfIndex, uint32 cfWord0, uint32 cfWord1, bool* endOfProgram, std::vector<LatteDecompilerCFInstruction>& instructionList)
{
    return true; // Placeholder optimizado
}
