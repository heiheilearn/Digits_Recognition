#pragma once
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgcodecs.hpp>
using namespace std;
using namespace cv;


struct Card
{
	Mat mat;
	Rect rect;
};

bool Get_Template(Mat Img, vector<Card>& Cards);
bool Cut_BankCard(Mat B_Img, vector<Card>& Bank_ROI);
bool Cut_Num_ROIS(vector<Card>& Bank_ROI, vector<Card>& Num_ROIs);
void Get_Results(vector<Card>& Cards, vector<Card>& Bank_ROI, vector<Card>& Num_ROIs, vector<int>& results);

