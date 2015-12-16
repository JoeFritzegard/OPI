#include "OPI/opi_cpp.h"
#include "../src/cl_support/opi_cl_support.h"
#include "CL/cl.h"

#include <typeinfo> //testing

// some plugin information
#define OPI_PLUGIN_NAME "OpenCL Example Propagator"
#define OPI_PLUGIN_AUTHOR "ILR TU BS"
#define OPI_PLUGIN_DESC "A simple test"
// the plugin version
#define OPI_PLUGIN_VERSION_MAJOR 0
#define OPI_PLUGIN_VERSION_MINOR 1
#define OPI_PLUGIN_VERSION_PATCH 0
#include <iostream>
class TestPropagator:
		public OPI::Propagator
{
	public:
		TestPropagator(OPI::Host& host)
		{
			testproperty_int = 0;
			testproperty_float = 0.0f;
			testproperty_double = 42.0;
			testproperty_string = "test";
			for(int i = 0; i < 5; ++i)
				testproperty_int_array[i] = i;
			testproperty_float_array[0] = 4.0f;
			testproperty_float_array[1] = 2.0f;
			for(int i = 0; i < 4; ++i)
				testproperty_double_array[i] = 9.0 * i;
			registerProperty("int", &testproperty_int);
			registerProperty("float", &testproperty_float);
			registerProperty("double", &testproperty_double);
			registerProperty("string", &testproperty_string);
			registerProperty("int_array", testproperty_int_array, 5);
			registerProperty("float_array", testproperty_float_array, 2);
			registerProperty("double_array", testproperty_double_array, 4);
		}

		virtual ~TestPropagator()
		{

		}

		virtual OPI::ErrorCode runPropagation(OPI::Population& data, double julian_day, float dt )
		{
			std::cout << "Test int: " << testproperty_int << std::endl;
			std::cout << "Test float: " <<  testproperty_float << std::endl;
			std::cout << "Test string: " << testproperty_string << std::endl;

			// cl_int for OpenCL error reporting
			cl_int err;

			// Get GPU support module from host and cast it to the OpenCL implementation.
			// This will provide important additional information such as the OpenCL context
			// and default command queue.
			OPI::GpuSupport* gpu = getHost()->getGPUSupport();
			ClSupportImpl* cl = dynamic_cast<ClSupportImpl*>(gpu);

			// define kernel source code
			const char* kernelCode = "\n" \
				"struct Orbit {float semi_major_axis;float eccentricity;float inclination;float raan;float arg_of_perigee;float mean_anomaly;float bol;float eol;}; \n" \
				"__kernel void propagate(__global struct Orbit* orbit, double julian_day, float dt) { \n" \
				"int i = get_global_id(0); \n" \
				"orbit[i].semi_major_axis = 6500.0 + i; \n" \
				"} \n";

			// create kernel program
			cl_program program = clCreateProgramWithSource(cl->getOpenCLContext(), 1, (const char**)&kernelCode, NULL, &err);
			if (err != CL_SUCCESS) std::cout << "Error creating program: " << err << std::endl;

			// build kernel for default device
			err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

			// print build log for debugging purposes
			char buildLog[2048];
			clGetProgramBuildInfo(program, cl->getOpenCLDevice(), CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
			cout << "--- Build log ---\n " << buildLog << endl;

			// create the kernel object
			cl_kernel kernel = clCreateKernel(program, "propagate", &err);
			if (err != CL_SUCCESS) std::cout << "Error creating kernel: " << err << std::endl;

			// Calling getOrbit and getObjectProperties with the DEVICE_CUDA flag will return
			// cl_mem objects in the OpenCL implementation. They must be explicitly cast to
			// cl_mem before they can be used as kernel arguments. This step will also trigger
			// the memory transfer from host to OpenCL device.
			cl_mem orbit = reinterpret_cast<cl_mem>(data.getOrbit(OPI::DEVICE_CUDA));
			err = clSetKernelArg(kernel, 0, data.getSize(), &orbit);
			if (err != CL_SUCCESS) cout << "Error setting population data: " << err << endl;

			// set remaining arguments (julian_day and dt)
			err = clSetKernelArg(kernel, 1, sizeof(double), &julian_day);
			if (err != CL_SUCCESS) cout << "Error setting jd data: " << err << endl;
			err = clSetKernelArg(kernel, 2, sizeof(float), &dt);
			if (err != CL_SUCCESS) cout << "Error setting dt data: " << err << endl;

			// run the kernel
			const size_t s = data.getSize();
			err = clEnqueueNDRangeKernel(cl->getOpenCLQueue(), kernel, 1, NULL, &s, NULL, 0, NULL, NULL);
			if (err != CL_SUCCESS) cout << "Error running kernel: " << err << endl;

			// wait for the kernel to finish
			clFinish(cl->getOpenCLQueue());

			// Don't forget to notify OPI of the updated data on the device!
			data.update(OPI::DATA_ORBIT, OPI::DEVICE_CUDA);
			
			return OPI::SUCCESS;
		}
	private:
		int testproperty_int;
		float testproperty_float;
		double testproperty_double;
		int testproperty_int_array[5];
		float testproperty_float_array[2];
		double testproperty_double_array[4];
		std::string testproperty_string;
};

#define OPI_IMPLEMENT_CPP_PROPAGATOR TestPropagator

#include "OPI/opi_implement_plugin.h"

