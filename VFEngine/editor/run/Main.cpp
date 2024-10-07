#pragma once
#include "../handlers/EditorHandler.hpp"

int main(int argc, char* argv[]) {

	//need to create luncher in javaFX and send path to the project
	//need to read the project config and set the level and content path
	handlers::EditorHandler editorHandler;
	editorHandler.init();
	editorHandler.run();
	editorHandler.cleanUp();
	return 0;
}