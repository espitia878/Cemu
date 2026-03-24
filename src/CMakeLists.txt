cmake_minimum_required(VERSION 3.10)
project(cemuMain)

# Definiciones globales para Android
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
    add_compile_definitions(VK_NO_PROTOTYPES)
endif()

# Flags de compilación para evitar avisos innecesarios
add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# IMPORTANTE: No usamos add_subdirectory aquí para evitar el error de configuración en Android
# En su lugar, definimos la librería principal directamente.

set(SOURCES
    Main.cpp
    MainLLE.cpp
    # Rutas de los archivos del descompilador (El corazón del fix)
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerAnalyzer.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerEmitGLSL.cpp
    Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerRegisterDataTypeTracker.cpp
)

# Definimos CemuBin como librería compartida para Android
add_library(CemuBin SHARED ${SOURCES})

# Enlazamos las librerías necesarias
target_link_libraries(CemuBin PRIVATE
    CemuAudio
    CemuCafe
    CemuCommon
    CemuGui
    CemuComponents
    CemuConfig
    CemuInput
    CemuUtil
    log
    android
    jnigraphics
)

# Configuración de salida
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin"
    OUTPUT_NAME "Cemu"
)

# Si tienes subdirectorios específicos de Android, se incluyen al final
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/cpp/CMakeLists.txt")
    add_subdirectory(android/app/src/main/cpp)
endif()
