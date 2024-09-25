workspace "VulkanApp"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "VKEngine"  -- Specify where to place generated files
   startproject "Editor"  -- Set the default startup project

-- Check if the Vulkan SDK environment variable is set
local vulkanLibPath = os.getenv("VULKAN_SDK")
if not vulkanLibPath then
   error("VULKAN_SDK environment variable is not set.")
end

-- Group for Engine Projects
group "Engine"

-- Project 1: Editor
project "Editor"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   location "VKEngine/editor"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files { "VKEngine/editor/**.hpp", "VKEngine/editor/**.cpp" }
   
   includedirs {
      "VKEngine/core/interface",          -- Core headers
	  "dependencies/imgui",  
	  "dependencies/ImGuizmo",   
	  "dependencies/imgui-node-editor",
	  "VKEngine/utilities" 
   }

   links {
      "Core"                           -- Link Core project
   }

   defines { "_CRT_SECURE_NO_WARNINGS" }

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
      "VKEngine/graphics/controllers",   -- Graphics headers
      "VKEngine/utilities",             -- Utilities headers (if used in Core)
	  "dependencies/imgui",
	  "dependencies/imgui/backends",
	  vulkanLibPath.."/Include"	  
   }

   links { "Graphics" }  -- Link against Graphics and Utilities
   defines { "_CRT_SECURE_NO_WARNINGS" }

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
      "dependencies/glfw/include",
      "dependencies/spdlog/include",
	  "dependencies/imgui",
	  "dependencies/imgui/backends",
      "dependencies/glm",
      "VKEngine/utilities",           -- Utilities headers
      vulkanLibPath.."/Include"
   }

   defines { "_CRT_SECURE_NO_WARNINGS" }

   libdirs {
      vulkanLibPath.."/Lib"
   }

   links {
      "GLFW",
      "Utilities",                    -- Link against Utilities project
	  "imgui"
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      links { "shaderc_sharedd.lib" }

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      links { "shaderc_shared.lib" }

-- Project 4: Runtime (Moved after Core and Graphics)
project "Runtime"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   location "VKEngine/runtime"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files { "VKEngine/runtime/**.hpp", "VKEngine/runtime/**.cpp" }

   includedirs {
      "VKEngine/core/interface",
      "VKEngine/graphics/interface"
   }

   links { "Core"}  -- Link against Core and Graphics

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- Project 5: Utilities (Moved before Graphics)
project "Utilities"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   location "VKEngine/utilities"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files { "VKEngine/utilities/**.hpp", "VKEngine/utilities/**.cpp" }

   includedirs {
      "dependencies/spdlog/include",
      "dependencies/glm"
   }

   links { "spdLog" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

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

-- Project: spdLog (Moved under libs group)
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


-- Project: imgui (Moved under libs group)
project "imgui"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   -- Only include core ImGui files and Vulkan backend
   files {
      "dependencies/imgui/*.h",
      "dependencies/imgui/*.cpp",
      "dependencies/imgui/backends/imgui_impl_vulkan.*",  -- Only Vulkan part
      "dependencies/imgui/backends/imgui_impl_glfw.*",  -- Only Vulkan part
      "dependencies/ImGuizmo/*.h",
      "dependencies/ImGuizmo/*.cpp",
      "dependencies/imgui-node-editor/*.h",
      "dependencies/imgui-node-editor/*.cpp"
   }

   -- Exclude folders: misc and examples
   removefiles {
      "dependencies/imgui/misc/**",
      "dependencies/imgui/examples/**",
      "dependencies/ImGuizmo/examples/**",
      "dependencies/ImGuizmo/vcpkg-example/**",
      "dependencies/imgui-node-editor/external/**",
      "dependencies/imgui-node-editor/misc/**",
      "dependencies/imgui-node-editor/examples/**",
   }

   includedirs {
      "dependencies/imgui",                       -- Core ImGui headers
      "dependencies/imgui/backends",              -- Vulkan backend headers
      "dependencies/ImGuizmo",              
      "dependencies/imgui-node-editor", 
	  "dependencies/glfw/include",	  
      vulkanLibPath.."/Include"                   -- Vulkan SDK headers
   }
   
   libdirs {
      vulkanLibPath.."/Lib"
   }
   
   links {
      "vulkan-1.lib"
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

-- Project: JoltPhysics (Moved under libs group)	  
project "jolt"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}"

   files {
      "dependencies/JoltPhysics/Jolt/*.h",
      "dependencies/JoltPhysics/Jolt/*.cpp"
   }

   includedirs {
      "dependencies/JoltPhysics"            
   }
   

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"



-- Project: assimp and softal need to build with cmake...