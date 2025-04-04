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
        "Dependencies/GLFW/include", -- GLFW 头文件路径
        "Dependencies/GLEW/include" -- GLEW 头文件路径
    }

    -- 库文件目录
    libdirs { 
        "Dependencies/GLFW/lib-vc2022",
        "Dependencies/GLEW/lib/Release/Win32" -- GLEW 库路径
    }

    -- 连接的库
    links { 
        "glfw3", 
        "opengl32", 
        "glew32s" -- 静态链接 GLEW
    }

    -- Windows 配置
    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS", "GLEW_STATIC" } -- 静态链接 GLEW 需要定义 GLEW_STATIC

    -- Debug 配置
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    -- Release 配置
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
