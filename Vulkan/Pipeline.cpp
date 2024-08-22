/***********************************************************************
Pipeline - Base class representing Vulkan pipelines.
Copyright (c) 2022-2024 Oliver Kreylos

This file is part of the Vulkan C++ Wrapper Library (Vulkan).

The Vulkan C++ Wrapper Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vulkan C++ Wrapper Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vulkan C++ Wrapper Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Vulkan/Pipeline.h>

#include <utility>
#include <Vulkan/Device.h>

namespace Vulkan {

/*************************
Methods of class Pipeline:
*************************/

Pipeline::Pipeline(Device& sDevice)
	:DeviceAttached(sDevice),
	 pipeline(VK_NULL_HANDLE)
	{
	}

Pipeline::Pipeline(Pipeline&& source)
	:DeviceAttached(std::move(source)),
	 pipeline(source.pipeline)
	{
	/* Invalidate the source: */
	source.pipeline=VK_NULL_HANDLE;
	}

Pipeline::~Pipeline(void)
	{
	/* Destroy the pipeline: */
	vkDestroyPipeline(device.getHandle(),pipeline,0);
	}

}
