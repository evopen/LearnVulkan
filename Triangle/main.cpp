#include <vulkan/vulkan.h>
#include <exception>
#include <iostream>

class HelloTriangleApplication
{
public:
	void run()
	{
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan()
	{
	}

	void mainLoop()
	{
	}

	void cleanup()
	{
	}
};

int main(int argc, char* argv[])
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
