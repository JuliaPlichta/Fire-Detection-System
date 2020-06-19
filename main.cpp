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

	cout << "0. Camera" << endl;
	cout << "1. Video file 1" << endl;
	cout << "2. Video file 2" << endl;
	cout << "3. Video file 3" << endl;
	cout << "4. Video file 4" << endl;
	cout << "Choose source video:" << endl;
	cin >> choice;

	switch (choice) 
	{
		case 0: {
			cap.open(0);
			break;
		}
		case 1: {
			cap.open("firevideo1.mp4");
			break;
		}
		case 2: {
			cap.open("firevideo2.mp4");
			break;
		}
		case 3: {
			cap.open("firevideo3.mp4");
			break;
		}
		case 4: {
			cap.open("firevideo4.mp4");
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

	Mat frame, frame_gray, frame_prev, frame_prev_gray, frame_diff, frame_thresh, frame_thresh_dilate;
	int thresh_value = 20;
	int max_thresh_value = 255;

	cap >> frame;

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

		imshow("Frame", frame);
		imshow("Frame Diff", frame_diff);
		imshow("Frame Thresh", frame_thresh);
		imshow("Frame Thresh Dilate", frame_thresh_dilate);

		waitKey(15);
		if (GetKeyState(VK_ESCAPE) & 0x8000) {
			return 0;
		}
	}
	cap.release();
	destroyAllWindows();

	return 0;
}