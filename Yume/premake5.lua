project "Yume"
    kind "StaticLib"
    language "C++"
    staticruntime "On"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "ympch.h"
    pchsource "src/ympch.cpp"

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "vendor/spdlog/include",
        "vendor/DirectX-Headers/include",
        "%{wks.location}/vendor/DirectXTK/Inc"
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
            "%{wks.location}/vendor/DirectXTK/bin-v/x64/Debug"
        }
    
    filter "configurations:Release"
        defines "YM_RELEASE"
        runtime "Release"
        optimize "On"

        libdirs {
            "%{wks.location}/vendor/DirectXTK/bin-v/x64/Release"
        }