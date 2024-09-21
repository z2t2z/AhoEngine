workspace "AhoEngine"
    architecture "x64"
    startproject "AhoEditor"

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
IncludeDir["glm"] = "AhoEngine/Vendor/glm"
IncludeDir["entt"] = "AhoEngine/Vendor/entt/include"
IncludeDir["assimp"] = "AhoEngine/Vendor/assimp/assimp/include"
IncludeDir["stb_image"] = "AhoEngine/Vendor/stb_image"

group "Dependencies"
    include "AhoEngine/Vendor/GLFW"
    include "AhoEngine/Vendor/Glad"
    include "AhoEngine/Vendor/ImGui"
    include "AhoEngine/Vendor/assimp"
group ""

project "AhoEngine"
    location "AhoEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "Ahopch.h"
    pchsource "AhoEngine/Source/Ahopch.cpp"

    files {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp",
        "%{prj.name}/Vendor/glm/glm/**.inl",
        "%{prj.name}/Vendor/glm/glm/**.hpp",
        "%{prj.name}/Vendor/stb_image/**.h",
        "%{prj.name}/Vendor/stb_image/**.cpp"
    }
	
    defines {
		"_CRT_SECURE_NO_WARNINGS"
	}

    includedirs {
        "%{prj.name}/Vendor/spdlog/include",
        "%{prj.name}/Source",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.stb_image}",
        "AhoEditor/Source/Camera"
    }

    links {
        "GLFW",
        "Glad",
        "ImGui",
        "assimp",
        "opengl32.lib"
    }

    filter "system:windows" 
        systemversion "latest"

        defines {
			"AHO_ENABLE_ASSERTS",
            "AHO_PLATFORM_WINDOWS",
            "AHO_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        buildoptions "/MDd"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        runtime "Release"
        symbols "on"

    filter "configurations:Dist"
        defines "AHO_DIST"
        runtime "Release"
        optimize "on"

    
project "AhoEditor"
    location "AhoEditor"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++20"
	staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs {
        "%{IncludeDir.ImGui}",
        "AhoEngine/Vendor/spdlog/include",
        "AhoEngine/Source",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}",
    }

    links {
        "AhoEngine"
    }

    filter "system:windows" 
        systemversion "latest"

        defines {
            "AHO_PLATFORM_WINDOWS",            
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        runtime "Debug"
        buildoptions "/MDd"
        symbols "On"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        runtime "Release"
        symbols "on"

    filter "configurations:Dist"
        defines "AHO_DIST"
        runtime "Release"
        optimize "on"


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs {
        "%{IncludeDir.ImGui}",
        "AhoEngine/Vendor/spdlog/include",
        "AhoEngine/Source",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}",
    }

    links {
        "AhoEngine"
    }

    filter "system:windows" 
        systemversion "latest"

        defines {
            "AHO_PLATFORM_WINDOWS",            
        }

    filter "configurations:Debug"
        defines "AHO_DEBUG"
        runtime "Debug"
        buildoptions "/MDd"
        symbols "On"
    
    filter "configurations:Release"
        defines "AHO_RELEASE"
        runtime "Release"
        symbols "on"

    filter "configurations:Dist"
        defines "AHO_DIST"
        runtime "Release"
        optimize "on"        