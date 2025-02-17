project "Shapes"
    language "C++"
    staticruntime "On"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Yume/vendor/spdlog/include",
        "%{wks.location}/Yume/vendor/DirectX-Headers/include",
        "%{wks.location}/Yume/src",
        "%{wks.location}/vendor/DirectXTK/Inc"
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
            "%{wks.location}/vendor/DirectXTK/bin-v/x64/Debug"
        }
    
    filter "configurations:Release"
        kind "WindowedApp"
        defines "YM_RELEASE"
        runtime "Release"
        optimize "On"

        libdirs {
            "%{wks.location}/vendor/DirectXTK/bin-v/x64/Release"
        }