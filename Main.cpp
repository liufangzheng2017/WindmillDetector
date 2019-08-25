#include "Main.h"

int main(void) {
	std::cout << "[Windmill Detector] Version: 20190822.1655" << std::endl;
	std::cout << "[Usage] Press SPACE to display next frame. Automatically quit after the last frame." << std::endl;
	WindmillDetector* wmdMain = new WindmillDetector;
	while (true) {
		if (wmdMain->Detect())
			return 1;
	}
	return 0;
}