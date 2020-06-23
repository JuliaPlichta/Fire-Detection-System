#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include<iostream>
#include<stdio.h>
#include<conio.h>
#include<Windows.h>
#include<Mmsystem.h>

#define yellow Scalar(0, 250, 250)
#define blue Scalar(255, 60, 0)
#define red Scalar(0, 0, 255)
#define ESC 27
#define Q 113

#pragma comment(lib, "Winmm.lib")

using namespace cv;
using namespace std;

VideoCapture choose_video();
void help(bool is_day);
bool select_mode(int key, bool is_day);
Mat detect_motion(Mat frame, Mat frame_prev);
Mat detect_color(Mat frame, Mat frame_motion, bool is_day);
int detect_contours(Mat frame, Mat frame_fire);

int main() {
	VideoCapture cap = choose_video();
	
	if (!cap.isOpened()) {
		cout << "Cannot open camera/video";
		waitKey(1000);
		return -1;
	}

	Mat frame, frame_gray, frame_prev, frame_original;
	Mat frame_motion, frame_fire;
	int is_fire = 0;
	bool is_day = true;
	int key = waitKey(1);

	cap >> frame;
	
	//check if the first frame is a night or day image
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	if (mean(frame_gray)[0] < 70) {
		is_day = false;
	}
	help(is_day);
	
	while (true)
	{
		frame.copyTo(frame_prev);

		cap >> frame;
		if (frame.empty()) {
			break;
		}
		frame.copyTo(frame_original);
		is_day = select_mode(key, is_day);

		frame_motion = detect_motion(frame, frame_prev);
		frame_fire = detect_color(frame, frame_motion, is_day);
		is_fire = detect_contours(frame, frame_fire);

		if (is_fire == 2) {
			putText(frame, "FIRE!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, red, 2);
		}

		if (is_fire == 1)
			putText(frame, "WARNING!!!", Point(10, 30), FONT_HERSHEY_PLAIN, 2, yellow, 2);
		is_fire = 0;

		imshow("Frame", frame);
		imshow("Frame Original", frame_original);

		key = waitKey(15);
		if (key == ESC)
			break;
	}
	cap.release();
	destroyAllWindows();

	return 0;
}

VideoCapture choose_video() {
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
		break;
	}
	}
	return cap;
}

void help(bool is_day) {
	string mode;
	if (is_day)
		mode = "Daylight mode";
	else
		mode = "Night light mode";
	cout << "Current mode is: " << mode << endl;
	cout << "Press '1' to change mode" << endl;
}

bool select_mode(int key, bool is_day) {
	string mode;
	if (key == '1') {
		if (is_day) {
			is_day = false;
			mode = "Night light mode";
		}
		else {
			is_day = true;
			mode = "Daylight mode";
		}
		cout << mode << endl;
	}
	if (is_day)
		mode = "Daylight mode";
	else 
		mode = "Night light mode";
	if (key == Q) {
		help(is_day);
	}
	return is_day;
}

Mat detect_motion(Mat frame, Mat frame_prev){
	Mat frame_gray, frame_prev_gray, frame_diff, frame_thresh, frame_motion;
	int thresh_value = 20;
	int max_thresh_value = 255;

	cvtColor(frame_prev, frame_prev_gray, COLOR_BGR2GRAY);
	cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
	absdiff(frame_gray, frame_prev_gray, frame_diff);

	threshold(frame_diff, frame_thresh, thresh_value, max_thresh_value, THRESH_BINARY);
	dilate(frame_thresh, frame_thresh, Mat(), Point(-1, -1), 3, 0);
	erode(frame_thresh, frame_thresh, Mat(), Point(-1, -1), 1, 0);

	bitwise_and(frame, frame, frame_motion, frame_thresh);

	return frame_motion;
}

Mat detect_color(Mat frame, Mat frame_motion, bool is_day) {
	Mat frame_hsv, frame_fire, mask, mask1, mask2;
	GaussianBlur(frame_motion, frame_motion, Size(3, 3), 0);
	cvtColor(frame_motion, frame_hsv, COLOR_BGR2HSV);

	if (!is_day) {
		inRange(frame_hsv, Scalar(0, 0, 160), Scalar(30, 255, 255), mask1);
		inRange(frame_hsv, Scalar(160, 50, 160), Scalar(180, 255, 255), mask2);
		mask = mask1 | mask2;
	}
	else {
		inRange(frame_hsv, Scalar(0, 150, 90), Scalar(75, 255, 255), mask);
	}
	bitwise_and(frame, frame_hsv, frame_fire, mask);

	return frame_fire;
}

int detect_contours(Mat frame, Mat frame_fire) {
	Mat frame_fire_gray, frame_fire_binary;
	int is_fire = 0;

	cvtColor(frame_fire, frame_fire_gray, COLOR_HSV2BGR);
	cvtColor(frame_fire_gray, frame_fire_gray, COLOR_BGR2GRAY);

	blur(frame_fire_gray, frame_fire_binary, Size(3, 3));
	threshold(frame_fire_binary, frame_fire_binary, 50, 255, THRESH_BINARY);

	vector<vector<Point>> contours;
	findContours(frame_fire_binary, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> hull(contours.size());
	vector<double> contours_perimeter(contours.size());
	vector<double> hull_perimeter(contours.size());
	vector<double> ratio(contours.size());

	for (size_t i = 0; i < contours.size(); i++) {
		convexHull(contours[i], hull[i]);
		contours_perimeter[i] = contours[i].size();
		hull_perimeter[i] = hull[i].size();
		ratio[i] = hull_perimeter[i] / contours_perimeter[i];
	}

	for (size_t i = 0; i < contours.size(); i++) {
		if (ratio[i] <= 0.6) {
			drawContours(frame, contours, (int)i, blue, -1);
			is_fire = 2;
		}
		else if (ratio[i] > 0.6 && ratio[i] < 0.8) {
			drawContours(frame, contours, (int)i, yellow, -1);
			if (is_fire == 0)
				is_fire = 1;
		}
	}
	return is_fire;
}