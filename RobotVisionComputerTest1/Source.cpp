/*
Name: Kyle Tompkins
Date Started: 9/7/17
Last Modified: 9/12/17 10:40am
*/
///camera resolution 640x480
///camera focal length 612
#include <iostream>
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include <string>
#include <Windows.h>
#include <math.h>

using namespace std;
using namespace cv;

vector<vector<Point> > contours;
Point CameraCenter = Point(320, 240);
vector<Vec4i> hierarchy;

///public varibles
RNG rng(12345);
Scalar RED(0, 0, 255);//BGR
Scalar GREEN(0, 255, 0);
Scalar BLUE(255, 0, 0);
Scalar PURPLE(255, 0, 255);
Scalar BLACK(0, 0, 0);
int const TARGET_WIDTH = 4;
int const FOCAL_LENGTH = 612;
int LeftRightDistance;
int DistanceToTarget;
bool FoundTarget = false;
bool LeftRightAligned = false;
bool DistanceAligned = false;
string LeftRightSTR;
string FoundTargetSTR;

VideoCapture cap(0); // open the default camera


bool PointCompare(Point2f point1, Point2f point2);//point

int main()
{
	if (!cap.isOpened())  // check if we succeeded
		return -1;
	//for use with leds//cap.set(CAP_PROP_EXPOSURE, -10);//set the camera exposure

	while (waitKey(100))
	{
#pragma region target_locator
		///resets
		FoundTarget = false;
		LeftRightAligned = false;
		DistanceAligned = false;

		

		///normal image
		Mat frame;//new image 
		cap >> frame;//set cap(video camera) to the new image
		//imshow("Original Video", frame);//open window showing image
		//cout << frame.rows << " " << frame.cols << endl;

		///GrayScale Image
		Mat gray;
		cap >> gray;
		cvtColor(gray, gray, CV_BGR2GRAY);
		//imshow("GrayScale Image", gray);

		///Edge Detedction
		Mat blur_gray;
		Mat canny_gray;
		blur(gray, blur_gray, Size(3, 3));
		Canny(blur_gray, canny_gray, 40, 40 * 2, 3);
		//imshow("Edges", canny_gray);

		///Contours
		//findContours(InputOutputArray image, OutputArrayOfArrays contours, OutputArray hierarchy, int mode, int method, Point offset=Point())
		findContours(canny_gray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		Mat drawing = Mat::zeros(canny_gray.size(), CV_8UC3);
		for (int i = 0; i < contours.size(); i++)//draws all contours
		{
			//drawContours(mat drawing, vector vector point, what contours to draw, color or contors,line thickness, line type, hierarchy, maxlevel ,offset
			if (contours[i].size() >= 100)
			{
				Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));//random color for each contour
				drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());//draws the contour image
			}
		}
		//imshow("Image with Contours", drawing);

		///Bonding box
		try
		{
			///vectors for bounding rectangles and center circles
			vector<vector<Point> > contours_poly(contours.size());
			vector<Rect> boundRect(contours.size());
			vector<Point2f>CenterPoints(contours.size());
			vector<float>radius(contours.size());
			for (int i = 0; i < contours.size(); i++)
			{
				approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
				boundRect[i] = boundingRect(Mat(contours_poly[i]));
			}
			for (int i = 0; i < contours.size(); i++)
			{
				if (boundRect[i].area() > 2500)
				{
					CenterPoints[i] = Point2f((boundRect[i].width / 2) + boundRect[i].x, (boundRect[i].height / 2) + boundRect[i].y);//array of center points of bounding rectangles
				}
			}
			for (int i = 0; i < contours.size(); i++)
			{
				for (int x = 0; x < contours.size(); x++)
				{
					if (boundRect[i].area() >= 2500 && boundRect[i].area() <= 50000 && ((boundRect[i].height) / (boundRect[i].width)) == 1)//2500,50000
					{
						if (PointCompare(CenterPoints[i], CenterPoints[x]) && i != x && hierarchy[i][2]>0)//looks for 2 rectcangle with simular center points
						{
							rectangle(frame, boundRect[i].tl(), boundRect[i].br(), RED, 2, 8, 0);//rectanle

							circle(frame, (CenterPoints[i], CenterPoints[x]), 5, GREEN, 1, 8, 0);//center of rectangles
							//arrowedLine(Mat& img, Point pt1, Point pt2, const Scalar& color, int thickness = 1, int line_type = 8, int shift = 0, double tipLength = 0.1)
							arrowedLine(frame, (CameraCenter, CameraCenter), (CenterPoints[i], CenterPoints[x]), PURPLE, 2, 8, 0, .1);
							LeftRightDistance = CameraCenter.x - CenterPoints[x].x;//distance to center from target
							DistanceToTarget = (TARGET_WIDTH * FOCAL_LENGTH) / boundRect[i].width;//distance from camera to target    (real width of target inches*focal length)/rectangle width from camera pixels
							FoundTarget = true;//is there a target
						}
					}
				}
			}

			circle(frame, (CameraCenter, CameraCenter), 5, BLUE, 1, 8, 0);//center camera circle
		}
		catch (...) {}
#pragma endregion edits video input to find double square image, finds target center

#pragma region Alignment

		
		if (FoundTarget && LeftRightDistance > -10 && LeftRightDistance < 10)//left or right 10 pixel tolorence
		{
			LeftRightAligned = true;
			//putText(Mat& img, const string& text, Point org, int fontFace, double fontScale, Scalar color, int thickness = 1, int lineType = 8, bool bottomLeftOrigin = false)
			putText(frame, "ALIGNED", Point2f(300, 25), FONT_HERSHEY_PLAIN, 2, BLACK, 3, 8, false);//text if target is aligned in tolorence
		}
		///Text in top left corner with measurements to target
		if (FoundTarget)
		{
			FoundTargetSTR = "Target Aquired";
		}
		else
		{
			FoundTargetSTR = "Target Not Found";
		}

		if (LeftRightDistance > 0)
		{
			LeftRightSTR = "Distance to center from left: " + to_string(abs(LeftRightDistance));
		}
		else
		{
			LeftRightSTR = "Distance to center from right: " + to_string(abs(LeftRightDistance));
		}

		putText(frame, FoundTargetSTR, Point2f(0, 10), FONT_HERSHEY_PLAIN, .8, PURPLE, 1.5, 8, false);//text if target was found
		putText(frame, LeftRightSTR, Point2f(0, 20), FONT_HERSHEY_PLAIN, .8, PURPLE, 1.5, 8, false);//text for distance to center from target
		putText(frame, "Distance to Target: " + to_string(DistanceToTarget) + " Inches", Point2f(0, 30), FONT_HERSHEY_PLAIN, .8, PURPLE, 1.5, 8, false);//text for distace from camera to target
		if (DistanceToTarget > 8 && DistanceToTarget < 15)
		{
			DistanceAligned = true;
			/*ONLY FOR TEST*/cout << "Distance Aligned" << endl;
		}

		imshow("Bounding retangle", frame);//final image
#pragma endregion Finds left and right allignment and distance (use this data to move robot)
	}
	cout << "out of loop" << endl;
}

///used to find distance between points
bool PointCompare(Point2f point1, Point2f point2)
{
	if (abs(point1.x - point2.x) < 5 && abs(point1.y - point2.y) < 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}