#pragma once

#include "Kanto/Asset/Asset.h"

namespace Kanto 
{

	using ResourceDescriptorInfo = void*;

	class RendererResource : public Asset
	{
	public:
		virtual ResourceDescriptorInfo GetDescriptorInfo() const = 0;
	};

}
