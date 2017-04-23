#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <OpenNI.h>
#include <cstdlib>
#include <iostream>
using namespace openni;
using namespace cv;

class DepthRead
{
public:
	DepthRead(VideoFrameRef klatka,	VideoStream	*stream,Device *dev);
	DepthRead::DepthRead();
	~DepthRead();
	int getdepth(int X, int Y);
private:
	VideoFrameRef klatka;
	VideoStream	*stream;
	Device *dev;
};

