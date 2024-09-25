#pragma once
#include "../handlers/EditorHandler.hpp"

int main(int argc, char* argv[]) {
	handlers::EditorHandler editorHandler;
	editorHandler.init();
	editorHandler.run();
	editorHandler.cleanUp();
	return 0;
}