#pragma once
#include "CoreInterface.hpp"

int main(int argc, char* argv[]) {
	interface::CoreInterface coreInterface;
	coreInterface.init();
	coreInterface.run();
	coreInterface.cleanUp();
	return 0;
}