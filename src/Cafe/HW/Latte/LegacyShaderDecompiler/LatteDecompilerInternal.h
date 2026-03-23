#pragma once

#include <vector>
#include <string>
#include <bitset>
#include "Cafe/HW/Latte/Core/LatteConst.h"

struct LatteDecompilerShaderContext
{
	// --- MODIFICACIÓN PARA XIAOMI 14T PRO ---
	// Usamos 'static' para que no afecte al tamaño de la estructura
	// y así ShaderSerializer.cpp no note ningún cambio.
	static inline bool m_is_vulkan_android = false; 
	// ---------------------------------------

	struct LatteDecompilerOutput_t* output;
	struct LatteDecompilerShader* shader;
	LatteConst::ShaderType shaderType;
	const class LatteDecompilerOptions* options;
	uint32* contextRegisters; 
	struct LatteContextRegister* contextRegistersNew;
	uint64 shaderBaseHash;
	class StringBuf* shaderSource;
	std::vector<struct LatteDecompilerCFInstruction> cfInstructions;
	struct LatteFetchShader* fetchShader{};
	struct LatteParsedGSCopyShader* parsedGSCopyShader;
	bool hasError;

	struct
	{
		bool writesColor[8];
		bool writesDepth;
		bool writesStencil;
		bool writesSampleMask;
		bool usesDiscard;
		bool usesSampleID;
		bool usesSamplePosition;
		bool usesSampleMaskIn;
		bool usesCentroid;
		bool usesDepthRange;
		bool usesFragCoord;
		bool usesFrontFacing;
		bool usesPrimitiveID;
		bool usesLayer;
		bool usesViewportIndex;
		bool usesInstanceID;
		bool usesVertexID;
		bool usesDrawID;
		bool usesBaseVertex;
		bool usesBaseInstance;
		bool outputPointSize;
		bool hasTextureRead;
		bool hasTextureWrite;
		bool hasSSBORead;
		bool hasSSBOWrite;
		std::bitset<32> texUnitUsesTexelCoordinates;
		bool hasCubeMapTexture;
		bool hasGradientLookup;
		bool usesRelativeGPRRead;
		bool usesRelativeGPRWrite;
		uint8 gprUseMask[(128 + 7) / 8];
		bool hasStreamoutWrite;
		bool hasRedcCUBE;
		bool modifiesPixelActiveState;
		bool usesIntegerValues;
		sint32 activeStackMaxDepth;
		bool writesPointSize{};
		bool useSSBOForStreamout{};
		uint32 numEmitVertex{};
	} analyzer;

	bool isSubroutine;
	struct LatteDecompilerSubroutineInfo* subroutineInfo;

	bool hasUniformVarBlock;
	sint32 currentBindingPointVK{};
	sint32 currentBufferBindingPointMTL{};
	sint32 currentTextureBindingPointMTL{};
	struct ALUClauseTemporariesState* aluPVPSState{nullptr};
	std::vector<struct LatteDecompilerSubroutineInfo> list_subroutines;
};

void LatteDecompiler_analyze(LatteDecompilerShaderContext* shaderContext, struct LatteDecompilerShader* shader);
