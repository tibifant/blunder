ProjectName = "blunder"
project(ProjectName)

  --Settings
  kind "StaticLib"
  language "C++"
  flags { "FatalWarnings" }
  staticruntime "On"

    cppdialect "C++20"

  filter {"system:windows"}
    buildoptions { '/MP' }
    ignoredefaultlibraries { "msvcrt" }
  
  filter { }
  
  defines { "_CRT_SECURE_NO_WARNINGS", "SSE2", }
  
  objdir "intermediate/obj"

  files { "src/**.c", "src/**.cc", "src/**.cpp", "src/**.cxx", "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl", "src/**rc" }
  files { "include/**.h", "include/**.hh", "include/**.hpp", "include/**.inl" }

  files { "project.lua" }
  
  includedirs { "include**" }
  
  targetname(ProjectName)
  targetdir "../builds/lib"
  debugdir "../builds/lib"

filter {}
warnings "Extra"

filter {"configurations:Release"}
  targetname "%{prj.name}"
filter {"configurations:Debug"}
  targetname "%{prj.name}D"

filter { }
  exceptionhandling "Off"
  rtti "Off"
  floatingpoint "Fast"

filter { "configurations:Debug*" }
	defines { "_DEBUG" }
	optimize "Off"
	symbols "FastLink"

filter { "configurations:Release*" }
	defines { "NDEBUG" }
	optimize "Speed"
	flags { "NoBufferSecurityCheck" }
  omitframepointer "On"
  symbols "On"

filter { "system:windows" }
	defines { "WIN32", "_WINDOWS" }
  flags { "NoPCH", "NoMinimalRebuild" }
 
filter { "system:windows", "configurations:Release" }
  flags { "NoIncrementalLink" }

filter { "system:windows", "configurations:Debug" }
  ignoredefaultlibraries { "libcmt" }
filter { }
