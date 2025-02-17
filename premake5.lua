workspace "Yume"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

    startproject "Box"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Yume"
include "Demos/Box"
include "Demos/Shapes"