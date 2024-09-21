#pragma once
#include <memory>
#include "GraphicsInterface.hpp"

namespace interface {
	class CoreInterface
	{
	private:
		std::unique_ptr<interface::GraphicsInterface> graphicsInterface{ std::make_unique<interface::GraphicsInterface>() };
	public:
		explicit CoreInterface() = default;
		~CoreInterface() = default;

		void init();
		void run() const;
		void cleanUp();
	};
}

