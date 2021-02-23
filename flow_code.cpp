// visualize flo files quickly
// visual determination of flo file validity
#include <iostream>
#include "flowImg.h"

using namespace std;

int main(int argc, char *argv[])
{
	string filename;
	if (argc > 1) {
		filename = std::string(argv[1]);
	}
	else {
		cout << "Load error\n";
		return 0;
	}

	size_t loc = filename.find_last_of('.');
	string dot = filename.substr(loc);
	if (dot.compare(".flo") != 0)
	{
		cout << "wrong file type, must be .flo\n";
		exit(EXIT_FAILURE);
	}

	FlowImage a;
	a.openFlow(filename.data());
	cv::Mat colorImg;
	a.colorFlow(colorImg, a.findMaxDist());
	cv::imshow("color img", colorImg);
	cv::waitKey(0);
}