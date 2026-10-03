cmake_minimum_required(VERSION 3.20)
project(sandbox CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source files ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE ALL_SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../source/*.cpp"
)

# Exclude precompiled-header stub and Assimp-dependent importer
list(FILTER ALL_SRC_FILES EXCLUDE REGEX ".*pch\\.cpp$")
list(FILTER ALL_SRC_FILES EXCLUDE REGEX ".*MeshImporter\\.cpp$")

file(GLOB_RECURSE VENDOR_SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/*.cpp"
)

add_executable(sandbox
    ${ALL_SRC_FILES}
    ${VENDOR_SRC_FILES}
)

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../source
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/include/imgui
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(sandbox PRIVATE
    PLATFORM_WASM
    GLEW_STATIC
)

# ── Emscripten link flags ─────────────────────────────────────────────────────
set(EM_LINK_FLAGS
    "-s USE_GLFW=3"
    "-s USE_WEBGL2=1"
    "-s FULL_ES3=1"
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s WASM_MEM_MAX=512MB"
    "-s ASYNCIFY=1"
    "-s EXPORTED_FUNCTIONS=['_main','_malloc','_free','_OnFileLoadedFromBrowser','_OnFileSavedFromBrowser']"
    "-s EXPORTED_RUNTIME_METHODS=['UTF8ToString','stringToUTF8','lengthBytesUTF8','FS']"
)

# Preload asset folders when they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Assets")
    list(APPEND EM_LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../Assets@/Assets")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../resources")
    list(APPEND EM_LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../resources@/resources")
endif()

# Use minimal shell for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND EM_LINK_FLAGS "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell_minimal.html")
endif()

list(JOIN EM_LINK_FLAGS " " EM_LINK_FLAGS_STR)

set_target_properties(sandbox PROPERTIES
    OUTPUT_NAME  "sandbox"
    SUFFIX       ".html"
    LINK_FLAGS   "${EM_LINK_FLAGS_STR}"
)
