workspace "VulkanApp"
   configurations { "Debug", "Release" }
   location "VKEngine"  -- Specify where to place generated files
   startproject "Editor"  -- Set the default startup project


local vulkanLibPath = os.getenv("VULKAN_SDK")
-- Project 1: Editor
project "Editor"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   location "VKEngine/editor"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files { "VKEngine/editor/**.hpp", "VKEngine/editor/**.cpp" }
   
   includedirs {
      "VKEngine/core/interface/CoreInterface.hpp"
   }

   links { "Core" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- Project 2: Core
project "Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   location "VKEngine/core"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files { "VKEngine/core/**.hpp", "VKEngine/core/**.cpp" }
   
   includedirs {
      "VKEngine/graphics/interface/GraphicsInterface.hpp"
   }

   links { "Graphics" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- Project 3: Graphics
project "Graphics"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   location "VKEngine/graphics"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files { "VKEngine/graphics/**.hpp", "VKEngine/graphics/**.cpp" }

   includedirs {
      "dependencies",
      vulkanLibPath.."/Include"
   }

   libdirs {
      vulkanLibPath.."/Lib"
   }

   links {
      "GLFW",
      "spdLog",
      "vulkan-1.lib"
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      links { "shaderc_sharedd.lib" }

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      links { "shaderc_shared.lib" }

-- Group for Libraries
group "libs"

-- Project: GLFW
project "GLFW"
   kind "StaticLib"
   language "C"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files {
      "dependencies/glfw/include/GLFW/**.h",
      "dependencies/glfw/src/**.c"
   }

   includedirs {
      "dependencies/glfw/include"
   }

   defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- Project: spdLog
project "spdLog"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files {
      "dependencies/spdlog/include/spdlog/**.h",
      "dependencies/spdlog/src/**.cpp"
   }

   includedirs {
      "dependencies/spdlog/include"
   }

   defines { "SPDLOG_COMPILED_LIB" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
