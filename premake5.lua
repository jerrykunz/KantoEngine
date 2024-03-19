include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "KantoEngine"
	architecture "x86_64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	--include "KantoEngine/vendor/Box2D"
	include "KantoEngine/vendor/GLFW"
	include "KantoEngine/vendor/NFD-Extended"
	include "KantoEngine/vendor/Glad"
	--include "KantoEngine/vendor/msdf-atlas-gen"
	include "KantoEngine/vendor/imgui"
	--include "KantoEngine/vendor/yaml-cpp"
group ""

group "Core"
	include "KantoEngine"
	--include "KantoEngine-ScriptCore"
group ""

--group "Tools"
	--include "KantoEnginenut"
--group ""

group "Misc"
	include "Sandbox"
group ""
