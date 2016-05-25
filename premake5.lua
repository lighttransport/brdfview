-- premake5.lua
sources = { "./src/**.h", "./src/**.cpp" }

workspace "BRDFViewWorkspace"
  configurations { "debug", "release" }
  language "C++"
  basedir "build"

  includedirs { "/usr/local/include" }
  libdirs { "/usr/local/lib" }

  configuration { "macosx", "gmake" }
    buildoptions { '-std=c++11' }
    links { "GLEW", "glfw3" }
    linkoptions { '-framework OpenGL' }  -- gl
    links { "pthread" }

  configuration { "linux", "gmake" }
    buildoptions { '-std=c++11' }
    links { "GLEW", "glfw", "GLU", "GL" }  -- gl

    -- for CentOS
    --links { "GLEW", "glfw3", "GLU", "GL" }  -- gl
    --links { "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama" }
    links { "pthread" }

  configuration { "windows", "gmake" }
    -- Assume MinGW
    buildoptions { '-std=c++11' }
    links { "GLEW", "glfw3" }


  -- Configuration
  configuration "debug"
    defines { "DEBUG" }
    flags { "Symbols" }

  configuration "release"
    defines { "NDEBUG" }
    flags { "Optimize" }

-- Project
project "viewer"
  kind "ConsoleApp"
  files { sources }
