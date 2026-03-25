cmake_minimum_required(VERSION 3.22)
project(cemuMain)

# ==============================================================================
# 1. PARCHE DE INTEGRACIÓN "TOTAL RECALL" (VCPKG + ANDROID)
# ==============================================================================
if(ANDROID)
    message(STATUS "[ASTUTO] Iniciando protocolo de detección de librerías para ARM64...")

    # Forzamos a CMake a buscar paquetes tanto en el NDK como en nuestras carpetas externas
    # Sin esto, Android ignora por completo a VCPKG
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

    # Ruta de VCPKG - Usamos la ruta relativa al proyecto para mayor seguridad
    set(VCPKG_ANDROID_DIR "${CMAKE_SOURCE_DIR}/dependencies/vcpkg/installed/arm64-android")

    if(EXISTS "${VCPKG_ANDROID_DIR}")
        list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ANDROID_DIR}")
        list(APPEND CMAKE_LIBRARY_PATH "${VCPKG_ANDROID_DIR}/lib")
        list(APPEND CMAKE_INCLUDE_PATH "${VCPKG_ANDROID_DIR}/include")
        
        # Inyectamos las rutas de share para que find_package no tenga excusas
        set(Boost_DIR "${VCPKG_ANDROID_DIR}/share/boost")
        set(ZArchive_DIR "${VCPKG_ANDROID_DIR}/share/zarchive")
        set(fmt_DIR "${VCPKG_ANDROID_DIR}/share/fmt")
        set(nlohmann_json_DIR "${VCPKG_ANDROID_DIR}/share/nlohmann_json")
        message(STATUS "[ASTUTO] Puente VCPKG establecido en: ${VCPKG_ANDROID_DIR}")
    endif()

    add_definitions(-DANDROID -DNOMINMAX)
    set(ENABLE_NSYSHID_LIBUSB OFF CACHE BOOL "" FORCE)
    set(ENABLE_WAYLAND OFF CACHE BOOL "" FORCE)
endif()

# ==============================================================================
# 2. OPTIMIZACIÓN EXTREMA (Xiaomi 14T Pro - Dimensity 9300+)
# ==============================================================================
# Tu procesador tiene núcleos Cortex-X4 y A720 (ARMv9). Vamos a sacarle fuego.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-O3 -flto -fomit-frame-pointer -ffast-math)
    
    if(ANDROID)
        # ARMv9-A es la arquitectura de tu chip. Esto activa instrucciones de IA y matemáticas rápidas.
        add_compile_options(-march=armv9-a+crypto+fp16+dotprod -mtune=generic)
        # Garbage collection de código: si no se usa, no ocupa espacio ni RAM
        add_compile_options(-ffunction-sections -fdata-sections)
    endif()
endif()

# ==============================================================================
# 3. NÚCLEO DEL PROYECTO
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(VK_NO_PROTOTYPES)

# Subdirectorios en orden lógico de dependencia
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
    # Librería estática que une todo el motor de Cemu
    add_library(CemuBin STATIC
        src/main.cpp
        src/mainLLE.cpp
    )
        # Librerías esenciales + El corazón de la emulación (CemuCafe)
    target_link_libraries(CemuBin PRIVATE 
        CemuCafe 
        CemuCommon 
        log 
        android 
        nativewindow
    )
    
    
    # Capa JNI (La que conecta el C++ con la App de Android)
    add_subdirectory(src/android/app/src/main/cpp)
else()
    add_executable(Cemu src/main.cpp src/mainLLE.cpp)
    target_link_libraries(Cemu PRIVATE CemuCafe)
endif()

# Limpieza de enlazado para que el APK sea más ligero
if(ANDROID)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
endif()
