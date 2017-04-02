#include <iostream>
#include "app.h"

int main(const int argc, const char* argv[]) {
	try {
		mark::app app(argc, argv);
		app.main();
	} catch (std::exception& error) {
		std::cout << "ERROR: " << error.what() << std::endl;
		return 1;
	}
	return 0;
}