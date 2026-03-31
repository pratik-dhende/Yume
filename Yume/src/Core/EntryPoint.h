#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "Application.h"

int main() {
	try
    {
        auto app = Yume::CreateApplication();
		app->Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}