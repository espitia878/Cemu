cmake_minimum_required(VERSION 3.20)
project(cemuMain)

# 1. Configuración para Android y MediaTek
if(ANDROID)
    add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR VK_NO_PROTOTYPES)
    add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes)
endif()

# 2. Los subdirectorios reales del port de Android
add_subdirectory(Common)
add_subdirectory(Cafe)
add_subdirectory(Cemu)
add_subdirectory(config)
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(util)
# resource se incluye si existe, si no, CMake lo ignorará
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resource/CMakeLists.txt")
    add_subdirectory(resource)
endif()

# 3. El puente con el código nativo de Android
# Esta es la parte que hace que la APP abra en tu Xiaomi
add_subdirectory(android/app/src/main/cpp)

# 4. Definición de la librería principal
set(SOURCES
    main.cpp
    mainLLE.cpp
    Cemu/Logging/CemuLogging.cpp
)

add_library(CemuBin STATIC ${SOURCES})

# 5. Vinculación de librerías (Solo las necesarias)
target_link_libraries(CemuBin PRIVATE
    CemuAudio
    CemuCafe
    CemuCommon
    CemuConfig
    CemuInput
    CemuUtil
    log android jnigraphics
)
