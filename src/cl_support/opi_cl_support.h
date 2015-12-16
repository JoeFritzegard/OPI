/* OPI: Orbital Propagation Interface
* Copyright (C) 2014 Institute of Aerospace Systems, TU Braunschweig, All rights reserved.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 3.0 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library.
*/
#include "../OPI/internal/opi_gpusupport.h"

#include <CL/cl.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

class ClSupportImpl :
	public OPI::GpuSupport
{
public:
	ClSupportImpl();
	~ClSupportImpl();

	virtual void init();

	virtual void copy(void* a, void* b, size_t size, bool host_to_device);
	virtual void allocate(void** a, size_t size);
	virtual void free(void* mem);
	virtual void shutdown();
	virtual void selectDevice(int device);
	virtual int getCurrentDevice();
	virtual std::string getCurrentDeviceName();
	virtual int getCurrentDeviceCapability();
	virtual int getDeviceCount();
	virtual cudaDeviceProp* getDeviceProperties(int device);

	inline cl_context getOpenCLContext() { return context; }
	inline cl_command_queue getOpenCLQueue() { return defaultQueue; }
	inline cl_device_id getOpenCLDevice() { return devices[currentDevice]; }
	inline cl_device_id* getOpenCLDeviceList() { return devices; }

private:
	cudaDeviceProp* CUDAProperties;

	cl_context context;
	cl_command_queue defaultQueue;
	cl_device_id* devices;
	cl_uint nDevices;
	int currentDevice;
};
