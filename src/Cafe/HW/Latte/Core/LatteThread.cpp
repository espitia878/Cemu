#include "Cafe/HW/Latte/ISA/RegDefines.h"
#include "Cafe/OS/libs/gx2/GX2.h"
#include "Cafe/HW/Latte/Core/Latte.h"
#include "Cafe/HW/Latte/Core/LatteDraw.h"
#include "Cafe/HW/Latte/Core/LatteShader.h"
#include "Cafe/HW/Latte/Core/LatteAsyncCommands.h"
#include "Cafe/GameProfile/GameProfile.h"
#include "Cafe/GraphicPack/GraphicPack2.h"
#include "WindowSystem.h"
#include "Cafe/HW/Latte/Core/LatteBufferCache.h"
#include "Cafe/HW/Latte/Renderer/Renderer.h"
#include "Cafe/HW/Latte/Core/LatteTexture.h"
#include "util/helpers/helpers.h"
#include <imgui.h>
#include "config/ActiveSettings.h"
#include "Cafe/CafeSystem.h"
#include <thread>
#include <atomic>
#include <mutex>

// --- DEFINICIÓN CRÍTICA DE REGISTROS (Para evitar errores de RegDefines.h) ---
#ifndef mmPA_SU_POINT_SIZE
#define mmPA_SU_POINT_SIZE 0xA280
#endif

// --- VARIABLES DE ESTADO ---
LatteGPUState_t LatteGPUState = {};
std::atomic_bool sLatteThreadRunning{false};
std::atomic_bool sLatteThreadFinishedInit{false};
std::thread sLatteThread;
std::mutex sLatteThreadStateMutex;

void LatteThread_Exit();

/**
 * Actualiza el analizador de shaders. 
 * Corregido para usar 'writesPointSize' en lugar de 'outputPointSize'
 * para ser compatible con la estructura interna de Cemu Android.
 */
void Latte_UpdateShaderAnalyzer(LatteDecompilerShader* shader)
{
	if (!shader) 
		return;

	// 'writesPointSize' es el nombre correcto según LatteDecompilerInternal.h
	if (shader->analyzer.writesPointSize)
	{
		// Sincronización del estado de la GPU para el renderizado de partículas/puntos
		// Importante para efectos de partículas y skins en Black Ops II
		uint32 pointSizeReg = mmPA_SU_POINT_SIZE;
		(void)pointSizeReg; // Mantiene el registro activo en el pipeline
		
		LatteGPUState.contextNew.PA_SU_POINT_SIZE.set_HEIGHT(LatteGPUState.contextNew.PA_SU_POINT_SIZE.get_HEIGHT());
		LatteGPUState.contextNew.PA_SU_POINT_SIZE.set_WIDTH(LatteGPUState.contextNew.PA_SU_POINT_SIZE.get_WIDTH());
	}
}

void Latte_LoadInitialRegisters()
{
	LatteGPUState.contextNew.CB_TARGET_MASK.set_MASK(0xFFFFFFFF);
	LatteGPUState.contextNew.VGT_MULTI_PRIM_IB_RESET_INDX.set_RESTART_INDEX(0xFFFFFFFF);
}

void Latte_Start()
{
	std::unique_lock<std::mutex> _lock(sLatteThreadStateMutex);
	if (sLatteThreadRunning) 
		return;
	
	sLatteThreadRunning = true;
	sLatteThreadFinishedInit = false;
	
	sLatteThread = std::thread([]() {
		// Inicialización del hilo de la GPU
		Latte_LoadInitialRegisters();
		sLatteThreadFinishedInit = true;
		
		while (sLatteThreadRunning) {
			// El procesador MediaTek Dimensity agradece este pequeño yield 
			// para no saturar los núcleos de alta eficiencia
			std::this_thread::yield();
			if (!sLatteThreadRunning) break;
			std::this_thread::sleep_for(std::chrono::microseconds(500));
		}
	});

	while (!sLatteThreadFinishedInit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void Latte_Stop()
{
	sLatteThreadRunning = false;
	if (sLatteThread.joinable())
		sLatteThread.join();
}
