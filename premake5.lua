workspace "Yume"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

    startproject "Box"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Yume"
    location "Yume"
    kind "StaticLib"
    language "C++"
    staticruntime "On"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "ympch.h"
    pchsource "%{prj.name}/src/ympch.cpp"

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/vendor/DirectX-Headers/include",
        "vendor/DirectXTK/Inc"
    }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

        defines
        {
            "YM_PLATFORM_WINDOWS",
        }

    filter "configurations:Debug"
        defines 
        {   
            "YM_DEBUG",
            "YM_ENABLE_ASSERTS",
        }
        runtime "Debug"
        symbols "On"

        libdirs {
            "vendor/DirectXTK/Bin/x64/Debug"
        }
    
    filter "configurations:Release"
        defines "YM_RELEASE"
        runtime "Release"
        optimize "On"

        libdirs {
            "vendor/DirectXTK/Bin/x64/Release"
        }


project "Box"
    location "Demos/Box"
    language "C++"
    staticruntime "On"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Demos/%{prj.name}/src/**.h",
        "Demos/%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Yume/vendor/spdlog/include",
        "Yume/vendor/DirectX-Headers/include",
        "Yume/src",
        "vendor/DirectXTK/Inc"
    }

    links
    {
        "Yume"
    }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

        defines
        {
            "YM_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines "YM_DEBUG"
        runtime "Debug"
        symbols "On"

        libdirs {
            "vendor/DirectXTK/Bin/x64/Debug"
        }
    
    filter "configurations:Release"
        kind "WindowedApp"
        defines "YM_RELEASE"
        runtime "Release"
        optimize "On"

        libdirs {
            "vendor/DirectXTK/Bin/x64/Release"
        }

project "Shapes"
    location "Demos/Shapes"
    language "C++"
    staticruntime "On"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "Demos/%{prj.name}/src/**.h",
        "Demos/%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Yume/vendor/spdlog/include",
        "Yume/vendor/DirectX-Headers/include",
        "Yume/src",
        "vendor/DirectXTK/Inc"
    }

    links
    {
        "Yume"
    }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"

        defines
        {
            "YM_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines "YM_DEBUG"
        runtime "Debug"
        symbols "On"

        libdirs {
            "vendor/DirectXTK/Bin/x64/Debug"
        }
    
    filter "configurations:Release"
        kind "WindowedApp"
        defines "YM_RELEASE"
        runtime "Release"
        optimize "On"

        libdirs {
            "vendor/DirectXTK/Bin/x64/Release"
        }