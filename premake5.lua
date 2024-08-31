workspace "VulkanProject"
   architecture "x64"
   configurations { "Debug", "Release" }

-- Main Project
project "VulkanApp"
   location "VulkanApp"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   staticruntime "on"

   targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
   objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

   files {
      "%{prj.name}/src/**.h",
      "%{prj.name}/src/**.cpp"
   }

   includedirs {
      "%{prj.name}/src",
      "C:/VulkanSDK/1.3.290.0/Include"   -- Replace with your Vulkan SDK include path
   }

   libdirs {
      "C:/VulkanSDK/1.3.290.0/Lib"       -- Replace with your Vulkan SDK library path
   }

   links {
      "vulkan-1",                 -- Replace with the Vulkan library name (e.g., vulkan-1.lib)
      "VulkanUtils"               -- Link the VulkanUtils static library to VulkanApp
   }

   filter "system:windows"
      systemversion "latest"

   filter "configurations:Debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "on"

   filter "configurations:Release"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "on"

-- Subproject
project "VulkanUtils"
   location "VulkanUtils"
   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   staticruntime "on"

   targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
   objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

   files {
      "%{prj.name}/src/**.h",
      "%{prj.name}/src/**.cpp"
   }

   includedirs {
      "%{prj.name}/src",
      "C:/VulkanSDK/1.3.290.0/Include"   -- Replace with your Vulkan SDK include path
   }

   filter "system:windows"
      systemversion "latest"

   filter "configurations:Debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "on"

   filter "configurations:Release"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "on"
