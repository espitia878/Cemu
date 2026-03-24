cmake_minimum_required(VERSION 3.20)
project(cemuMain)

# 1. Ajustes para Android
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR VK_NO_PROTOTYPES)
    add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 2. Subdirectorios que SÍ existen en tu carpeta src/
add_subdirectory(Common)
add_subdirectory(Cafe)
add_subdirectory(Cemu)
add_subdirectory(config)
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(util)
add_subdirectory(resource)

# 3. Archivos Fuente del Ejecutable
# Aquí incluimos el archivo de Logging que encontraste en src/Cemu/Logging/
set(SOURCES
    main.cpp
    mainLLE.cpp
    Cemu/Logging/CemuLogging.cpp
)

if(ANDROID)
    add_library(CemuBin STATIC ${SOURCES})
    # Esto apunta a la carpeta de pegado de Android (JNI)
    add_subdirectory(android/app/src/main/cpp)
else()
    add_executable(CemuBin ${SOURCES})
endif()

# 4. Vinculación de Librerías
# HE ELIMINADO: CemuGui y CemuComponents (porque no existen en tu carpeta src)
target_link_libraries(CemuBin PRIVATE
    CemuAudio
    CemuCafe
    CemuCommon
    CemuConfig
    CemuInput
    CemuUtil
)

if(ANDROID)
    target_link_libraries(CemuBin PRIVATE log android jnigraphics)
endif()

# 5. Propiedades de salida
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<1:>"
    OUTPUT_NAME "Cemu"
)
