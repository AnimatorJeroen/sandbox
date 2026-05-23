cmake_minimum_required(VERSION 3.20)
project(sandbox CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source files ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE ALL_SRC_FILES CONFIGURE_DEPENDS
    "source/*.cpp"
)

# Exclude precompiled-header stub and Assimp-dependent importer
list(FILTER ALL_SRC_FILES EXCLUDE REGEX ".*pch\\.cpp$")
list(FILTER ALL_SRC_FILES EXCLUDE REGEX ".*MeshImporter\\.cpp$")

file(GLOB_RECURSE VENDOR_SRC_FILES CONFIGURE_DEPENDS
    "vendor/*.cpp"
)

add_executable(sandbox
    ${ALL_SRC_FILES}
    ${VENDOR_SRC_FILES}
)

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(sandbox PRIVATE
    source
    vendor/include
    vendor/include/imgui
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
)

# Preload asset folders when they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/Assets")
    list(APPEND EM_LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/Assets@/Assets")
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/resources")
    list(APPEND EM_LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/resources@/resources")
endif()

list(JOIN EM_LINK_FLAGS " " EM_LINK_FLAGS_STR)

set_target_properties(sandbox PROPERTIES
    OUTPUT_NAME  "sandbox"
    SUFFIX       ".html"
    LINK_FLAGS   "${EM_LINK_FLAGS_STR}"
)
