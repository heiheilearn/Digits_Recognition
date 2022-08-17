#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgcodecs.hpp>
#include<iostream>

using namespace std;
using namespace cv;

struct Card
{
	Mat mat;
	Rect rect;
};


bool Get_Template(Mat Img, vector<Card>& Cards) {
	Mat ImgGray, ImgInRange;
	//转成灰度图
	cvtColor(Img, ImgGray, COLOR_BGR2GRAY);

	//二值化
	inRange(ImgGray,Scalar(0,0,0),Scalar(0,0,0),ImgInRange);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(ImgInRange, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	cout << contours.size() << endl;
	for (int i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect(contours[i]);
		double ratio = double(rect.width) / double(rect.height);

		//筛选出字符轮廓
		if (ratio > 0.5 && ratio < 1)
		{
			Mat roi = Img(rect);  //将字符扣出，放入Cards容器备用
			Cards.push_back({ roi ,rect });
		}
	}
	if (Cards.empty())return false;

	//进行字符排序，使其按（0、1、2...7、8、9）顺序排序
	//冒泡排序
	for (int i = 0; i < Cards.size() - 1; i++)
	{
		for (int j = 0; j < Cards.size() - 1 - i; j++)
		{
			if (Cards[j].rect.x > Cards[j + 1].rect.x)
			{
				Card temp = Cards[j];
				Cards[j] = Cards[j + 1];
				Cards[j + 1] = temp;
			}
		}
	}		
	return true;
}

int main(){
	
	string Template_path = "C:\\Users\\Rong\\Desktop\\Digits\\ocr_a_reference.png";
	Mat Img = imread(Template_path);
	vector<Card> Cards;
	Get_Template(Img, Cards);
	/*for (int i = 0; i < Cards.size(); i++) {
		imshow("test", Cards[i].mat);
		waitKey(0);
	}*/

	string Bank_Card_path = "C:\\Users\\Rong\\Desktop\\Digits\\credit_card_05.png";
	Mat B_Img = imread(Bank_Card_path);
	
	Mat B_ImgGray, B_ImgGauss,B_ImgInRange, B_ImgClose;

	cvtColor(B_Img, B_ImgGray, COLOR_BGR2GRAY);
	GaussianBlur(B_ImgGray, B_ImgGauss, Size(3, 3), 0);
	imshow("B_Gauss", B_ImgGauss);

	threshold(B_ImgGauss,B_ImgInRange, 0, 255, THRESH_BINARY | THRESH_OTSU);

	Mat kernel_ = getStructuringElement(MORPH_RECT, Size(15, 5));
	morphologyEx(B_ImgInRange, B_ImgClose, MORPH_CLOSE, kernel_);
	imshow("B_Close", B_ImgClose);

	vector<vector<Point>>contours;
	vector<Card> Bank_ROI;
	findContours(B_ImgClose, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		//通过面积、长宽比筛选出银行卡号区域
		double area = contourArea(contours[i]);

		if (area > 800 && area < 1400)
		{
			Rect rect = boundingRect(contours[i]);
			float ratio = double(rect.width) / double(rect.height);

			if (ratio > 2.8 && ratio < 3.1)
			{
				Mat ROI = B_Img(rect);
				Bank_ROI.push_back({ ROI ,rect });
			}
		}
	}
	for (int i = 0; i < Bank_ROI.size() - 1; i++)
	{
		for (int j = 0; j < Bank_ROI.size() - 1 - i; j++)
		{
			if (Bank_ROI[j].rect.x > Bank_ROI[j + 1].rect.x)
			{
				Card temp = Bank_ROI[j];
				Bank_ROI[j] = Bank_ROI[j + 1];
				Bank_ROI[j + 1] = temp;
			}
		}
	}
	
	for (int i = 0; i < Bank_ROI.size(); i++) {
		imshow("Bank_ROI", Bank_ROI[i].mat);
		waitKey(0);
	}

	waitKey(0);
}
