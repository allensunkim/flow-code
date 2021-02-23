#pragma once
#ifndef _FLOW_IMAGE_H_
#define _FLOW_IMAGE_H_
#define unknown_val 1e10

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <cmath>

class FlowImage
{
public:
	cv::Mat flowData;

	FlowImage()
	{}

	// return whether flow vector is unknown
	static bool invalid(float fx, float fy)
	{
		return (abs(fx) > unknown_val || abs(fy) > unknown_val || isnan(fx) || isnan(fy));
	}
	void openFlow(const char* file_path);
	float pythag(float fx, float fy) { return sqrt(fx * fx + fy * fy); }
	void color(float fx, float fy, unsigned char* pix);
	float findMaxDist();
	void colorFlow(cv::Mat& color_flow_image, float maxDist);
};

#endif
