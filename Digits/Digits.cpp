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
	threshold(ImgGray, ImgInRange, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	/*imshow("ImgInRange", ImgInRange);
	waitKey(0);*/
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
bool Cut_BankCard(Mat B_Img, vector<Card>& Bank_ROI) {
	Mat B_ImgGray, B_ImgGauss, B_ImgInRange, B_ImgClose;

	cvtColor(B_Img, B_ImgGray, COLOR_BGR2GRAY);
	GaussianBlur(B_ImgGray, B_ImgGauss, Size(3, 3), 0);
	//imshow("B_Gauss", B_ImgGauss);

	threshold(B_ImgGauss, B_ImgInRange, 0, 255, THRESH_BINARY | THRESH_OTSU);

	Mat kernel_ = getStructuringElement(MORPH_RECT, Size(15, 5));
	morphologyEx(B_ImgInRange, B_ImgClose, MORPH_CLOSE, kernel_);
	imshow("B_Close", B_ImgClose);
	waitKey(0);
	vector<vector<Point>>contours;
	findContours(B_ImgClose, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
	{
		//cout << "contours.size() " << contours.size() << endl;

		//通过面积、长宽比筛选出银行卡号区域
		double area = contourArea(contours[i]);
		cout << "contours[i].area " << area << endl;

		if (area > 800 && area < 1400)
		{
			Rect rect = boundingRect(contours[i]);
			float ratio = double(rect.width) / double(rect.height);
			//rectangle(B_Img, Rect(rect.tl(), rect.br()), Scalar(0, 255, 0), 2);
			cout << "ratio " << ratio << endl;
			if (ratio > 3 && ratio < 4)
			{
				//rectangle(B_Img, Rect(rect.tl(), rect.br()), Scalar(0, 255, 0), 2);
				Mat ROI = B_Img(rect);
				Bank_ROI.push_back({ ROI ,rect });

			}
		}
	}
	if (Bank_ROI.size() != 4)return false;
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
	return true;


}
bool Cut_Num_ROIS(vector<Card>& Bank_ROI, vector<Card>& Num_ROIs) {
	vector<vector<Point>> contours;
	Mat ImgGray, ImgInRange;
	for (int i = 0; i < Bank_ROI.size(); i++) {
		cvtColor(Bank_ROI[i].mat, ImgGray, COLOR_BGR2GRAY);
		threshold(ImgGray, ImgInRange, 0, 255, THRESH_BINARY | THRESH_OTSU);
		//imshow("ImgInRange", ImgInRange);

		findContours(ImgInRange, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		//imshow("ImgInRange_", ImgInRange);
		//waitKey(0);
		for (int j = 0; j < contours.size(); j++) {
			Rect rect = boundingRect(contours[j]);
			/*cout << "rect_width " << rect.width << endl;
			cout << "rect_height " << rect.height << endl;*/
			if (rect.height > 20 && rect.width > 10) {
				Rect ROI_Rect(rect.x + Bank_ROI[i].rect.x, rect.y + Bank_ROI[i].rect.y, rect.width, rect.height);
				Mat Num_ROI = Bank_ROI[i].mat(rect);
				Num_ROIs.push_back({ Num_ROI,ROI_Rect });
			}
		}
	}
	//cout << Num_ROIs.size() << endl;
	if (Num_ROIs.size() != 16) return false;
	for (int i = 0; i < Num_ROIs.size() - 1; i++)
	{
		for (int j = 0; j < Num_ROIs.size() - 1 - i; j++)
		{
			if (Num_ROIs[j].rect.x > Num_ROIs[j + 1].rect.x)
			{
				Card temp = Num_ROIs[j];
				Num_ROIs[j] = Num_ROIs[j + 1];
				Num_ROIs[j + 1] = temp;
			}
		}
	}
	//cout << Num_ROIs.size() << endl;
	return true;
}


int main() {

	string Template_path = "C:\\Users\\Rong\\Desktop\\Digits\\ocr_a_reference.png";
	Mat Img = imread(Template_path);
	//cout << "Img" << Img.size() << endl;
	vector<Card> Cards;
	Get_Template(Img, Cards);
	/*for (int i = 0; i < Cards.size(); i++) {
		imshow("test", Cards[i].mat);
		waitKey(0);
	}*/

	string Bank_Card_path = "C:\\Users\\Rong\\Desktop\\Digits\\credit_card_01.png";
	Mat B_Img = imread(Bank_Card_path);
	resize(B_Img, B_Img, Size(442, 279), 1, 1, INTER_LINEAR);
	cout << B_Img.size() << endl;
	vector<Card> Bank_ROI;
	Cut_BankCard(B_Img, Bank_ROI);
	vector<Card> Num_ROIs;
	int flag = Cut_Num_ROIS(Bank_ROI, Num_ROIs);
	//cout << "flag " << flag << endl;
	/*for (int i = 0; i < Bank_ROI.size(); i++) {
		imshow("test", Bank_ROI[i].mat);
		waitKey(0);
	}*/

	/*for (int i = 0; i < Num_ROIs.size(); i++) {
		imshow("test", Num_ROIs[i].mat);
		waitKey(0);
	}*/
	/*waitKey(0); */

	vector<int> label;
	for (int i = 0; i < 10; i++) {
		label.push_back(i);
	}
	/*for (int i = 0; i < 10; i++) {
		cout << label[i] << endl;
	}*/

	vector<int> results;
	for (int i = 0; i < Num_ROIs.size(); i++)
	{
		//将字符resize成合适大小，利于识别
		resize(Num_ROIs[i].mat, Num_ROIs[i].mat, Size(20, 25), 1, 1, INTER_LINEAR);


		Mat Num_gray;
		cvtColor(Num_ROIs[i].mat, Num_gray, COLOR_BGR2GRAY);
		threshold(Num_gray, Num_gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
		//imshow("NUM_ROIS", Num_gray);
		//waitKey(0);
		int maxIndex = 0;
		double Max = 0.0;
		for (int j = 0; j < Cards.size(); j++)
		{

			resize(Cards[j].mat, Cards[j].mat, Size(20, 25), 1, 1, INTER_LINEAR);

			Mat temp_gray;
			cvtColor(Cards[j].mat, temp_gray, COLOR_BGR2GRAY);
			inRange(temp_gray, Scalar(0, 0, 0), Scalar(0, 0, 0), temp_gray);
			//threshold(temp_gray, temp_gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
			/*imshow("CARDS", temp_gray);
			waitKey(0);*/
			//进行模板匹配，识别数字
			Mat result;
			matchTemplate(Num_gray, temp_gray, result, TM_CCORR);
			double minVal, maxVal;
			Point minLoc, maxLoc;

			minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
			cout << "cards" << i << "    nums" << j << "   " << maxVal << endl;
			//得分最大的视为匹配结果
			if (maxVal > Max)
			{
				Max = maxVal;
				maxIndex = j; //匹配结果
			}
		}
		//waitKey(0);
		results.push_back(maxIndex);//将匹配结果进行保存
	}

	for (int i = 0; i < results.size(); i++) {
		cout << results[i] << endl;
	}

	//将匹配结果进行显示
	for (int i = 0; i < Bank_ROI.size(); i++)
	{
		rectangle(B_Img, Rect(Bank_ROI[i].rect.tl(), Bank_ROI[i].rect.br()), Scalar(0, 255, 0), 2);
	}
	for (int i = 0; i < Num_ROIs.size(); i++)
	{
		cout << label[results[i]] << " ";
		putText(B_Img, to_string(label[results[i]]), Point(Num_ROIs[i].rect.tl()), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
	}

	imshow("Demo", B_Img);
	waitKey(0);
	destroyAllWindows();
}
