cmake_minimum_required(VERSION 3.22)

# --- DEFINICIÓN DE LA LIBRERÍA JNI ---
add_library(CemuAndroid SHARED
    AndroidAudio.cpp
    AndroidEmulatedController.cpp
    CompressTitleCallbacks.cpp
    GameTitleLoader.cpp
    Image.cpp
    JNIUtils.cpp
    NativeActiveSettings.cpp
    NativeAccount.cpp
    NativeEmulation.cpp
    NativeGameTitles.cpp
    NativeGraphicPacks.cpp
    NativeInput.cpp
    NativeLib.cpp
    NativeLocalization.cpp
    NativeLogging.cpp
    NativeSettings.cpp
    NativeSwkbd.cpp
    WuaConverter.cpp
)

# --- OPTIMIZACIÓN DE HARDWARE (Xiaomi 14T Pro) ---
set_target_properties(CemuAndroid PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    # IPO (LTO) permite que el compilador optimice funciones entre archivos
    INTERPROCEDURAL_OPTIMIZATION ON
)

# --- CONEXIÓN DE LIBRERÍAS (EL "MOTOR" DE CEMU) ---
target_link_libraries(CemuAndroid PRIVATE
    CemuBin          # El núcleo que configuramos en el raíz
    CemuCommon
    CemuAudio
    CemuComponents
    CemuCafe
    CemuGui
    ZArchive::zarchive
    stb
    log              # Para depuración en tiempo real
    android          # Funciones base del sistema
    jnigraphics      # Renderizado rápido de texturas
    nativewindow     # Latencia cero en pantalla
    EGL              # Necesario para contextos gráficos estables
    GLESv3           # Para aprovechar la GPU Immortalis-G720
)

# --- BANDERAS DE SEGURIDAD Y RENDIMIENTO ---
target_compile_options(CemuAndroid PRIVATE 
    -w               # Silencia advertencias que bloquean el build
    -fexceptions     # Permite el manejo de errores complejos de C++
    -frtti           # Identificación de tipos en tiempo de ejecución (vital para Cemu)
    -O3              # Máxima optimización de código
)

# Parche para asegurar que las excepciones no inflen demasiado el binario
if(ANDROID)
    target_link_options(CemuAndroid PRIVATE "-Wl,--exclude-libs,ALL" "-Wl,--gc-sections")
endif()

message(STATUS "[ASTUTO] Puente JNI blindado para arquitectura ARMv9-A")
