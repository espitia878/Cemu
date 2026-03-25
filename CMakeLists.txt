# COMPONENTE VITAL: Esta debe ser SIEMPRE la primera línea
cmake_minimum_required(VERSION 3.22)

project(cemuMain)

# OPTIMIZACIÓN: Desactivar libusb para Android para evitar el error de "package not found"
if(ANDROID)
    set(ENABLE_NSYSHID_LIBUSB OFF CACHE BOOL "" FORCE)
    add_definitions(-DANDROID)
endif()

option(CEMU_CXX_FLAGS "Additional flags used for compiling Cemu source code")
if(CEMU_CXX_FLAGS)
    add_compile_options(${CEMU_CXX_FLAGS})
endif()

add_compile_definitions(VK_NO_PROTOTYPES)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# CORRECCIÓN 1: Estructura de subdirectorios con prefijo src/
add_subdirectory(src/Common)
add_subdirectory(src/gui)
add_subdirectory(src/Cafe)
add_subdirectory(src/Cemu)
add_subdirectory(src/config)
add_subdirectory(src/input)
add_subdirectory(src/audio)
add_subdirectory(src/util)
add_subdirectory(src/imgui)
add_subdirectory(src/resource)

if(ANDROID)
    # CORRECCIÓN 3: Librería estática para el núcleo del emulador
    add_library(CemuBin STATIC
        src/main.cpp
        src/mainLLE.cpp
    )
    # Enlace con el código nativo de la app Android
    add_subdirectory(src/android/app/src/main/cpp)
else()
    # Configuración para compilación en PC
    add_executable(CemuBin
        src/main.cpp
        src/mainLLE.cpp
    )
endif()
