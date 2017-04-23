// MiW.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DepthRead.h"
using namespace std;
using namespace cv;
using namespace openni;
//#include <opencv.hpp>
/*	DEFAULT:
GR,GG,GB na 30
MR,MG,MB na 255
*/
Mat matryca, elementER, elementDY;			//Mat
int GR = 30, GG = 30, GB = 30, Max_Size;	//Red Green Blue
int MR = 255, MG = 255, MB = 255;			//Max color Value
RGB888Pixel* obraz;							//Pixele
const int MaxVal = 255;
int erozja = 0, dylacja = 0;
FILE*dump;
DepthRead globalread;

class MemPoint {
public:
	MemPoint(float x, float y, float depth) { this->x = x; this->y = y; this->depth = depth; }
	float x, y, depth;
};
vector<MemPoint*>memory;

void pobierz(VideoStream *VS, VideoFrameRef VFR) {
	VS->readFrame(&VFR);
	obraz = (RGB888Pixel*)VFR.getData();
	matryca.create(VFR.getHeight(), VFR.getWidth(), CV_8UC3);
	memmove(matryca.data, obraz, 3 * VFR.getHeight()*VFR.getWidth()*sizeof(uint8_t));
	cvtColor(matryca, matryca, CV_BGR2RGB); 
	Max_Size = VFR.getWidth()*VFR.getHeight();
}
void obrazglebia(Device *d, VideoStream *vs, VideoFrameRef frame) {
	

	VideoStream* pstream = &*vs; int dummy;
	openni::OpenNI::waitForAnyStream(&pstream, 1, &dummy, openni::STATUS_TIME_OUT);
	vs->readFrame(&frame);
	

	openni::DepthPixel* pixel = (openni::DepthPixel*)frame.getData();//uint16
	cout << "Frame sizes \nWidth = "; cout << frame.getWidth() << endl << "Height = " << frame.getHeight() << endl;
	for (int i = 0; i < 450; i++) { cout << pixel[i] << "\t"; }

	matryca = Mat(CvSize(frame.getWidth(), frame.getHeight()), CV_16UC1, pixel);
	int max = 0;
	for (int i = 0; i < frame.getWidth() * frame.getHeight(); i++) {
		if (pixel[i] > max) max = pixel[i];
	}
	cout << endl << "max = " << max << endl;
	matryca.convertTo(matryca, CV_32F);
	float divisor = max / 255.0;
	for (int i = 0; i < matryca.rows; i++) {
		for (int j = 0; j < matryca.cols; j++) {
			matryca.at<float>(i, j) /= divisor;
		}
	}

}

void sprzataj(Device *D, VideoStream *VS, VideoFrameRef VFR) {
	VS->stop();
	VS->destroy();
	//klatka.release();
	VFR.release();
	D->close();
}
void pobierzobraz() {
	//OpenNI::initialize();
	Device dev;
	VideoStream stream;
	VideoFrameRef klatka;
	dev.open(ANY_DEVICE);
	stream.create(dev, SENSOR_COLOR);
	stream.start();
	stream.readFrame(&klatka);	
	obraz = (RGB888Pixel*)klatka.getData();	
	matryca.create(klatka.getHeight(), klatka.getWidth(), CV_8UC3);
	//	memcpy(matryca.data, obraz, 3 * klatka.getHeight()*klatka.getWidth()*sizeof(uint8_t));
	
	memmove(matryca.data, obraz, 3 * klatka.getHeight()*klatka.getWidth()*sizeof(uint8_t));
	 
	cvtColor(matryca, matryca, CV_BGR2RGB);
	Max_Size = klatka.getWidth()*klatka.getHeight();
	stream.stop();	
	stream.destroy();
	dev.close();


	//OpenNI::shutdown();
}



void editGreen(int g) {
	cout <<(int) obraz[g].g<<endl;
	for (int i = 0; i < Max_Size; i++) {
		obraz[i].g -= g;
	}
	memcpy(matryca.data, obraz, 3 * Max_Size*sizeof(uint8_t));
	cvtColor(matryca, matryca, CV_BGR2RGB);
}
void editGreen(int, void*) {
	GG = min(MG - 1, GG);
	setTrackbarPos("Green", "Project Basilisk", GG);
}
void editRed(int, void*) {
	GR = min(MR - 1, GR);

}
void editBlue(int, void*) {
	GB = min(MB - 1, GB);
}
void editMaxB(int, void*){
	MB = max(MB, GB + 1);
}
void editMaxG(int, void*) {
	MG = max(MG, GG + 1);
}
void editMaxR(int, void*) {
	MR = max(MR, GR + 1);
}
void erode(int, void*) {
	elementER = getStructuringElement(MORPH_CROSS,
		Size(2 * erozja + 1, 2 * erozja+ 1),
		Point(erozja, erozja));

}
void dilate(int, void*) {
	elementDY = getStructuringElement(MORPH_ELLIPSE,
		Size(2 * dylacja + 1, 2 * dylacja + 1),
		Point(dylacja, dylacja));
}
void inittrackbars() {
	createTrackbar("Red", "Project Basilisk", &GR, MR, editRed);// editRed(39, 0);
	createTrackbar("Green", "Project Basilisk", &GG, MG, editGreen); //editGreen(0, 0);
	createTrackbar("Blue", "Project Basilisk", &GB, MB, editBlue); //editBlue(0, 0);
	createTrackbar("MaxR", "Project Basilisk", &MR, 255, editMaxR); //editMaxR(147, 0);
	createTrackbar("MaxG", "Project Basilisk", &MG, 255, editMaxG);// editMaxG(11, 0);
	createTrackbar("MaxB", "Project Basilisk", &MB, 255, editMaxB); //editMaxB(47, 0);
	createTrackbar("Erode? ", "Project Basilisk", &erozja, 20, erode);
	createTrackbar("Dilate? ", "Project Basilisk", &dylacja, 20, dilate);
}
void controlrange(bool &ison) {
	if (ison) {
		ison = false; return;
	}
	else if (!ison) { ison = true; return; }
}

