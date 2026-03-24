cmake_minimum_required(VERSION 3.10)
project(cemuMain)

# 1. Definiciones de plataforma
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
    add_compile_definitions(VK_NO_PROTOTYPES)
endif()

# 2. Flags globales de compilación
add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 3. PROCESAR SUBDIRECTORIOS (Obligatorio para que existan las librerías)
# No los bloqueamos, los necesitamos todos.
add_subdirectory(Common)
add_subdirectory(gui)
add_subdirectory(Cafe)
add_subdirectory(Cemu)
add_subdirectory(config)
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(util)
add_subdirectory(imgui)
add_subdirectory(resource)

# 4. Definición del binario principal (CemuBin)
# Aquí incluimos los 4 archivos del descompilador para forzar su existencia
set(SOURCES
    main.cpp
    mainLLE.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerAnalyzer.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerEmitGLSL.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerRegisterDataTypeTracker.cpp
)

if(ANDROID)
    # En Android se compila como librería estática que luego se une al JNI
    add_library(CemuBin STATIC ${SOURCES})
else()
    add_executable(CemuBin ${SOURCES})
    set(OUTPUT_NAME "Cemu")
    set_target_properties(CemuBin PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")
endif()

# 5. Enlace de librerías (Ahora sí existirán porque procesamos los subdirectories)
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

if(ANDROID)
    target_link_libraries(CemuBin PRIVATE log android jnigraphics)
    # Si existe el subdirectorio de Android, lo procesamos al final
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/cpp/CMakeLists.txt")
        add_subdirectory(android/app/src/main/cpp)
    endif()
endif()

# 6. Directorio de salida
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<1:>"
)
