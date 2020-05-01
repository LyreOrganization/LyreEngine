workspace "LyreEngine"
    architecture "x86_64"
    configurations { "Debug", "Release" }

    startproject "Launch"

outputdir = "%{cfg.architecture}_%{cfg.system}_%{cfg.buildcfg}"

project "LyreEngine"
    location "code/%{prj.name}"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

	pchheader "LyrePch.h"
	pchsource "code/%{prj.name}/src/LyrePch.cpp"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin/objs/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.location}/src/**.h",
        "%{prj.location}/src/**.cpp"
    }

	includedirs {
		"%{prj.location}/src"
	}

    filter "system:windows"
        defines { "LYRE_PLATFORM_WINDOWS" }
        systemversion "latest"
        
        files {
            "%{prj.location}/platform/win64/**",
            "%{prj.location}/platform/DirectX/**"
        }

		includedirs {
			"%{prj.location}/platform/win64",
			"%{prj.location}/platform/DirectX"
		}

    filter "configurations:Debug"
        defines { "LYRE_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "LYRE_RELEASE" }
        optimize "On"

project "Launch"
    location "code/%{prj.name}"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin/objs/" .. outputdir .. "/%{prj.name}")
    
    files {
        "%{prj.location}/src/**.h",
        "%{prj.location}/src/**.cpp"
    }

    includedirs {
        "code/LyreEngine/src"
    }

	links {
		"LyreEngine"
	}

    filter "system:windows"
        defines { "LYRE_PLATFORM_WINDOWS" }
		systemversion "latest"
        files {
            "%{prj.location}/entry/win64/**.h",
            "%{prj.location}/entry/win64/**.cpp"
        }
		includedirs {
			"code/LyreEngine/platform/win64"
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