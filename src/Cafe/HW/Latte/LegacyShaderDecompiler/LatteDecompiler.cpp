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

extern void _LatteDecompiler_Process(LatteDecompilerShaderContext* shaderContext, uint8* programData, uint32 programSize);

void LatteDecompiler_InitContext(LatteDecompilerShaderContext& dCtx, const LatteDecompilerOptions& options, LatteDecompilerOutput_t* output, LatteConst::ShaderType shaderType, uint64 shaderBaseHash, uint32* contextRegisters)
{
	dCtx.output = output;
	dCtx.shaderType = shaderType;
	dCtx.options = &options;
	dCtx.shaderBaseHash = shaderBaseHash;
	dCtx.contextRegisters = contextRegisters;
	dCtx.contextRegistersNew = (LatteContextRegister*)contextRegisters;
	dCtx.shader = nullptr; // Inicialización de seguridad
	
	if (output) {
		output->shaderType = shaderType;
		output->shader = nullptr;
	}
}

bool LatteDecompiler_ParseCFInstruction(LatteDecompilerShaderContext* shaderContext, uint32 cfIndex, uint32 cfWord0, uint32 cfWord1, bool* endOfProgram, std::vector<LatteDecompilerCFInstruction>& instructionList)
{
	return true; 
}

void LatteDecompiler_DecompileVertexShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, struct LatteFetchShader* fetchShader, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
	performanceMonitor.gpuTime_shaderCreate.beginMeasuring();
	LatteDecompilerShaderContext shaderContext = {};
	LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Vertex, shaderBaseHash, contextRegisters);
	
	LatteDecompilerShader* shader = new LatteDecompilerShader(LatteConst::ShaderType::Vertex);
	shaderContext.shader = shader;
	if (output) {
		output->shader = shader;
	}

	_LatteDecompiler_Process(&shaderContext, programData, programSize);
	performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

void LatteDecompiler_DecompileGeometryShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, uint8* gsCopyProgramData, uint32 gsCopyProgramSize, uint32 vsRingParameterCount, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
	performanceMonitor.gpuTime_shaderCreate.beginMeasuring();
	LatteDecompilerShaderContext shaderContext = {};
	LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Geometry, shaderBaseHash, contextRegisters);
	
	LatteDecompilerShader* shader = new LatteDecompilerShader(LatteConst::ShaderType::Geometry);
	shaderContext.shader = shader;
	if (output) {
		output->shader = shader;
	}

	_LatteDecompiler_Process(&shaderContext, programData, programSize);
	performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}

void LatteDecompiler_DecompilePixelShader(uint64 shaderBaseHash, uint32* contextRegisters, uint8* programData, uint32 programSize, LatteDecompilerOptions& options, LatteDecompilerOutput_t* output)
{
	performanceMonitor.gpuTime_shaderCreate.beginMeasuring();
	LatteDecompilerShaderContext shaderContext = {};
	LatteDecompiler_InitContext(shaderContext, options, output, LatteConst::ShaderType::Pixel, shaderBaseHash, contextRegisters);
	
	LatteDecompilerShader* shader = new LatteDecompilerShader(LatteConst::ShaderType::Pixel);
	shaderContext.shader = shader;
	if (output) {
		output->shader = shader;
	}

	_LatteDecompiler_Process(&shaderContext, programData, programSize);
	performanceMonitor.gpuTime_shaderCreate.endMeasuring();
}
