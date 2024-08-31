workspace "VulkanApp"
   configurations { "Debug", "Release" }
   location "VKEngine"  -- Specify where to place generated files
   startproject "Editor"  -- Set the default startup project

   -- Project 1: Editor
   project "Editor"
      kind "ConsoleApp"
      language "C++"
	  cppdialect "C++20"
      targetdir "bin/%{cfg.buildcfg}"

      files { "VKEngine/editor/**.hpp","VKEngine/editor/**.cpp"}
	  
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

      files { "VKEngine/core/**.hpp" ,"VKEngine/core/**.cpp"}
	  
	  links { "Graphic" }

      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"

      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"
		 
		 
	project "Graphics"
      kind "StaticLib"  
      language "C++"
	  cppdialect "C++20"
      targetdir "bin/%{cfg.buildcfg}"

      files { "VKEngine/graphics/**.hpp", "VKEngine/graphics/**.cpp" }
	  
	  includedirs {
      "dependencies",
      "C:/VulkanSDK/1.3.290.0/Include"
	  }

	 libdirs {
	  "C:/VulkanSDK/1.3.290.0/Lib"
     }

     links {
      "GLFW",
      "vulkan-1.lib"
     }
	 

      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"
		 
		 links {"shaderc_sharedd.lib"}

      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"
		 
		 links {"shaderc_shared.lib"}

	project "GLFW"
      kind "StaticLib"  
      language "C"
	  location "build/dependencies"
      targetdir "bin/%{cfg.buildcfg}"

      files { "dependencies/glfw/include/GLFW/**.h",
	  "dependencies/glfw/src/**.c" }
	  
	  defines {"_GLFW_WIN32"}

      filter "configurations:Debug"
         defines { "DEBUG" }
         symbols "On"

      filter "configurations:Release"
         defines { "NDEBUG" }
         optimize "On"