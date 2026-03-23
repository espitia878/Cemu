#pragma once

struct LatteDecompilerALUInstruction
{
	struct LatteDecompilerCFInstruction* cfInstruction{};
	bool isOP3{};
	uint32 opcode{};
	uint32 instructionGroupIndex{};
	uint8 indexMode{};
	uint8 omod{};
	// destination
	uint32 destGpr{};
	uint8 destRel{};
	uint8 destElem{};
	uint8 destClamp{};
	uint8 writeMask{};
	// flags
	uint8 updateExecuteMask{};
	uint8 updatePredicate{};
	// source operands
	struct
	{
		uint32 sel{};
		uint8 rel{};
		uint8 abs{};
		uint8 neg{};
		uint8 chan{};
	}sourceOperand[3];
	union
	{
		uint32 w[4];
		float f[4];
	}literalData;
	// information from analyzer stage
	uint8 aluUnit{}; // 0-3 -> ALU.x/y/u/w (PV), 4 -> Trans unit (PS)
	uint8 indexInGroup{}; // index of instruction within instruction group
	bool isLastInstructionOfGroup{};
};

struct LatteDecompilerTEXInstruction
{
	struct LatteDecompilerCFInstruction* cfInstruction{};
	uint32 opcode{};
	// texture or vertex fetch
	// shared
	sint32 srcGpr;
	sint32 dstGpr;
	sint8 dstSel[4];
	// texture fetch
	struct
	{
		sint32 textureIndex{};
		sint32 samplerIndex{};
		uint32 offset{};
		sint8 srcSel[4]{};
		sint8 offsetX{};
		sint8 offsetY{};
		sint8 offsetZ{};
		bool unnormalized[4]{}; // set if texture coordinates are in [0,dim] range instead of [0,1]
		sint8 lodBias{}; // divide by 16 to get actual value
	}textureFetch;
	// memRead
	struct
	{
		uint32 arrayBase{};
		sint8 srcSelX{};
		uint32 format{};
		uint8 nfa{};
		uint8 isSigned{};
	}memRead;
};

struct LatteDecompilerCFInstruction
{
	uint32 type{};
	uint32 cfAddr{};
	// for clauses with instructions
	uint32 addr{};
	sint32 count{};
	// clause contains either ALU or TEX instructions
	std::vector<LatteDecompilerALUInstruction> instructionsALU;
	std::vector<LatteDecompilerTEXInstruction> instructionsTEX;
	// for clauses that access uniform buffers
	uint32 cBank0Index{};
	uint32 cBank1Index{};
	uint32 cBank0AddrBase{};
	uint32 cBank1AddrBase{};
	// for exports
	uint32 exportType{};
	uint8  exportComponentSel[4]{};
	uint32 exportBurstCount{};
	// for mem write
	uint32 memWriteArraySize{};
	uint8 memWriteCompMask{};
	uint8 memWriteElemSize{}; // 0-3
	// for exports and mem write
	uint32 exportArrayBase{};
	uint32 exportSourceGPR{};
	// misc
	uint32 cfCond{};
	uint32 popCount{};
	// information from analyzer stage
	bool modifiesPredicate{};
	bool modifiesActiveMask{};
	uint32 numPredInstructions{};
	sint32 activeStackDepth{}; // stack depth during the clause/CF instruction

	LatteDecompilerCFInstruction()
	{

	}

	~LatteDecompilerCFInstruction()
	{
		cemu_assert_debug(!(instructionsALU.size() != 0 && instructionsTEX.size() != 0)); // make sure we haven't accidentally added the wrong instruction type
	}

#if BOOST_OS_WINDOWS
	LatteDecompilerCFInstruction(LatteDecompilerCFInstruction& mE) = default;
	LatteDecompilerCFInstruction(LatteDecompilerCFInstruction&& mE) = default;
#else
	LatteDecompilerCFInstruction(const LatteDecompilerCFInstruction& mE) = default;
	LatteDecompilerCFInstruction(LatteDecompilerCFInstruction&& mE) = default;
#endif

	LatteDecompilerCFInstruction& operator=(LatteDecompilerCFInstruction&& mE) = default;
};

struct LatteDecompilerSubroutineInfo
{
	uint32 cfAddr;
	std::vector<LatteDecompilerCFInstruction> instructions;
};

// helper struct to track the highest accessed offset within a buffer
struct LatteDecompilerBufferAccessTracker
{
	bool hasStaticIndexAccess{false};
	bool hasDynamicIndexAccess{false};
	sint32 highestAccessDynamicIndex{0};
	sint32 highestAccessStaticIndex{0};

