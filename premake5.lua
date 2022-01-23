workspace "Game"
    architecture "x64"

    configurations {
        "Debug",
        "Release"
    }

output_dir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

project "Game"
    location "projects"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"

    targetdir ("builds/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("builds/obj/" .. output_dir .. "/%{prj.name}")

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "include"
    }

    links {
        "lib/raylib",
        "winmm",
        "gdi32"
    }