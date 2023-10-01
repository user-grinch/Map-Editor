
----------------------------
-- Project Generator
----------------------------
-- Environment vars
PSDK_DIR = os.getenv("PLUGIN_SDK_DIR")
GTASA_DIR = "D:/Games/GTA San Andreas"
DX9SDK_DIR = os.getenv("DXSDK_DIR")

if (DX9SDK_DIR == nil) then
    error("DIRECTX9_SDK_DIR environment variable not set")
end

if (PSDK_DIR == nil) then
    error("PLUGIN_SDK_DIR environment variable not set")
end
----------------------------

workspace "MapEditorSA"
    configurations { "Debug", "Release" }
    architecture "x86"
    platforms "Win32"
    language "C++"
    cppdialect "C++20"
    characterset "MBCS"
    staticruntime "On"
    location "../build"

    project "depend"
        kind "StaticLib"
        targetdir "../build/bin"

        files { 
            "../include/**",
            "../include/**.h", 
            "../include/**.hpp", 
            "../include/**.c", 
            "../include/**.cpp" 
        }
        libdirs (PSDK_DIR .. "/output/lib")

        filter "configurations:Debug"
            defines { "DEBUG", "IS_PLATFORM_WIN" }
            symbols "On"

        filter "configurations:Release"
            defines { "NDEBUG", "IS_PLATFORM_WIN" }
            optimize "On"


    project "MapEditorSA"
        kind "SharedLib"
        targetdir (GTASA_DIR)
        targetextension ".asi"
        
        files { 
            "../src/**.h", 
            "../src/**.hpp", 
            "../src/**.cpp" 
        }
        includedirs {
            "../include/",
            PSDK_DIR .. "/plugin_sa/",
            PSDK_DIR .. "/plugin_sa/game_sa/",
            PSDK_DIR .. "/shared/",
            PSDK_DIR .. "/shared/game/",
            DX9SDK_DIR .. "/Include"
        }
        libdirs {
            PSDK_DIR .. "/output/lib",
            DX9SDK_DIR .. "/lib/x86"
        }
        
        defines { 
            "NDEBUG", 
            "IS_PLATFORM_WIN",
            "_CRT_SECURE_NO_WARNINGS",
            "_CRT_NON_CONFORMING_SWPRINTFS",
            "GTASA",
            "_DX9_SDK_INSTALLED",
            "PLUGIN_SGV_10US",
            "GTASA"
        }

        pchheader "pch.h"
        pchsource "../src/pch.cpp"

        filter "configurations:Debug"
            symbols "On"
            links { 
                "depend",
                "d3d9",
                "d3dx9",
                "d3d11",
                "urlmon",
                "plugin_d.lib" 
            }

        filter "configurations:Release"
            optimize "On"
            links { 
                "depend",
                "d3d9",
                "d3dx9",
                "d3d11",
                "urlmon",
                "plugin.lib" 
            }
        