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
IncludeDir["ImGui"] = "AhoEngine/Vendor/ImGui"
IncludeDir["GLFW"] = "AhoEngine/Vendor/GLFW/include"
IncludeDir["Glad"] = "AhoEngine/Vendor/Glad/include"

include "AhoEngine/Vendor/ImGui"
include "AhoEngine/Vendor/GLFW"
include "AhoEngine/Vendor/Glad"

project "AhoEngine"
    location "AhoEngine"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Ahopch.h"
    pchsource "AhoEngine/Source/Ahopch.cpp"

    files {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs {
        "%{prj.name}/Vendor/spdlog/include",
        "%{prj.name}/Source",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
    }

    links {
        "ImGui",
        "GLFW",
        "Glad",
        "opengl32.lib"
    }

    dependson { "ImGui" }

    filter "system:windows" 
        cppdialect "C++20"
        staticruntime "Off"
        systemversion "latest"

        defines {
            "AHO_PLATFORM_WINDOWS",
            "AHO_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
            -- "_DEBUG", ?
            -- "_CONSOLE"
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        buildoptions "/MDd"
        symbols "On"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        buildoptions "/MD"
        symbols "On"

    filter "configurations:Dist"
        defines "AHO_DIST"
        buildoptions "/MD"
        optimize "On"

    
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

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
            "{COPY} ../bin/" .. outputdir .. "/AhoEngine/AhoEngine.dll %{cfg.targetdir}"
            -- ("{COPYFILE} %{cfg.buildtarget.relpath} ../Bin" .. outputdir .. "/Sandbox"),
            -- "echo Copying DLL to ../Bin/" .. outputdir .. "/Sandbox"
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        buildoptions "/MDd"
        symbols "On"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        buildoptions "/MD"
        symbols "On"

    filter "configurations:Dist"
        defines "AHO_DIST"
        buildoptions "/MD"
        optimize "On"