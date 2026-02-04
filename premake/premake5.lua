workspace "Sandbox"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "Sandbox"
    location "../"

-- Check for vcpkg integration
local vcpkgInstalled = os.getenv("VCPKG_ROOT") ~= nil or os.isfile(path.join(os.getenv("USERPROFILE") or "", "vcpkg", "vcpkg.exe"))

if vcpkgInstalled then
    print("vcpkg detected - Assimp will be linked automatically via vcpkg integration")
end

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("../build/bin/" .. "%{cfg.buildcfg}")
    objdir ("../build/bin-int/" .. "%{cfg.buildcfg}")

    -- Precompiled header configuration
    pchheader "pch.h"
    pchsource "../source/pch.cpp"

    files {
        "../source/**.h",
        "../source/**.cpp",
        "../vendor/**.h",
        "../vendor/**.cpp"
    }

    -- Exclude vendor files from using precompiled header
    filter "files:../vendor/**"
        flags { "NoPCH" }
    filter {}

    includedirs {
        "../vendor/include/",
        "../vendor/include/imgui/",
        "../source/"
    }

    libdirs {
        "../vendor/libs/"
    }

    links {
        "opengl32.lib" -- Manually linked system library
    }

    -- Automatically add all .lib files from vendor/libs/
    local libFiles = os.matchfiles("../vendor/libs/*.lib")
    for _, lib in ipairs(libFiles) do
        links { path.getname(lib) }
    end

    defines {
        "GLEW_STATIC"
    }

    -- vcpkg configuration
    if vcpkgInstalled then
        print("Configuring Assimp via vcpkg...")
        -- When using vcpkg integrate install, libraries are automatically linked
        -- Just need to ensure the vcpkg toolchain is being used
    else
        print("WARNING: vcpkg not detected!")
    end
       
