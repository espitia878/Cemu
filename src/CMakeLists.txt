cmake_minimum_required(VERSION 3.10)
project(cemuMain)

# 1. Definiciones de plataforma
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR VK_NO_PROTOTYPES)
endif()

# 2. Flags de compilación para evitar errores por advertencias
add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 3. Subdirectorios REALES (Basado en lo que sí tienes en src/)
add_subdirectory(Common)
add_subdirectory(Cafe)
add_subdirectory(Cemu)
add_subdirectory(config)
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(util)
add_subdirectory(resource)
# NO incluimos 'gui' ni 'components' porque son para Windows o Kotlin

# 4. Definición del Binario (CemuBin)
# Forzamos la inclusión de los descompiladores de shaders para evitar errores de Linker
set(SOURCES
    main.cpp
    mainLLE.cpp
    # El archivo de Logging que encontraste:
    Cemu/Logging/CemuLogging.cpp
    # Archivos críticos del descompilador:
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerAnalyzer.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerEmitGLSL.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerRegisterDataTypeTracker.cpp
)

if(ANDROID)
    add_library(CemuBin STATIC ${SOURCES})
    # Aquí es donde se compila el código nativo de Android que encontraste
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/cpp")
        add_subdirectory(android/app/src/main/cpp)
    endif()
else()
    add_executable(CemuBin ${SOURCES})
endif()

# 5. Enlace de Librerías (SIN CemuComponents ni CemuGui)
target_link_libraries(CemuBin PRIVATE
    CemuAudio
    CemuCafe
    CemuCommon
    # Quitamos CemuGui y CemuComponents porque no aplican a este build nativo
    CemuConfig
    CemuInput
    CemuUtil
)

if(ANDROID)
    target_link_libraries(CemuBin PRIVATE log android jnigraphics)
endif()

# 6. Configuración de salida
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<1:>"
    OUTPUT_NAME "Cemu"
)
