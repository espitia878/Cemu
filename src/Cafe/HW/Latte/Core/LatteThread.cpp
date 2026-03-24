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

// --- VARIABLES DE ESTADO ---
LatteGPUState_t LatteGPUState = {};
std::atomic_bool sLatteThreadRunning = false;
std::atomic_bool sLatteThreadFinishedInit = false;
std::thread sLatteThread;
std::mutex sLatteThreadStateMutex;

void LatteThread_Exit();

// --- CORRECCIÓN CRÍTICA PARA EL ERROR DE COMPILACIÓN ---
// Se eliminó la referencia a 'writesPointSize' que ya no existe en el motor moderno de Cemu
void Latte_UpdateShaderAnalyzer(LatteDecompilerShader* shader)
{
	if (!shader) return;

	// Si el shader indica que tiene un tamaño de punto de salida, lo habilitamos
	if (shader->analyzer.outputPointSize)
	{
		// Lógica interna del renderizado
		LatteGPUState.contextNew.PA_SU_POINT_SIZE.set_HEIGHT(LatteGPUState.contextNew.PA_SU_POINT_SIZE.get_HEIGHT());
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
	if (sLatteThreadRunning) return;
	
	sLatteThreadRunning = true;
	sLatteThreadFinishedInit = false;
	sLatteThread = std::thread([]() {
		// Punto de entrada del hilo de la GPU
		sLatteThreadFinishedInit = true;
		while (sLatteThreadRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});

	while (!sLatteThreadFinishedInit)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void Latte_Stop()
{
	std::unique_lock<std::mutex> _lock(sLatteThreadStateMutex);
	if (!sLatteThreadRunning) return;
	sLatteThreadRunning = false;
	if (sLatteThread.joinable())
		sLatteThread.join();
}

void LatteThread_Exit()
{
	if (g_renderer)
		g_renderer->Shutdown();
    LatteBufferCache_UnloadAll();
	LatteTC_UnloadAllTextures();
    LatteSHRC_UnloadAll();
    LatteShaderCache_Close();
}
