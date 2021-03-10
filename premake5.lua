workspace "ndi-compressor"
	architecture "x86_64"
	startproject "ndi-compressor"
	
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
		"%{prj.location}/vendor/include"
	}
	
	links
	{ 
		"ws2_32",
		"Processing.NDI.Lib.x64"
	}
	
	libdirs
	{ 
		"%{prj.location}/vendor/lib" 
	}

	filter "system:windows"
		systemversion "latest"

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