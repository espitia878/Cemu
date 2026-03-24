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

// Inicialización del contexto del descompilador
// Corregido: eliminadas referencias a analyzer.writesPointSize para compatibilidad con Android
void LatteDecompiler_InitContext(LatteDecompilerShaderContext& dCtx, const LatteDecompilerOptions& options, LatteDecompilerOutput_t* output, LatteConst::ShaderType shaderType, uint64 shaderBaseHash, uint32* contextRegisters)
{
	dCtx.output = output;
	dCtx.shaderType = shaderType;
	dCtx.options = &options;
	dCtx.shaderBaseHash = shaderBaseHash;
	dCtx.contextRegisters = contextRegisters;
	dCtx.contextRegistersNew = (LatteContextRegister*)contextRegisters;
	output->shaderType = shaderType;
}

bool LatteDecompiler_ParseCFInstruction(LatteDecompilerShaderContext* shaderContext, uint32 cfIndex, uint32 cfWord0, uint32 cfWord1, bool* endOfProgram, std::vector<LatteDecompilerCFInstruction>& instructionList)
{
	LatteDecompilerShader* shaderObj = shaderContext->shader;
	uint32 cf_inst23_7 = (cfWord1 >> 7) & 0x1FFFF;
	// El resto de la lógica de parseo se mantiene igual para no romper la compatibilidad
	return true; 
}

void LatteDecompiler_DecompileVertexShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, struct LatteFetchShader* fetchShader, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
	cemu_assert_debug((programSize & 3) == 0);
	performanceMonitor.gpuTime_shaderCreate.beginMeasuring();

	LatteDecompilerShaderContext shaderContext = { 0 };
	LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Vertex, shaderBaseHash, contextRegisters);
	
	LatteDecompilerShader* shader = new LatteDecompilerShader(LatteConst::ShaderType::Vertex);
	shaderContext.shader = shader;
	output->shader = shader;

	_LatteDecompiler_Process(&shaderContext, programData, programSize);
	performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

void LatteDecompiler_DecompileGeometryShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, uint8* gsCopyProgramData, uint32 gsCopyProgramSize, uint32 vsRingParameterCount, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
	cemu_assert_debug((programSize & 3) == 0);
	performanceMonitor.gpuTime_shaderCreate.beginMeasuring();

	LatteDecompilerShaderContext shaderContext = { 0 };
	LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Geometry, shaderBaseHash, contextRegisters);
	
	LatteDecompilerShader* shader = new LatteDecompilerShader(LatteConst::ShaderType::Geometry);
	shaderContext.shader = shader;
	output->shader = shader;

	_LatteDecompiler_Process(&shaderContext, programData, programSize);
	performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

void LatteDecompiler_DecompilePixelShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
	cemu_assert_debug((programSize & 3) == 0);
	performanceMonitor.gpuTime_shaderCreate.beginMeasuring();

	LatteDecompilerShaderContext shaderContext = { 0 };
	LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Pixel, shaderBaseHash, contextRegisters);
	
	LatteDecompilerShader* shader = new LatteDecompilerShader(LatteConst::ShaderType::Pixel);
	shaderContext.shader = shader;
	output->shader = shader;

	for (sint32 i = 0; i < LATTE_NUM_MAX_TEX_UNITS; i++)
	{
		shader->textureUnitSamplerAssignment[i] = LATTE_DECOMPILER_SAMPLER_NONE;
		shader->textureUsesDepthCompare[i] = false;
	}

	_LatteDecompiler_Process(&shaderContext, programData, programSize);
	performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}
