# VertexForge

## Introduction
VertexForge is a modular game engine framework built in C++ using modern technologies such as Vulkan, GLFW, ImGui, and spdlog. The project is structured to support the development of real-time applications, with components divided into several sub-projects including core engine functionality, a graphical editor, runtime, utilities, and libraries like Jolt Physics.

## Table of Contents
- [Introduction](#introduction)
- [Table of Contents](#table-of-contents)
- [Installation](#installation)
- [Usage](#usage)
- [Features](#features)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Configuration](#configuration)
- [Contributors](#contributors)
- [License](#license)

## Installation

### Prerequisites
- Ensure you have installed the Vulkan SDK, GLFW, and CMake.
- Set the `VULKAN_SDK` environment variable to the Vulkan SDK installation path.
- Premake5: You will need Premake5 to generate project files for your build system (e.g., Visual Studio, Makefiles).
- CMake (for dependencies like Assimp and OpenAL).

### Steps
1. Clone the repository:
    ```bash
    git clone https://github.com/your-repo/vertexforge.git
    cd vertexforge
    ```
2. Build the project using your preferred IDE (e.g., Visual Studio) or the command line:
    ```bash
    premake5 vs2022
    ```
3. Open the solution or makefile and build the projects in `Debug` or `Release` mode.
4. Ensure all dependencies (Vulkan, GLFW, ImGui, etc.) are available and configured correctly in your build environment.

## Usage
The primary entry points for development are the `Editor` and `Runtime` projects:
- **Editor**: Launches the graphical editor interface.
- **Runtime**: Runs the application using the built engine.

To run the engine, simply build and run the `Editor` or `Runtime` projects.

### Running in Debug/Release Mode
To run in `Debug` or `Release` configurations:
1. Select the desired configuration in your IDE (e.g., `Debug` or `Release`).
2. Build the solution.
3. Execute the `Editor` or `Runtime` project from the `bin` directory.

## Features
- **Modular Engine Architecture**: Split into `Core`, `Graphics`, `Runtime`, and `Utilities` projects for flexible development.
- **Vulkan-based Rendering**: Uses Vulkan for high-performance graphics rendering.
- **ImGui Integration**: Provides a graphical interface for development tools and editors.
- **Physics Engine**: Integrates Jolt Physics for real-time simulation.
- **Cross-Platform Compatibility**: Supports Windows with future plans for other platforms.

## Project Structure
- **VFEngine**
  - `editor`: Graphical editor application.
  - `core`: Core engine functionality.
  - `graphics`: Rendering and graphics pipeline.
  - `runtime`: Main application runtime.
  - `utilities`: Common utilities shared across projects.
  - `dependencies`: External libraries (GLFW, ImGui, Jolt, etc.).
  
## Dependencies
- **Vulkan SDK**: For rendering.
- **GLFW**: For window management and input.
- **ImGui**: For GUI rendering.
- **spdlog**: For logging.
- **Jolt Physics**: Physics simulation library.

### Library Dependencies
- GLFW
- spdLog
- ImGui
- JoltPhysics
- glm
- stb

## Configuration
### Build Configurations
- **Debug**: Includes debug symbols and uses the `DEBUG` preprocessor definition.
- **Release**: Optimized for performance and uses the `NDEBUG` preprocessor definition.

### Environment Variables
Ensure the following environment variable is set:
- `VULKAN_SDK`: Path to your Vulkan SDK installation.

## Contributors
- [Matatn Migdal](https://github.com/matan45) (Lead Developer)


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
