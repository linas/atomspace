
#define CL_HPP_MINIMUM_OPENCL_VERSION 200
#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CL/opencl.hpp>
#include <iostream>
#include <fstream>

void use_dev(cl::Device ocldev)
{
	std::string dname = ocldev.getInfo<CL_DEVICE_NAME>();
	std::string dvers = ocldev.getInfo<CL_DEVICE_VERSION>();
	printf("Using device %s\n", dname.c_str());
	printf("\tVersion %s\n", dvers.c_str());

#if 0
	std::ifstream helloWorldFile("hello.cl");
	std::string src(std::istreambuf_iterator<char>(helloWorldFile), (std::istreambuf_iterator<char>()));

	cl::Program::Sources sources( 1, std::make_pair(src.c_str(), src.length() + 1));

	cl::Context context(device);
	cl::Program program(context, sources);

	auto err = program.build("-cl-std=CL1.2");

	char buf[16];
	cl::Buffer memBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, sizeof(buf));
	cl::Kernel kernel(program, "HelloWorld", &err);
	kernel.setArg(0, memBuf);

	cl::CommandQueue queue(context, device);
	queue.enqueueTask(kernel);
	queue.enqueueReadBuffer(memBuf, CL_TRUE, 0, sizeof(buf), buf);

	std::cout << "hello";
	std::cin.get();
#endif
}

int main(int argc, char* argv[])
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	printf("Got %d plaforms\n", platforms.size());

	cl::Platform oclplat;
	cl::Device ocldev;
	for (const auto& plat : platforms)
	{
		std::string pname = plat.getInfo<CL_PLATFORM_NAME>();
		std::string pvend = plat.getInfo<CL_PLATFORM_VENDOR>();
		std::string pvers = plat.getInfo<CL_PLATFORM_VERSION>();
		printf("Platform %s\n", pname.c_str());
		printf("\tVendor %s\n", pvend.c_str());
		printf("\tVersion %s\n", pvers.c_str());

		std::vector<cl::Device> devices;
		// plat.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		plat.getDevices(CL_DEVICE_TYPE_GPU, &devices);

		printf("\tThis platform has %d GPU devices\n", devices.size());

		for (const auto& dev: devices)
		{
			std::string dname = dev.getInfo<CL_DEVICE_NAME>();
			std::string dvers = dev.getInfo<CL_DEVICE_VERSION>();
			printf("\tDevice %s\n", dname.c_str());
			printf("\t\tVersion %s\n", dvers.c_str());

			// hacky
			oclplat = plat;
			ocldev = dev;
		}

		// I aint got no version 3.
		// if (pvers.find("OpenCL 3.") != std::string::npos)
		// 	oclplat = plat;
	}
	printf("\n\n");

	use_dev(ocldev);
}
