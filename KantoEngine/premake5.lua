project "KantoEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/obj/" .. outputdir .. "/%{prj.name}")

	pchheader "knpch.h"
	pchsource "src/knpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",

		"Platform/" .. firstToUpper(os.target()) .. "/**.hpp",
		"Platform/" .. firstToUpper(os.target()) .. "/**.cpp",

		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/choc/**.h",
		"vendor/yaml-cpp/src/**.cpp",
		"vendor/yaml-cpp/src/**.h",
		"vendor/yaml-cpp/include/**.h",

		--"vendor/ImGuizmo/ImGuizmo.h",
		--"vendor/ImGuizmo/ImGuizmo.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_VULKAN"
	}

	includedirs
	{
		"src",
		"%{IncludeDir.spdlog}",
		--"vendor/spdlog/include",
		"%{IncludeDir.choc}",
		"%{IncludeDir.tracy}",
		"%{IncludeDir.nfd}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.Box2D}",
		--"%{IncludeDir.filewatch}",
		"%{IncludeDir.GLFW}",
		--"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		--"%{IncludeDir.mono}",
		--"%{IncludeDir.yaml_cpp}",
		--"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}",
		--"vendor",
	}

	--should be the same as the dependency project name in VS, capitalization doesn't seem to matter (tracy vs Tracy)
	links
	{
		"Box2D",
		"GLFW",
		"NFD-Extended",
		"Tracy",
		--"Glad",
		"ImGui",
		"%{Library.Vulkan}",
		"msdf-atlas-gen",
		"msdfgen"
	}

	--filter "files:vendor/ImGuizmo/**.cpp"
	--flags { "NoPCH" }
	
	filter "files:vendor/yaml-cpp/src/**.cpp or files:vendor/imgui/misc/cpp/imgui_stdlib.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
		 "KN_PLATFORM_WINDOWS",
		}

		--links
		--{
		--	"%{Library.WinSock}",
		--	"%{Library.WinMM}",
		--	"%{Library.WinVersion}",
		--	"%{Library.BCrypt}",
		--}

	filter "configurations:Debug"
		defines "KN_DEBUG"
		runtime "Debug"
		symbols "on"

		--links
		--{
		--	"%{Library.ShaderC_Debug}",
		--	"%{Library.SPIRV_Cross_Debug}",
		--	"%{Library.SPIRV_Cross_GLSL_Debug}"
		--}

	filter "configurations:Release"
		defines "KN_RELEASE"
		runtime "Release"
		optimize "on"

		--links
		--{
		--	"%{Library.ShaderC_Release}",
		--	"%{Library.SPIRV_Cross_Release}",
		--	"%{Library.SPIRV_Cross_GLSL_Release}"
		--}

	filter "configurations:Dist"
		defines "KN_DIST"
		runtime "Release"
		optimize "on"

		--links
		--{
		--	"%{Library.ShaderC_Release}",
		--	"%{Library.SPIRV_Cross_Release}",
		--	"%{Library.SPIRV_Cross_GLSL_Release}"
		--}
