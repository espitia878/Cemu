cmake_minimum_required(VERSION 3.22)
project(cemuMain)

# ==============================================================================
# 1. PARCHE DE INTEGRACIÓN Y DETECCIÓN DE ENTORNO
# ==============================================================================
if(ANDROID)
    message(STATUS "[EXPERTO] Optimizando Cemu para entorno Android...")
    
    # Ruta absoluta de dependencias VCPKG
    set(VCPKG_ANDROID_DIR "${CMAKE_SOURCE_DIR}/dependencies/vcpkg/installed/arm64-android")
    
    if(EXISTS "${VCPKG_ANDROID_DIR}")
        list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ANDROID_DIR}")
        # Forzamos la detección de librerías críticas
        foreach(lib boost zarchive fmt nlohmann_json pugixml)
            set(${lib}_DIR "${VCPKG_ANDROID_DIR}/share/${lib}")
        endforeach()
    endif()

    # Desactivar componentes de PC que causan errores en móviles
    set(ENABLE_NSYSHID_LIBUSB OFF CACHE BOOL "" FORCE)
    set(ENABLE_WAYLAND OFF CACHE BOOL "" FORCE)
    add_definitions(-DANDROID -DNOMINMAX)
endif()

# ==============================================================================
# 2. OPTIMIZACIONES DE RENDIMIENTO (Xiaomi 14T Pro / Dimensity 9300+)
# ==============================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # -O3: Optimización máxima de velocidad
    # -flto: Optimización en tiempo de enlazado (más FPS)
    # -fomit-frame-pointer: Libera un registro para cálculos extra
    add_compile_options(-O3 -flto -fomit-frame-pointer -ffunction-sections -fdata-sections)
    
    if(ANDROID)
        # Banderas específicas para procesadores ARM de 64 bits modernos
        add_compile_options(-march=armv8.2-a+fp16+dotprod -mcpu=cortex-a78) 
        # Nota: Aunque tu chip es más nuevo, esta base asegura estabilidad máxima y gran velocidad.
        
        # Optimización de punto flotante para emulación
        add_compile_options(-ffast-math)
    endif()
endif()

# ==============================================================================
# 3. CONFIGURACIÓN DE LIBRERÍAS Y SÍMBOLOS
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(VK_NO_PROTOTYPES)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ==============================================================================
# 4. ESTRUCTURA DEL PROYECTO
# ==============================================================================
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
    # Creamos el núcleo estático para ser consumido por la interfaz JNI de Android
    add_library(CemuBin STATIC
        src/main.cpp
        src/mainLLE.cpp
    )
    
    # Enlazamos las librerías base de Android necesarias para gráficos y logs
    target_link_libraries(CemuBin PRIVATE log android nativewindow)
    
    # Capa de comunicación Android-C++ (JNI)
    add_subdirectory(src/android/app/src/main/cpp)
else()
    add_executable(Cemu src/main.cpp src/mainLLE.cpp)
    target_link_libraries(Cemu PRIVATE CemuCafe)
endif()

# ==============================================================================
# 5. LIMPIEZA FINAL (Evita errores de símbolos duplicados)
# ==============================================================================
if(ANDROID)
    # El linker de Android es estricto; eliminamos secciones muertas para ahorrar RAM
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
endif()