void controls(bool isn,bool cnt,int msr, bool vrb) {
	switch (vrb) {case 0:
		cout << endl << endl << "\tSterowanie:\n\tq - wyjscie\n\ts - zapisz obraz\n"
			"\tr - przelacz progowanie\n\tc - przelacz konturowanie\n"
			"\tm - czytanie odleglosci\n"; 
		break;
	case 1:
		printf("\tKlawisz\tStan\tFunkcja\n");
		printf("\tq\tn/a\tWyjscie\n");
		printf("\ts\tn/a\tZapisz do pliku\n");
		printf("\tr\t%d\tPrzelacz Progowanie\n", isn);
		printf("\tc\t%d\tPrzelacz Wykrywanie\n", cnt);
		printf("\tm\t%d\tPrzelacz czytanie odleglosci(LMB)\n", msr);
		printf("\th\tn/a\tPokaz Pomoc\n");
		break;
	}
	
		//s r c h m p

}
void accomodate(vector<RotatedRect>InSight);
void kontur(){
	RNG rng(12345);
	vector<vector<Point>>kontury;
	vector<Vec4i>hierarchy;
	findContours(matryca, kontury, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
//	vector<RotatedRect>minrect(kontury.size());
	vector<RotatedRect>minellipse(kontury.size());
	for (int i = 0; i < kontury.size(); i++) {
//		minrect[i] = minAreaRect(Mat(kontury[i]));
		if (kontury[i].size() > 5) {
			minellipse[i] = fitEllipse(Mat(kontury[i]));

		}
	}
	
	matryca = Mat::zeros(matryca.size(), CV_8UC3);
	for (int i = 0; i < kontury.size(); i++) {
		Scalar kolor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	drawContours(matryca, kontury, i, kolor, 1, 8, vector<Vec4i>(), 0, Point());
		ellipse(matryca, minellipse[i], kolor, 2, 8);
	/*	Point2f rect_points[4]; minrect[i].points(rect_points);
		for (int j = 0; j < 4; j++) {
			line(matryca, rect_points[j], rect_points[(j + 1) % 4], kolor, 1, 8);
		}*/
	}
	cout << "memory has " << minellipse.size() << endl;
	if (!minellipse.empty()) {
		//fprintf_s(dump, "Detected Ellipses:\t%d\n", minellipse.size());
		if (memory.empty()) { memory.push_back(new MemPoint(minellipse[0].center.x, minellipse[0].center.y, 0)); return; }
		for (auto it = minellipse.begin(); it != minellipse.end(); it++) {
			//cout << "Center of ellipse:\t" << (*it).center.x << "\n\t\t" << (*it).center.y << endl << endl;
			//fprintf_s(dump, "Ellipse center:\nX:\t%f\nY:\t%f\n", (*it).center.x, (*it).center.x);
		}
	}
	
	accomodate(minellipse);
}
bool mouse = false;

void odleglosc(int event, int x, int y, int flags, void*userdata) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		printf("Pixel w pozycji:\n\tX =\t%d\n\tY =\t%d\n", x, y);
		//fprintf_s(dump, "Depth read:\nX:\t%d\nY:\t%d\n", x, y);
	if(mouse)	printf("odl:\t%d\n", globalread.getdepth(x,y));
		break;
	}
}
bool IsBlind(vector<RotatedRect>Objects) {
	return Objects.empty();
}

void accomodate(vector<RotatedRect>InSight) {
	if (IsBlind(InSight)) {
		GR--;
		MR++;

		cv::setTrackbarPos("Red", "Project Basilisk", GR);
	}//Sees little to nothing
	else if (InSight.size() < 12) {
		switch (InSight.size()) {
		case 1:

			break;
		case 2:

			break;
		case 3:

			break;
		default://4 and more
			break;
		}
	}//Too Many Objects
	else if (InSight.size() >= 12) {
		if (MG > 0 || MB > 0) {
			MG--; MB--;
			setTrackbarPos("MaxG", "ProjectBasilisk", MG);
			setTrackbarPos("MaxB", "ProjectBasilisk", MB);
		}
		else {
			erozja++;
			setTrackbarPos("Erode? ", "Project Basilisk", erozja);
		}
	}
}
Mat potot(VideoStream &color, VideoFrameRef colorFrame){
/*openni::Device device;
openni::VideoStream  color;
openni::VideoFrameRef colorFrame;

auto rc = openni::OpenNI::initialize();
rc = device.open(openni::ANY_DEVICE);
rc = color.create(device, openni::SENSOR_COLOR);
rc = color.start();*/
Mat frame;

	color.readFrame(&colorFrame);
	const openni::RGB888Pixel* imageBuffer = (const openni::RGB888Pixel*)colorFrame.getData();

	frame.create(colorFrame.getHeight(), colorFrame.getWidth(), CV_8UC3);
	memcpy(frame.data, imageBuffer, 3 * colorFrame.getHeight()*colorFrame.getWidth()*sizeof(uint8_t));

	cv::cvtColor(frame, frame, CV_BGR2RGB); //this will put colors right
	
	return frame;
}

