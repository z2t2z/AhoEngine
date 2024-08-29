workspace "AhoEngine"
    architecture "x64"
    startproject "Sandbox"

    configurations {
        "Debug",
        "Release", 
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "AhoEngine/Vendor/GLFW/include"

include "AhoEngine/Vendor/GLFW"


project "AhoEngine"
    location "AhoEngine"
    kind "SharedLib"
    language "C++"

    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    pchheader "Ahopch.h"
    pchsource "AhoEngine/Source/Ahopch.cpp"

    files {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs {
        "%{prj.name}/Vendor/spdlog/include",
        "%{prj.name}/Source",
        "%{IncludeDir.GLFW}"
    }

    links {
        "GLFW",
        "opengl32.lib"
    }

    filter "system:windows" 
        cppdialect "C++20"
        staticruntime "Off"
        systemversion "latest"

        defines {
            "AHO_PLATFORM_WINDOWS",
            "AHO_BUILD_DLL",
            -- "_DEBUG", ?
            -- "_CONSOLE"
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        symbols "On"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "AHO_DIST"
        optimize "On"

    
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Bin-Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs {
        "AhoEngine/Vendor/spdlog/include",
        "AhoEngine/Source"
    }

    links {
        "AhoEngine"
    }

    filter "system:windows" 
        cppdialect "C++20"
        staticruntime "Off"
        systemversion "latest"

        defines {
            "AHO_PLATFORM_WINDOWS",
            -- "_DEBUG",
            -- "_CONSOLE"                
        }

        postbuildcommands {
            "{COPY} ../Bin/" .. outputdir .. "/AhoEngine/AhoEngine.dll %{cfg.targetdir}"
            -- ("{COPYFILE} %{cfg.buildtarget.relpath} ../Bin" .. outputdir .. "/Sandbox"),
            -- "echo Copying DLL to ../Bin/" .. outputdir .. "/Sandbox"
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        symbols "On"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        symbols "On"

    filter "configurations:Dist"
        defines "AHO_DIST"
        optimize "On"