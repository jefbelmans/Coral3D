#include "first_app.h"

// STD
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	coral_3d::first_app app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}