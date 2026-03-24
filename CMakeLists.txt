project(cemuMain)

option(CEMU_CXX_FLAGS "Additional flags used for compiling Cemu source code")
if(CEMU_CXX_FLAGS)
	add_compile_options(${CEMU_CXX_FLAGS})
endif()

# ... (mantener validaciones de punteros y definiciones de plataforma)

add_compile_definitions(VK_NO_PROTOTYPES)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# CORRECCIÓN 1: Añadir prefijo 'src/' a todos los subdirectorios
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

# CORRECCIÓN 2: Añadir el subdirectorio de componentes que falta
# add_subdirectory(src/components) # Descomenta esto si la carpeta existe

if(ANDROID)
    # CORRECCIÓN 3: Ruta completa a los archivos fuente para Android
    add_library(CemuBin STATIC
        src/main.cpp
        src/mainLLE.cpp
    )
    add_subdirectory(src/android/app/src/main/cpp)
else()
	# CORRECCIÓN 4: Ruta completa a los archivos fuente para PC
	add_executable(CemuBin
		src/main.cpp
		src/mainLLE.cpp
	)
    # ... (resto de configuración de MSVC/WIN32)
endif()

# ... (mantener configuración de MACOS_BUNDLE si aplica)

set_target_properties(CemuBin PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<1:>"
	OUTPUT_NAME "${OUTPUT_NAME}"
)

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

# ... (resto del archivo)
