workspace "ndi-compressor"
	startproject "ndi-compressor"
	architecture "x86_64"
	
	configurations
	{
		"Debug",
		"Release",
		"Optik"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}
	
	
project "ndi-compressor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{prj.location}/bin/%{prj.name}-%{cfg.system}-%{cfg.architecture}/")
	objdir ("%{prj.location}/bin-int/%{prj.name}-%{cfg.system}-%{cfg.architecture}/")

	includedirs
	{
		"%{prj.location}/vendor/sockpp/include/",
		"%{prj.location}/vendor/libyaml/include/",
		"%{prj.location}/vendor/Processing-NDI-Lib/include/",
		"%{prj.location}/vendor/avlibs/include/"
	}
	
	libdirs
	{
		"%{prj.location}/vendor/sockpp/bin/sockpp-%{cfg.system}-%{cfg.architecture}/",
		"%{prj.location}/vendor/libyaml/bin/libyaml-%{cfg.system}-%{cfg.architecture}/",
		"%{prj.location}/vendor/Processing-NDI-Lib/lib/",
		"%{prj.location}/vendor/avlibs/lib/"
	}
	
	filter "system:windows"
		systemversion "latest"
		links
		{
			"ws2_32",
			"sockpp",
			"avcodec",
			"avutil",
			"swscale",
			"libyaml",
			"Processing.NDI.Lib.x64"
		}
		
	filter "system:not_windows"
		links
		{
			"sockpp",
			"avcodec",
			"avutil",
			"swscale",
			"libyaml",
			"Processing.NDI.Lib.x64"
		}
	
	filter "configurations:Debug"
		defines "__DEBUGLOG"
		runtime "Debug"
		symbols "on"
		files
		{
			"src/**.h",
			"src/**.cpp"
		}

	
	filter "configurations:Release"
		defines "NDEBUG"
		runtime "Release"
		optimize "on"
		files
		{
			"src/**.h",
			"src/**.cpp"
		}

	filter "configurations:Optik"
		defines "__PROFILE"
		runtime "Release"
		optimize "on"
		files
		{
			"src/**.h",
			"src/**.cpp",
			"vendor/optik/**.h",
			"vendor/optik/**.cpp"
		}
		
include "vendor/sockpp/"
include "vendor/libyaml/"