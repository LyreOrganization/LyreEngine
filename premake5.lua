workspace "LyreEngine"
    architecture "x64"
    configurations { "Debug", "Release" }

    startproject "Launch"

outputdir = "%{cfg.architecture}_%{cfg.system}_%{cfg.buildcfg}"

dependencies = {}
dependencies["glm"] = "External/glm";

project "LyreEngine"
    sources = "Source/%{prj.name}"

    location "Intermediate/"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

	pchheader "LyrePch.h"
	pchsource "Source/%{prj.name}/LyrePch.cpp"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "%{sources}/**.h",
        "%{sources}/**.cpp",
        "%{sources}/**.fx"
    }

	includedirs {
		"%{sources}",
		dependencies["glm"]
	}

    filter "system:windows"
        defines { "LYRE_PLATFORM_WINDOWS" }
        systemversion "latest"
        
        files {
            "%{sources}/Platform/win64/**",
            "%{sources}/Platform/DirectX/**"
        }

		includedirs {
			"%{sources}/Platform/win64",
			"%{sources}/Platform/DirectX"
		}

    filter "configurations:Debug"
        defines { "LYRE_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "LYRE_RELEASE" }
        optimize "On"

project "Launch"
    sources = "Source/%{prj.name}"

    location "Intermediate/"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
    objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")
    
    files {
        "%{sources}/**.h",
        "%{sources}/**.cpp"
    }

    includedirs {
        "Source/LyreEngine"
    }

	links {
		"LyreEngine"
	}

    filter "system:windows"
        defines { "LYRE_PLATFORM_WINDOWS" }
		systemversion "latest"
        files {
            "%{sources}/Entry/win64/**.h",
            "%{sources}/Entry/win64/**.cpp"
        }
		includedirs {
			"Source/LyreEngine/Platform/win64"
		}
		links {
			"d3d11"
		}

    filter "configurations:Debug"
        defines { "LYRE_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "LYRE_RELEASE" }
        optimize "On"