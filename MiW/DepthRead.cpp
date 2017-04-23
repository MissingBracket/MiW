#include "stdafx.h"
#include "DepthRead.h"


DepthRead::DepthRead(VideoFrameRef klatka, VideoStream	*stream, Device *dev)
{
	this->dev = dev;
	this->klatka = klatka;
	this->stream = stream;
}
DepthRead::DepthRead(){}

DepthRead::~DepthRead()
{

}
int DepthRead::getdepth(int X, int Y) {
	stream->stop();
	stream->create(*dev, SENSOR_DEPTH);
	stream->start();
	//stream->readFrame(&klatka);

	VideoStream* pstream = &*stream; int dummy;
	openni::OpenNI::waitForAnyStream(&pstream, 1, &dummy, openni::STATUS_TIME_OUT);
	stream->readFrame(&klatka);

	openni::DepthPixel* pixel = (openni::DepthPixel*)klatka.getData();//uint16

	stream->stop(); stream->create(*dev, SENSOR_COLOR);
	stream->start();
	return pixel[X*klatka.getHeight() + Y];

}