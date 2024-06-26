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

	--I give up, these need to be the same as KantoEngine project. src and vendor should be enough but no...
	includedirs
	{
		--"{/src}",
		"%{wks.location}/KantoEngine/vendor/spdlog/include",
		"%{wks.location}/KantoEngine/src",
		"%{wks.location}/KantoEngine/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.nfd}",
		"%{IncludeDir.ImGui}",
		"%{wks.location}/KantoEngine/vendor/GLFW/include",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
	}

	links
	{
		"ImGui",
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
