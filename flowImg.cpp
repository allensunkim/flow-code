#define TAG_FLOAT 202021.25  // .flo magic number

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp> // imshow
#include <opencv2/imgproc.hpp> // cvtcolor

#include "flowImg.h"

using namespace std;

void FlowImage::openFlow(const char* file_path)
{
	FILE* stream = fopen(file_path, "rb");
	if (stream == 0)
	{
		cout << "could not open file\n";
		exit(EXIT_FAILURE);
	}

	int width, height;
	float tag;

	// 4byte tag, 4byte width, 4byte height all little endian
	if ((int)fread(&tag, sizeof(float), 1, stream) != 1 ||
		(int)fread(&width, sizeof(int), 1, stream) != 1 ||
		(int)fread(&height, sizeof(int), 1, stream) != 1)
	{
		cout << "file read error\n";
		exit(EXIT_FAILURE);
	}

	if (tag != TAG_FLOAT) // check endianness
	{
		cout << "wrong magic number, check endianness\n";
		exit(EXIT_FAILURE);
	}

	// dimension san checks
	if (width < 1 || width > 99999)
	{
		cout << "illegal width\n";
		exit(EXIT_FAILURE);
	}

	if (height < 1 || height > 99999)
	{
		cout << "illegal height\n";
		exit(EXIT_FAILURE);
	}

	int bands = 2; // dual band u[r0,c0],v[r0,c0],u[r0c1]...
	flowData = cv::Mat(height, width, CV_32FC2);

	int n = bands * width * height;
	if ((int)fread(flowData.data, sizeof(float), n, stream) != n)
	{
		cout << "file is too short\n";
		exit(EXIT_FAILURE);
	}

	fclose(stream);
}


void FlowImage::color(float fx, float fy, unsigned char* pix)
{
	// LAB colorspace for simplicity
	// align values of dist, fx and fy to L[0,100], A[-110,110], B[-110,110] channels respectively
	//pix[0] = (int)(pythag(fx, fy) / 1.41421 * 100); // dull
	pix[0] = (int)log((pythag(fx, fy) / 1.41421 * 10))*100; // moar color
	pix[1] = (int)(fx * 110.);
	pix[2] = (int)(fy * 110.);
}

float FlowImage::findMaxDist()
{
	int width = flowData.cols;
	int height = flowData.rows;

	float maxx = -1e10, maxy = -1e10;
	float minx = 1e10, miny = 1e10;
	float dist = -1;
	for (int y = 0; y < height; y++)
	{
		int row_offset = y * width;
		float* flow_row_ptr = ((float*)flowData.data) + 2 * row_offset;
		for (int x = 0; x < width; x++)
		{
			float fx = flow_row_ptr[2 * x + 0];
			float fy = flow_row_ptr[2 * x + 1];
			if (invalid(fx, fy)) { continue; }
			maxx = max(maxx, fx);
			maxy = max(maxy, fy);
			minx = min(minx, fx);
			miny = min(miny, fy);
			float pythag = sqrt(fx * fx + fy * fy);
			dist = max(dist, pythag);
		}
	}
	if (dist == 0) cout << "flow distances are zero\n";
	return dist;
}

void FlowImage::colorFlow(cv::Mat& color_flow_image, float maxDist)
{
	if (maxDist <= 0) { cout << "max flow dist must be larger than 0\n"; exit(EXIT_FAILURE); }
	int width = flowData.cols;
	int height = flowData.rows;
	if (width <= 0 || height <= 0) { cout << "flow data is invalid\n"; exit(EXIT_FAILURE); }

	if (color_flow_image.rows != flowData.rows || color_flow_image.cols != flowData.cols)
	{
		// convert to lab colorspace
		color_flow_image = cv::Mat(height, width, CV_8UC3);
		cv::cvtColor(color_flow_image, color_flow_image, cv::COLOR_BGR2Lab);
	}

	for (int y = 0; y < height; y++)
	{
		int row_offset = y * width;
		float* flow_row_ptr = ((float*)flowData.data) + 2 * row_offset; // dual band
		uchar* img_row_ptr = color_flow_image.data + 3 * row_offset; // 3 channel
		for (int x = 0; x < width; x++)
		{
			float fx = flow_row_ptr[2 * x + 0];
			float fy = flow_row_ptr[2 * x + 1];
			uchar* pix = img_row_ptr + 3 * x;
			if (invalid(fx, fy)) { pix[0] = pix[1] = pix[2] = 0; }
			else
			{
				color(fx / maxDist, fy / maxDist, pix);
			}
		}
	}
	cv::cvtColor(color_flow_image, color_flow_image, cv::COLOR_Lab2RGB); // convert back from LAB
}


