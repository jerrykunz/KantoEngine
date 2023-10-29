project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/KantoEngine/vendor/spdlog/include",
		"%{wks.location}/KantoEngine/src",
		"%{wks.location}/KantoEngine/vendor",
		"%{IncludeDir.glm}",
		--"%{IncludeDir.entt}"
	}

	links
	{
		"KantoEngine"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "KN_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "KN_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "KN_DIST"
		runtime "Release"
		optimize "on"
