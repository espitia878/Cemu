cmake_minimum_required(VERSION 3.20)
project(cemuMain)

# 1. Ajustes de Compilador para Android y MediaTek
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR VK_NO_PROTOTYPES)
    add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 2. Directorios de Inclusión (Para que el compilador encuentre los .h)
# Esto evita que tengamos que entrar en cada subcarpeta
target_include_directories(CemuBin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/Cafe
    ${CMAKE_CURRENT_SOURCE_DIR}/Cemu
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/input
    ${CMAKE_CURRENT_SOURCE_DIR}/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/util
)

# 3. Definición del Binario y Archivos Críticos
# He quitado los "add_subdirectory" que estaban fallando
set(SOURCES
    main.cpp
    mainLLE.cpp
    Cemu/Logging/CemuLogging.cpp
)

if(ANDROID)
    # En Android creamos una librería estática para que Gradle la use
    add_library(CemuBin STATIC ${SOURCES})
    
    # Solo entramos en la carpeta de Android si existe el archivo ahí
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/cpp/CMakeLists.txt")
        add_subdirectory(android/app/src/main/cpp)
    endif()
else()
    add_executable(CemuBin ${SOURCES})
endif()

# 4. Vinculación de Librerías del Sistema
# Nota: Si las librerías CemuAudio, CemuCafe, etc., no se están creando,
# el build fallará aquí. Por ahora, dejamos las esenciales de Android.
if(ANDROID)
    target_link_libraries(CemuBin PRIVATE log android jnigraphics)
endif()

# 5. Ubicación del archivo resultante
set_target_properties(CemuBin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin"
    OUTPUT_NAME "Cemu"
)
