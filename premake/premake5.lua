workspace "Sandbox"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "Sandbox"
    location "../"

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

    -- Enable multiprocessor compilation for Visual Studio
    filter "action:vs*"
        buildoptions { "/MP" }
    filter {}

    filter "configurations:Debug"
        symbols "on"
        -- Enable Edit and Continue for hot reload (/ZI)
        editandcontinue "On"
        optimize "Off"
        flags { "NoIncrementalLink" }
        removeflags { "NoIncrementalLink" }

    filter "configurations:Release"
        optimize "on"
        editandcontinue "Off"
