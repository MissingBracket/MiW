/*EyeC
Simple, not-so-optimal vision system
Current blob target : red
Current Platform : Windows
Current Hardware : Asus Xtion Pro
:>_MissingBracket
*/
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
//+----------GLOBAL VARS--------------
Mat matryca, elementER, elementDY,
	glebia;									//Mat
int GR = 30, GG = 30, GB = 30, Max_Size;	//Red Green Blue
int MR = 255, MG = 255, MB = 255;			//Max color Value
RGB888Pixel* obraz;							//Pixele
DepthPixel *dpixel;
const int MaxVal = 255;
int erozja = 0, dylacja = 0;
FILE*dump;
DepthRead globalread;
bool ison = true; 
bool verbose = false, 
cont = false,calibrated = false;
//+----------------------------------

class MemPoint {
public:
	MemPoint(float x, float y, float depth) { this->x = x; this->y = y; this->depth = depth; }
	float x, y, depth;
};
vector<MemPoint*>memory;
void CalMem();
void updatetrackbars() {
	setTrackbarPos("Red", "Project Basilisk", GR);
	setTrackbarPos("Green", "Project Basilisk", GG);
	setTrackbarPos("Blue", "Project Basilisk", GB);
	setTrackbarPos("MaxR", "Project Basilisk", MR);
	setTrackbarPos("MaxG", "Project Basilisk", MG);
	setTrackbarPos("MaxB", "Project Basilisk", MB);
}
void reset_values() {
	GR = GG = GB = 10;
	MR = MG = MB = 255;
	setTrackbarPos("Red", "Project Basilisk", GR);
	setTrackbarPos("Green", "Project Basilisk", GG);
	setTrackbarPos("Blue", "Project Basilisk", GB);
	setTrackbarPos("MaxR", "Project Basilisk", MR);
	setTrackbarPos("MaxG", "Project Basilisk", MG);
	setTrackbarPos("MaxB", "Project Basilisk", MB);
	
}
void sprzataj(Device *D, VideoStream *VS ,VideoStream *DVS, VideoFrameRef VFR, VideoFrameRef DVFR) {
	VS->stop();
	DVS->stop();
	VS->destroy();
	DVS->destroy();
	VFR.release();
	DVFR.release();
	D->close();
}
void pobierzobraz() {
	//DEBUG PURPOSE FUNCTION - WHEN ALL ELSE FAILS
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
	memmove(matryca.data, obraz, 3 * klatka.getHeight()*klatka.getWidth()*sizeof(uint8_t));
	cvtColor(matryca, matryca, CV_BGR2RGB);
	Max_Size = klatka.getWidth()*klatka.getHeight();
	stream.stop();
	stream.destroy();
	dev.close();
	//OpenNI::shutdown();
}
void editdGreen(int g) {
	cout << (int)obraz[g].g << endl;
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
	setTrackbarPos("Red", "Project Basilisk", GR);
}
void editBlue(int, void*) {
	GB = min(MB - 1, GB);
	setTrackbarPos("Blue", "Project Basilisk", GB);
}
void editMaxB(int, void*) {
	MB = max(MB, GB + 1);
	setTrackbarPos("MaxB", "Project Basilisk", MB);
}
void editMaxG(int, void*) {
	MG = max(MG, GG + 1);
	setTrackbarPos("MaxG", "Project Basilisk", MG);
}
void editMaxR(int, void*) {
	MR = max(MR, GR + 1);
	setTrackbarPos("MaxR", "Project Basilisk", MR);
}
void erode(int, void*) {
	elementER = getStructuringElement(MORPH_CROSS,
		Size(2 * erozja + 1, 2 * erozja + 1),
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
void controls() {
	cout << endl << endl;
	printf("The device is "); (calibrated == true ? printf("Calibrated\n") : printf("Not Calibrated\n"));
	switch (verbose) {
	case 0:
		cout << endl << endl << "\tSterowanie:\n\tq - wyjscie\n\ts - zapisz obraz\n"
			"\tr - przelacz progowanie\n\tc - przelacz konturowanie\n"
			"\tm - czytanie odleglosci\n\tk - przywroc wartosci\n\th - wypisz pomoc\n\tv - ustaw dokladnosc pomocy\n";
		break;
	case 1:
		printf("\tKlawisz\tStan\tFunkcja\n");
		printf("\tq\tn/a\tWyjscie\n");
		printf("\ts\tn/a\tZapisz do pliku\n");
		printf("\tr\t%d\tPrzelacz Progowanie\n", ison);
		printf("\tc\t%d\tPrzelacz Wykrywanie\n", cont);
		printf("\tk\tn/a\tReset wartosci - DEBUG\n");
		printf("\th\tn/a\tPokaz Pomoc\n");
		break;
	}
}
void accomodate(vector<RotatedRect>InSight);
void kontur() {
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
	//cout << "memory has " << minellipse.size() << endl;
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

void odleglosc(int event, int x, int y, int flags, void*userdata) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		printf("Pixel w pozycji:\n\tX =\t%d\n\tY =\t%d\n", x, y);
		printf("\tDepth at point: %d\n", dpixel[x * 320 + y]);
		break;
	}
}
vector<MemPoint*>Calibrationpoints;
void calibrate(int event, int x, int y, int flags, void*userdata) {
	switch (event) {
	case EVENT_LBUTTONDOWN:
		int recX = x;
		int recY = y;
		int recD = dpixel[x * 320 + y];
		printf("Data : %d\t%d\t%d\n", recX, recY, recD);
		if (Calibrationpoints.size() < 3) {
			Calibrationpoints.push_back(new MemPoint(recX, recY, recD));
		}
		
		printf("Pixel w pozycji:\n\tX =\t%d\n\tY =\t%d\n", recX, recY);
		printf("\tDepth at point: %d\n", recD);
		
		break;
	}
	
}
void ConvertCalibrationMemory(VideoStream *vs) {
	float Out_X, Out_Y, Out_Z;
	//CoordinateConverter::convertDepthToWorld(vs, Calibrationpoints[0]->x,Calibrationpoints[0]->y ,Calibrationpoints[0]->depth , &Out_X, &Out_Y, &Out_Z);	
	auto status = CoordinateConverter::convertDepthToWorld(*vs, Calibrationpoints[0]->x,
		Calibrationpoints[0]->y,
		Calibrationpoints[0]->depth,
		&Out_X, &Out_Y, &Out_Z);
	cout << ":>_ Converting\n";
	int inc = 0;
	for (auto it = Calibrationpoints.begin(); it != Calibrationpoints.end(); ++it) {
		cout << ":>_ Converting " << (*it)->x << '\t' << (*it)->y << '\t' << (*it)->depth << '\n';
		auto status = CoordinateConverter::convertDepthToWorld(*vs, (*it)->x,
			(*it)->y,
			(*it)->depth,
			&Out_X, &Out_Y, &Out_Z);
		Calibrationpoints[inc]->x		= Out_X;
		Calibrationpoints[inc]->y		= Out_Y;
		Calibrationpoints[inc]->depth	= Out_Z;
		inc++;
	}
	cout << ":>_ Converted " << Calibrationpoints.size() << " entries\n";
	CalMem();
}
bool IsBlind(vector<RotatedRect>Objects) {
	return Objects.empty();
}
void accomodate(vector<RotatedRect>InSight) {
	if (IsBlind(InSight)) {
		GR--;
		MR++;

		//cv::setTrackbarPos("Red", "Project Basilisk", GR);
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
	updatetrackbars();
}
Mat get_colour_image(VideoStream &color, VideoFrameRef colorFrame) {
	Mat frame;color.readFrame(&colorFrame);
	const openni::RGB888Pixel* imageBuffer = (const openni::RGB888Pixel*)colorFrame.getData();
	frame.create(colorFrame.getHeight(), colorFrame.getWidth(), CV_8UC3);
	memcpy(frame.data, imageBuffer, 3 * colorFrame.getHeight()*colorFrame.getWidth()*sizeof(uint8_t));
	cv::cvtColor(frame, frame, CV_BGR2RGB); //this will put colors right
	return frame;
}
Mat get_depth_image(VideoStream *vs, VideoFrameRef frame) {
	Mat depth;
	VideoStream* pstream = &*vs; int dummy;
	openni::OpenNI::waitForAnyStream(&pstream, 1, &dummy, openni::STATUS_TIME_OUT);
	vs->readFrame(&frame);
	dpixel = (openni::DepthPixel*)frame.getData();//uint16
	depth = Mat(CvSize(frame.getWidth(), frame.getHeight()), CV_16UC1, dpixel);
	int max = 0;
	for (int i = 0; i < frame.getWidth() * frame.getHeight(); i++) {
		if (dpixel[i] > max) max = dpixel[i];
	}
	depth.convertTo(depth, CV_32F);
	float divisor = max / 255.0;
	for (int i = 0; i < depth.rows; i++) {
		for (int j = 0; j < depth.cols; j++) {
			depth.at<float>(i, j) /= divisor;
		}
	}
	double min;
	double emax;
	minMaxIdx(depth, &min, &emax);
	convertScaleAbs(depth, depth, 255 / emax);
	return depth;
}
void CalMem() {
	printf("\n\tAccessing Calibration Memory [...]\n");
	printf("\n:>_ok\tCalibration Memory is %d in size\n", Calibrationpoints.size());
	printf("Calibration points as follow:\n");
	printf("Point \tX\tY\tD\n");

	for (auto it = Calibrationpoints.begin(); it != Calibrationpoints.end(); ++it) {
		cout<<"Point: "<<(*it)->x<<'\t'<<(*it)->y<<'\t' << (*it)->depth << endl;
	}
	printf("\n:>_ok\tCalibration Memory End\n");
}

void EyeC_Calibration(VideoStream *vs, VideoFrameRef frame) {
	printf(":>_ Device has to be calibrated before use.\n"
		":>_ Point three markers in prompted screen.\n");
	namedWindow("EyeC_Calibration", CV_WINDOW_NORMAL);
	setMouseCallback("EyeC_Calibration", calibrate, NULL);
	Mat Cal = get_depth_image(vs, frame);
	
	while (!calibrated) {
		char k = waitKey(1);
		if(!Cal.empty()) imshow("EyeC_Calibration", Cal);
		if (k == 'q') { exit(1); }//NEED CLEANING
		Cal = get_depth_image(vs, frame);
		if (Calibrationpoints.size() >= 3) {
			CalMem();
			printf(":>_ Are those points ok?\n"
			":>_ yY/nN\n");
			char agreement;
			scanf("%c", &agreement);
			switch (agreement) {
			case 'y':
			case 'Y':
				ConvertCalibrationMemory(vs);
				calibrated = true;
				//Now exits calibration.
				break;
			case 'n':
			case 'N':
				for (auto it = Calibrationpoints.begin(); it != Calibrationpoints.end(); ++it)
					delete (*it);
				Calibrationpoints.clear();
				EyeC_Calibration(vs,frame);
				break;
			default :
				printf("well, shit");
				break;
			}
		}
	}
}


void main() {
	
	auto rc = openni::OpenNI::initialize();			if (rc == STATUS_ERROR) { cout << "OpenNI Failed to Initialize!\n", exit(-1); }
	Device dev;
	VideoStream stream, d_stream;
	VideoFrameRef klatka, d_klatka;
	if ((rc = dev.open(openni::ANY_DEVICE)) == STATUS_ERROR) { cout << "Failed to open device!\n", exit(-1); }
	if ((rc = stream.create(dev, openni::SENSOR_COLOR)) == STATUS_ERROR) { cout << "Failed to create video stream!\n", exit(-1); }
	if ((rc = stream.start()) == STATUS_ERROR) { cout << "Failed to start new stream!\n", exit(-1); }
	//***DEPTH SECTION
	if ((rc = d_stream.create(dev, openni::SENSOR_DEPTH)) == STATUS_ERROR) { cout << "Failed to create Depth video stream!\n", exit(-1); }
	if ((rc = d_stream.start()) == STATUS_ERROR) { cout << "Failed to start new Depth stream!\n", exit(-1); }
	EyeC_Calibration(&d_stream, d_klatka);
	
	matryca = get_colour_image(stream, klatka);
	glebia = get_depth_image(&d_stream, d_klatka);
	namedWindow("Project Basilisk", CV_WINDOW_AUTOSIZE);
	namedWindow("Project Basilisk : Depth", CV_WINDOW_AUTOSIZE);
	imshow("Project Basilisk", matryca);
	imshow("Project Basilisk : Depth", glebia);
	inittrackbars();
	controls();
	char d = 'n';
	setMouseCallback("Project Basilisk", odleglosc, NULL);
//	setMouseCallback("Project Basilisk : Depth", calibrate, NULL);
	while (d != 'q') {
		d = (char)waitKey(1);
		if (ison) inRange(matryca, Scalar(GB, GG, GR), Scalar(MB, MG, MR), matryca);
		erode(matryca, matryca, elementER);
		dilate(matryca, matryca, elementDY);
		if (cont)kontur();
		imshow("Project Basilisk : Depth", glebia);
		imshow("Project Basilisk", matryca);
		if (d == 's') { imwrite("pic.jpg", matryca); /*fprintf_s(dump, "\nImage Write to pic.jpg\n");*/ }
		if (d == 'r')controlrange(ison);
		if (d == 'v')controlrange(verbose);
		if (d == 'c')if (ison)controlrange(cont);
		if (d == 'h') { controls(); }
		if (d == 'm') { CalMem(); }
		if (d == 'k') { reset_values(); }
		if (d == 'o') { ConvertCalibrationMemory(&d_stream); }
		//if (d == 'p') { for (auto it = memory.begin(); it != memory.end(); it++) { cout << (*it)->x << "\t" << (*it)->y << endl; } }
		glebia = get_depth_image(&d_stream, d_klatka);
		matryca = get_colour_image(stream, klatka);
	}
	sprzataj(&dev, &stream, &d_stream, klatka, d_klatka);
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
	matryca = get_colour_image(stream,klatka);
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
		matryca = get_colour_image(stream,klatka);
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