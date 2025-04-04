workspace "OpenGL"
    configurations { "Debug", "Release" }
    architecture "x86"
    startproject "OpenGL"

project "OpenGL"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    -- 输出目录
    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/%{cfg.buildcfg}")

    -- 源代码文件
    files { "OpenGL/src/**.h", "OpenGL/src/**.cpp" }

    -- 头文件包含目录
    includedirs {
        "OpenGL/src",
        "Dependencies/GLFW/include" -- GLFW 头文件路径
    }

    -- 库文件目录
    libdirs { 
        "Dependencies/GLFW/lib-vc2022"
    }

    -- 连接的库
    links { "glfw3", "opengl32" } -- Windows 需要链接 OpenGL

    -- Windows 配置
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }

    -- Debug 配置
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    -- Release 配置
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
