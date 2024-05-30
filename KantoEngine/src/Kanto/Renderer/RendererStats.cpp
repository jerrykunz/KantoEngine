#include "knpch.h"
#include "RendererStats.h"

namespace Kanto
{
	namespace RendererUtils
	{

		static ResourceAllocationCounts s_ResourceAllocationCounts;
		ResourceAllocationCounts& GetResourceAllocationCounts()
		{
			return s_ResourceAllocationCounts;
		}

	}

}
