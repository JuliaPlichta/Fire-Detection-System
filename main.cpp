#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include<iostream>
#include<stdio.h>
#include<conio.h>
#include<Windows.h>

using namespace cv;
using namespace std;

int main() {
	VideoCapture cap;

	int choice = 0;

	cout << "1. Camera" << endl;
	cout << "2. Video from your computer" << endl;
	cout << "Choose source video:" << endl;
	cin >> choice;

	switch (choice) 
	{
		case 1: {
			cap.open(0);
			break;
		}
		case 2: {
			cout << "Specify full pathname:" << endl;
			string pathname;
			cin >> pathname;
			cap.open(pathname);
			break;
		}
		default: {
			cout << "Wrong choice!" << endl;
			waitKey(1000);
			return -1;
		}
	}

	if (!cap.isOpened()) {
		cout << "Cannot open camera/video";
		waitKey(1000);
		return -1;
	}

	Mat frame, frame_gray, frame_prev, frame_prev_gray, frame_diff, frame_thresh, frame_thresh_dilate, frame_hsv;
	Mat mask1, mask2;
	int thresh_value = 20;
	int max_thresh_value = 255;

	cap >> frame;

	Scalar yellow = Scalar(0, 250, 250);
	Scalar blue = Scalar(255, 60, 0);
	Scalar red = Scalar(0, 0, 255);

	int is_fire = 0;

	while (true)
	{
		frame.copyTo(frame_prev);
		cvtColor(frame_prev, frame_prev_gray, COLOR_BGR2GRAY);

		cap >> frame;
		if (frame.empty()) {
			break;
		}

		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
		absdiff(frame_gray, frame_prev_gray, frame_diff);

		threshold(frame_diff, frame_thresh, thresh_value, max_thresh_value, THRESH_BINARY);
		dilate(frame_thresh, frame_thresh_dilate, Mat(), Point(-1, -1), 3, 0);
		erode(frame_thresh_dilate, frame_thresh_dilate, Mat(), Point(-1, -1), 1, 0);

		Mat frame_copy;
		frame.copyTo(frame_copy);
		Mat frame_motion;
		bitwise_and(frame_copy, frame_copy, frame_motion, frame_thresh_dilate);

		//========== color =================
		//GaussianBlur(frame_motion, frame_motion, Size(15, 15), 0);
		cvtColor(frame_motion, frame_hsv, COLOR_BGR2HSV);

		Mat frame_fire, frame_fire_gray;
		inRange(frame_hsv, Scalar(0, 0, 160), Scalar(30, 255, 255), mask1);
		inRange(frame_hsv, Scalar(160, 50, 160), Scalar(180, 255, 255), mask2);

		Mat1b mask = mask1 | mask2;

		bitwise_and(frame, frame_hsv, frame_fire, mask);
		
		cvtColor(frame_fire, frame_fire_gray, COLOR_HSV2BGR);
		cvtColor(frame_fire_gray, frame_fire_gray, COLOR_BGR2GRAY);

		Mat frame_fire_binary;
		blur(frame_fire_gray, frame_fire_binary, Size(3, 3));
		threshold(frame_fire_binary, frame_fire_binary, 50, 255, THRESH_BINARY);

		vector<vector<Point>> contours;
		findContours(frame_fire_binary, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

		vector<vector<Point>> hull(contours.size());
		vector<double> contours_perimeter(contours.size());
		vector<double> hull_perimeter(contours.size());
		vector<double> ratio(contours.size());

		for (size_t i = 0; i < contours.size(); i++)
		{
			convexHull(contours[i], hull[i]);
			contours_perimeter[i] = contours[i].size();
			hull_perimeter[i] = hull[i].size();
			ratio[i] = hull_perimeter[i] / contours_perimeter[i];
		}

		for (size_t i = 0; i < contours.size(); i++)
		{
			if (ratio[i] <= 0.6) {
				drawContours(frame, contours, (int)i, blue, -1);
				is_fire = 2;
			}
			else if (ratio[i] > 0.6 && ratio[i] < 0.8) {
				drawContours(frame, contours, (int)i, yellow, -1);
				if (is_fire == 0)
					is_fire = 1;
			}
			//else
			//	putText(frame, "WARNING!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, yellow, 2);
		}

		if (is_fire == 2) {
			putText(frame, "FIRE!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, red, 2);
			
		}

		if (is_fire == 1)
			putText(frame, "WARNING!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, yellow, 2);
		is_fire = 0;

		//int non_zero = countNonZero(frame_fire_gray);
		//if (non_zero > 1) {
		//	if (non_zero > 30)
		//		putText(frame, "FIRE!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, blue, 2);
		//		//fire;
		//	else 
		//		putText(frame, "WARNING!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, yellow, 2);
		//		//warning
		//}

		imshow("Frame", frame);
		imshow("Frame Fire", frame_fire);

		waitKey(15);
		if (GetKeyState(VK_ESCAPE) & 0x8000) {
			return 0;
		}
	}
	cap.release();
	destroyAllWindows();

	return 0;
}