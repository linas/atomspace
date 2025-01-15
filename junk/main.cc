
#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CL/opencl.hpp>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	printf("Got %d plaforms\n", platforms.size());

	cl::Platform ocl3plat;
	for (const auto& plat : platforms)
	{
		std::string platname = plat.getInfo<CL_PLATFORM_VERSION>();
		printf("Platform %s\n", platname.c_str());

		std::vector<cl::Device> devices;
		plat.getDevices(CL_DEVICE_TYPE_CPU, &devices);

		printf("\tThis platform has %d devices\n", devices.size());

		if (platname.find("OpenCL 3.") != std::string::npos)
			ocl3plat = plat;
	}

	std::vector<cl::Device> devices;
	ocl3plat.getDevices(CL_DEVICE_TYPE_CPU, &devices);

	printf("Got %d devices\n", devices.size());

#if 0
	auto device = devices.front();

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
