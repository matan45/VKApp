#pragma once


namespace window {
	class Window;
}

namespace core {
	class Device;
	class SwapChain;
}

namespace controllers {
	class Graphics
	{
	private:
		inline static window::Window* window = nullptr;   // inline static member
		inline static core::Device* device = nullptr;     // inline static member
		inline static core::SwapChain* swapChain = nullptr; // inline static member
		
	public:

		static void createContext();
		static void destroyContext();

		static window::Window& getWindow();
		static core::Device& getDevice();
		static core::SwapChain& getSwapChain();

	private:
		explicit Graphics() = default;
		~Graphics() = default;

		static void init();
		static void cleanup();
	};
};


