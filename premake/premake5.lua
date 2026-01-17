workspace "Sandbox"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "Sandbox"
    location "../"

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("../build/bin/" .. "%{cfg.buildcfg}")
    objdir ("../build/bin-int/" .. "%{cfg.buildcfg}")

    -- Precompiled header configuration
    pchheader "pch.h"
    pchsource "../source/app/pch.cpp"

    files {
        "../source/**.h",
        "../source/**.cpp",
        "../vendor/**.h",
        "../vendor/**.cpp"
    }

    includedirs {
        "../vendor/include/",
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

    filter "configurations:Debug"
        symbols "on"

    filter "configurations:Release"
        optimize "on"
