#pragma once
/*
#include "Asset.h"

#include <filesystem>

namespace Kanto
{

	struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type;

		std::filesystem::path FilePath;
		bool IsDataLoaded = false;
		bool IsMemoryAsset = false;

		bool IsValid() const { return Handle != 0 && !IsMemoryAsset; }
	};
}
