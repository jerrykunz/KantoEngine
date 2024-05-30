#pragma once

namespace Kanto
{

	namespace RendererUtils {

		struct ResourceAllocationCounts
		{
			uint32_t Samplers = 0;
		};

		ResourceAllocationCounts& GetResourceAllocationCounts();
	}
}
