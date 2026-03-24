cmake_minimum_required(VERSION 3.10)
project(cemuMain)

# Definiciones de preprocesador esenciales para Android y Vulkan
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
    add_compile_definitions(VK_NO_PROTOTYPES)
endif()

# Flags para silenciar advertencias de conversión que bloquean el build en Clang/Android
add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Listamos las fuentes necesarias. 
# NOTA: He incluido los 4 archivos del descompilador de hardware aquí mismo.
# Esto garantiza que el Linker los vea y no dé el error "undefined reference".
set(SOURCES
    main.cpp
    mainLLE.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerAnalyzer.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerEmitGLSL.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerRegisterDataTypeTracker.cpp
)

# En Android, CemuBin debe ser una librería (STATIC o SHARED) según la estructura del NDK
if(ANDROID)
    add_library(CemuBin STATIC ${SOURCES})
else()
    add_executable(CemuBin ${SOURCES})
endif()

# Enlazamos con los módulos internos de Cemu
# El sistema buscará estos nombres en los subdirectorios internos
target_link_libraries(CemuBin PRIVATE
    CemuAudio
    CemuCafe
    CemuCommon
    CemuGui
    CemuComponents
    CemuConfig
    CemuInput
    CemuUtil
)

# Librerías nativas de Android requeridas para que el APK funcione
if(ANDROID)
    target_link_libraries(CemuBin PRIVATE log android jnigraphics)
endif()

# Configuración de rutas de salida
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<1:>"
    OUTPUT_NAME "Cemu"
)

# Solo incluimos subdirectorios si no estamos en un flujo de Android restringido
# Esto evita el error "exit value 1" al configurar.
if(NOT ANDROID)
    add_subdirectory(Common)
    add_subdirectory(gui)
    add_subdirectory(Cafe)
    add_subdirectory(Cemu)
    add_subdirectory(config)
    add_subdirectory(input)
    add_subdirectory(audio)
    add_subdirectory(util)
endif()
