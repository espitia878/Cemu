cmake_minimum_required(VERSION 3.20)
project(cemuMain)

# 1. Configuraciones Globales para Android
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR VK_NO_PROTOTYPES)
    add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 2. Subdirectorios del Núcleo (Core)
# Estos directorios contienen la lógica del emulador que es común a todas las plataformas
add_subdirectory(Common)
add_subdirectory(Cafe)
add_subdirectory(Cemu)
add_subdirectory(config)
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(util)
add_subdirectory(resource)

# 3. Definición del binario CemuBin
# Agregamos los archivos necesarios para el arranque inicial
set(SOURCES
    main.cpp
    mainLLE.cpp
    Cemu/Logging/CemuLogging.cpp
)

if(ANDROID)
    # En Android se compila como librería estática
    add_library(CemuBin STATIC ${SOURCES})
    
    # IMPORTANTE: Aquí conectamos con la carpeta que viste en el Drive
    # Esta carpeta es la que genera el puente con el APK
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/cpp/CMakeLists.txt")
        add_subdirectory(android/app/src/main/cpp)
    endif()
else()
    add_executable(CemuBin ${SOURCES})
endif()

# 4. Enlace de Librerías
# Usamos solo las librerías que el port de Android reconoce
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

# 5. Salida del Binario
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin"
    OUTPUT_NAME "Cemu"
)
