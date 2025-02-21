workspace "Yume"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

    startproject "Box"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Engine"
    include "Yume"
group ""

group "Demos"
    include "Demos/Box"
group ""