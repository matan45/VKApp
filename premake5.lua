workspace "VulkanApp"
   configurations { "Debug", "Release" }
   location "build"  -- Specify where to place generated files

   -- Project 1: Editor
   project "Editor"
      kind "ConsoleApp"
      language "C++"
	  cppdialect "C++20"
      targetdir "bin/%{cfg.buildcfg}"

      files { "Editor/**.h", "Editor/**.cpp" }
	  
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
      targetdir "bin/%{cfg.buildcfg}"

      files { "Core/**.h", "Core/**.cpp" }
	  
	  links { "Graphic" }

      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"

      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"
		 
		 
	project "Graphic"
      kind "StaticLib"  
      language "C++"
	  cppdialect "C++20"
      targetdir "bin/%{cfg.buildcfg}"

      files { "Graphic/**.h", "Graphic/**.cpp" }
	  
	  includedirs {
      "dependencies",
      "C:/VulkanSDK/1.3.290.0/Include"
	  }

	 libdirs {
	  "C:/VulkanSDK/1.3.290.0/Lib"
     }

     links {
      "GLFW",
      "shaderc_sharedd.lib",
      "vulkan-1.lib"
     }
	 

      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"

      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"

	project "GLFW"
      kind "StaticLib"  
      language "C"
	  location "build/dependencies"
      targetdir "bin/%{cfg.buildcfg}"

      files { "dependencies/glfw/include/GLFW/**.h",
	  "dependencies/glfw/src/**.c" }

      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"

      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"