void main() {
	
	OpenNI::initialize();

	Device dev;
	VideoStream stream;
	VideoFrameRef klatka;

	auto rc = openni::OpenNI::initialize();
	rc = dev.open(openni::ANY_DEVICE);
	rc = stream.create(dev, openni::SENSOR_COLOR);
	rc = stream.start();
	bool ison = true; bool verbose = false,cont = false;
	matryca = potot(stream, klatka);
	namedWindow("Project Basilisk", CV_WINDOW_AUTOSIZE);
	imshow("Project Basilisk", matryca);
	inittrackbars();
	controls(ison,cont,mouse,verbose);
	char d = 'n';
	while (d != 'q') {
		d = (char)waitKey(1);
		if (ison) inRange(matryca, Scalar(GB, GG, GR), Scalar(MB, MG, MR), matryca);
		erode(matryca, matryca, elementER);
		dilate(matryca, matryca, elementDY);
		if (cont)kontur();
		imshow("Project Basilisk", matryca);
		setMouseCallback("Project Basilisk", odleglosc, NULL);
		if (d == 's') { imwrite("pic.jpg", matryca); fprintf_s(dump, "\nImage Write to pic.jpg\n"); }
		if (d == 'r')controlrange(ison);
		if (d == 'v')controlrange(verbose);
		if (d == 'c')if (ison)controlrange(cont);
		if (d == 'h') { system("cls"); controls(ison,cont,mouse,verbose); }
		if (d == 'm') { cout << "Czytanie odleglosci: " << !mouse << endl; controlrange(mouse); }
		//if (d == 'p') { for (auto it = memory.begin(); it != memory.end(); it++) { cout << (*it)->x << "\t" << (*it)->y << endl; } }
		matryca = potot(stream, klatka);
	}

	sprzataj(&dev, &stream, klatka);
	//fprintf_s(dump, "\nDump closing");
	//fclose(dump);
	OpenNI::shutdown();
}
//OBSOLETE BELOW

/*REFUSED COOPERATION
void dmain() {
	controls();
//	fopen_s(&dump, "DumpFile.txt", "w"); fprintf_s(dump, "Dump open for analysis\n\n");
	OpenNI::initialize();
	
	Device dev;
	VideoStream stream;
	VideoFrameRef klatka; globalread = DepthRead(klatka, &stream, &dev);
	auto debug  = dev.open(ANY_DEVICE);
	
	debug = stream.create(dev, SENSOR_DEPTH);
	
	debug = stream.start();

	bool ison = true; bool cont = false; 
	//pobierz( &stream, klatka);//pobierzobraz();
	//obrazglebia(&dev, &stream, klatka);
	//matryca = imread("redsmall.JPG");
	matryca = potot(stream,klatka);
	namedWindow("Project Basilisk", CV_WINDOW_AUTOSIZE);
	imshow("Project Basilisk", matryca);
	inittrackbars();
	char d = 'n';
	while (d != 'q') {
		d = (char)waitKey(1);
		if (matryca.empty())break;
		//matryca = imread("redsmall.jpg");//
		//pobierz(&stream, klatka);	//	pobierzobraz();
	//	obrazglebia(&dev, &stream, klatka);
		matryca = potot(stream,klatka);
		if (ison) inRange(matryca, Scalar(GB, GG, GR), Scalar(MB, MG, MR), matryca);
		erode(matryca, matryca, elementER);
		dilate(matryca, matryca, elementDY);
		if(cont)kontur();
		imshow("Project Basilisk", matryca);
		setMouseCallback("Project Basilisk", odleglosc, NULL);
		if (d == 's') { imwrite("pic.jpg", matryca); fprintf_s(dump, "\nImage Write to pic.jpg\n"); }
		if (d == 'r')controlrange(ison);
		if (d == 'c')if(ison)controlrange(cont);
		if (d == 'm') { cout << "Czytanie odleglosci: " << !mouse << endl; controlrange(mouse); }
		if (d == 'h') { controls(); }
		if (d == 'p') { for (auto it = memory.begin(); it != memory.end(); it++) { cout << (*it)->x << "\t" << (*it)->y << endl; } }
	}
	sprzataj(&dev, &stream, klatka);
	//fprintf_s(dump, "\nDump closing");
	//fclose(dump);
	OpenNI::shutdown();
}*/