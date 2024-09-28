#pragma once
namespace controllers {
	class OffScreenController;
}

namespace interface {

	class OffScreen
	{
	private:
		controllers::OffScreenController* offScreenController;

	public:
		explicit OffScreen();
		~OffScreen();

		void init();
		void cleanUp();

		void* render();
	};
}

