
----------------------------
-- Project Generator
----------------------------
-- Environment vars
PSDK_DIR = os.getenv("PLUGIN_SDK_DIR")
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
    cppdialect "C++latest"
    characterset "MBCS"
    staticruntime "On"
    location "../build"
    targetdir "../build/bin"

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
            defines {"DEBUG"}
            symbols "On"

        filter "configurations:Release"
            defines {"NDEBUG"}
            optimize "On"


    project "MapEditorSA"
        kind "SharedLib"
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
            "_CRT_SECURE_NO_WARNINGS",
            "_CRT_NON_CONFORMING_SWPRINTFS",
            "GTASA",
            "_GTA_",
            "_DX9_SDK_INSTALLED",
            "PLUGIN_SGV_10US",
            "GTASA", 
            "RW"
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
        