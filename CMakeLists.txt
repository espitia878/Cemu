project(cemuMain)

option(CEMU_CXX_FLAGS "Additional flags used for compiling Cemu source code")
if(CEMU_CXX_FLAGS)
	add_compile_options(${CEMU_CXX_FLAGS})
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message( FATAL_ERROR "Pointers are not 64bit" )
endif()

if(MSVC)
	add_compile_definitions(WIN32_LEAN_AND_MEAN CURL_STATICLIB)
elseif(UNIX)
	if(APPLE)
		add_compile_definitions(
			_XOPEN_SOURCE
			VK_USE_PLATFORM_MACOS_MVK
			VK_USE_PLATFORM_METAL_EXT
		)
	elseif(ANDROID)
		add_compile_definitions(VK_USE_PLATFORM_ANDROID_KHR)
	else()
		add_compile_definitions(
			VK_USE_PLATFORM_XLIB_KHR # legacy. Do we need to support XLIB surfaces?
			VK_USE_PLATFORM_XCB_KHR
		)
		if (ENABLE_WAYLAND)
			add_compile_definitions(VK_USE_PLATFORM_WAYLAND_KHR)
		endif()
	endif()
	# warnings
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-Wno-ambiguous-reversed-operator)
	endif()

    add_compile_options(-Wno-multichar -Wno-invalid-offsetof -Wno-switch -Wno-ignored-attributes -Wno-deprecated-enum-enum-conversion)
endif()

set(OUTPUT_NAME "Cemu")

# Lista de archivos fuente
set(SOURCES
	Main.cpp
	# Hardware / Latte / Decompiler (Nuevos archivos añadidos aquí)
	Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompiler.cpp
	Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerAnalyzer.cpp
	Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerEmitGLSL.cpp
	Cafe/HW/Latte/LegacyShaderDecompiler/LatteDecompilerRegisterDataTypeTracker.cpp
)

add_executable(CemuBin ${SOURCES})

if(APPLE)
	set_target_properties(CemuBin PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_BUNDLE_NAME "${OUTPUT_NAME}"
		MACOSX_BUNDLE_GUI_IDENTIFIER "info.cemu.Cemu"
		MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/info.plist"
	)
	
	# rpath for internal libraries
	set_target_properties(CemuBin PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")
	
	add_custom_command(TARGET CemuBin POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/bin/resources/update.sh" "${CMAKE_SOURCE_DIR}/bin/${OUTPUT_NAME}.app/Contents/MacOS/update.sh"
		COMMAND bash -c "install_name_tool -add_rpath @executable_path/../Frameworks ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_NAME}.app/Contents/MacOS/${OUTPUT_NAME}"
		COMMAND bash -c "install_name_tool -change ${LIBUSB_PATH} @executable_path/../Frameworks/libusb-1.0.0.dylib ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_NAME}.app/Contents/MacOS/${OUTPUT_NAME}")
endif()

set_target_properties(CemuBin PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<1:>"
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

if(ENABLE_SDL)
	target_link_libraries(CemuBin PRIVATE SDL2::SDL2)
endif()

if(UNIX AND NOT APPLE)
	target_link_options(CemuBin PRIVATE -z noexecstack)
endif()