	// track access, index is the array index and not a byte offset
	void TrackAccess(sint32 index, bool isDynamicIndex)
	{
		if (isDynamicIndex)
		{
			hasDynamicIndexAccess = true;
			if (index > highestAccessDynamicIndex)
				highestAccessDynamicIndex = index;
		}
		else
		{
			hasStaticIndexAccess = true;
			if (index > highestAccessStaticIndex)
				highestAccessStaticIndex = index;
		}
	}

	sint32 DetermineSize(uint64 shaderBaseHash, sint32 maximumSize) const
	{
		if(shaderBaseHash == 0x8ff56afdf1a2f837) // XCX text rendering
			return 24;
		if(shaderBaseHash == 0x37b9100c1310d3bb) // BotW UI backdrops 1
			return 24;
		if(shaderBaseHash == 0xf7ba548c1fefe24a) // BotW UI backdrops 2
			return 30;

		sint32 highestAccessIndex = -1;
		if(hasStaticIndexAccess)
			highestAccessIndex = highestAccessStaticIndex;
		if(hasDynamicIndexAccess)
			return maximumSize; 
		if (highestAccessIndex < 0)
			return 1; 
		return highestAccessIndex + 1;
	}

	bool HasAccess() const
	{
		return hasStaticIndexAccess || hasDynamicIndexAccess;
	}

	bool HasRelativeAccess() const
	{
		return hasDynamicIndexAccess;
	}
};

struct LatteDecompilerShaderContext
{
	LatteDecompilerOutput_t* output;
	LatteDecompilerShader* shader;
	LatteConst::ShaderType shaderType;
	const class LatteDecompilerOptions* options;
	uint32* contextRegisters; // deprecated
	struct LatteContextRegister* contextRegistersNew;
	uint64 shaderBaseHash;
	StringBuf* shaderSource;
	std::vector<LatteDecompilerCFInstruction> cfInstructions;
	LatteFetchShader* fetchShader{};
	LatteParsedGSCopyShader* parsedGSCopyShader;
	bool hasError;
	struct
	{
		uint8 defaultDataType;
		bool genFloatReg; 
		bool genIntReg; 
		bool useArrayGPRs; 
	}typeTracker;
	struct
	{
		bool hasStreamoutEnable{}; 
		bool hasLoops{}; 
		bool isPointsPrimitive{}; 
		bool outputPointSize{}; 
		std::bitset<256> inputAttributSemanticMask; 
		LatteDecompilerBufferAccessTracker uniformRegisterAccessTracker;
		LatteDecompilerBufferAccessTracker uniformBufferAccessTracker[LATTE_NUM_MAX_UNIFORM_BUFFERS];
		bool hasSSBORead; 
		bool hasSSBOWrite; 
		std::bitset<LATTE_NUM_MAX_TEX_UNITS> texUnitUsesTexelCoordinates;
		bool hasCubeMapTexture; 
		bool hasGradientLookup; 
		bool usesRelativeGPRRead; 
		bool usesRelativeGPRWrite; 
		uint8 gprUseMask[(LATTE_NUM_GPR + 7) / 8]; 
		bool hasStreamoutWrite; 
		bool hasRedcCUBE; 
		bool modifiesPixelActiveState; 
		bool usesIntegerValues; 
		sint32 activeStackMaxDepth; 
		bool writesPointSize{};
		bool useSSBOForStreamout{};
		uint32 numEmitVertex{}; 
	}analyzer;

	bool isSubroutine;
	LatteDecompilerSubroutineInfo* subroutineInfo;

	bool hasUniformVarBlock;
	sint32 currentBindingPointVK{};
	sint32 currentBufferBindingPointMTL{};
	sint32 currentTextureBindingPointMTL{};
	struct ALUClauseTemporariesState* aluPVPSState{nullptr};
	std::vector<LatteDecompilerSubroutineInfo> list_subroutines;
	bool m_is_vulkan_android{false}; // <--- Cambio para Xiaomi 14T Pro
};

void LatteDecompiler_analyze(LatteDecompilerShaderContext* shaderContext, LatteDecompilerShader* shader);
void LatteDecompiler_analyzeDataTypes(LatteDecompilerShaderContext* shaderContext);
void LatteDecompiler_emitGLSLShader(LatteDecompilerShaderContext* shaderContext, LatteDecompilerShader* shader);
void LatteDecompiler_emitMSLShader(LatteDecompilerShaderContext* shaderContext, LatteDecompilerShader* shader);

void LatteDecompiler_cleanup(LatteDecompilerShaderContext* shaderContext);

sint32 LatteDecompiler_getColorOutputIndexFromExportIndex(LatteDecompilerShaderContext* shaderContext, sint32 exportIndex);
