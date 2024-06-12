workspace "Yume"
    architecture "x64"

    configurations
    {
        "Debug",
        "Stage",
        "Release"
    }

    startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

project "Yume"
    location "Yume"
    kind "SharedLib"
    language "C++"

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
        "%{prj.name}/vendor/spdlog/include"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "YM_PLATFORM_WINDOWS",
            "YM_BUILD_DLL"
        }

        postbuildcommands
        {
            ("{COPYFILE} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
        }

    filter "configurations:Debug"
        defines 
        {   
            "YM_DEBUG",
            "YM_ENABLE_ASSERTS",
        }
        symbols "On"

    filter "configurations:Stage"
        defines "YM_STAGE"
        optimize "On"
    
    filter "configurations:Release"
        defines "YM_RELEASE"
        optimize "On"


project "Sandbox"
    location "Sandbox"
    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Yume/vendor/spdlog/include",
        "Yume/src"
    }

    links
    {
        "Yume"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "YM_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        kind "ConsoleApp"
        defines "YM_DEBUG"
        symbols "On"

    filter "configurations:Stage"
        kind "WindowedApp"
        defines "YM_STAGE"
        optimize "On"
    
    filter "configurations:Release"
        kind "WindowedApp"
        defines "YM_RELEASE"
        optimize "On"