# 银行卡数字识别
## <font color=pink>效果显示</font>
01号图：  
![](pic\\12.png)  

05号图:  
![](pic\\13.png)
## <font color=pink>一些小改进的地方</font>
1. 在对银行卡的数字逐个与模板匹配的时候，仅仅只做大小处理Size(60,80)、灰度处理之后效果并不好，数字识别错误的情况很严重    
   改进：对银行卡匹配的图像以及模板图像都做一个二值化处理，将大小调整为 Size(20, 25)
   ~~~C++
   //这里是对银行卡的每个数字区域做二值化处理
   threshold(Num_gray, Num_gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
   //对模板进行二值化 在这里有一点搞不明白threshold这个函数 用了inRange函数
   inRange(temp_gray, Scalar(0, 0, 0), Scalar(0, 0, 0), temp_gray);   
   ~~~

2. 做了大小调整和二值化处理之后，效果也不完全理想。然后我就把matchTemplate函数改成了相关性匹配法
   > 相关性匹配方法：完全匹配会得到很大值，不匹配会得到一个很小值或0。
   ~~~C++
   matchTemplate(Num_gray, temp_gray, result, TM_CCORR);
   ~~~

3. 关于银行卡数字区域大小
   在把所有银行卡大小都统一成Size(442,279)时，将检索到的轮廓逆合成矩形，通过cout矩形的宽高比，可以发现其实每张银行卡的宽高比都不太一样  
   01卡的条件是ratio > 3 && ratio < 4（ratio是宽高比）  
   05卡的条件是ratio > 2.8 && ratio < 3.1
   ~~~C++
   resize(B_Img, B_Img, Size(442, 279), 1, 1, INTER_LINEAR);
   ~~~
   ~~~C++
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
   ~~~



## <font color=pink>首先处理模板，把模板上的字先抠出来</font>
### 将图片转换为灰度图，进行二值化，找轮廓
通过contours.size()可以得知函数找到了十个轮廓“#10”  
因为图片二值化之后，在图上画轮廓很难观察，所以我画在了原图上

~~~C++
vector<vector<Point>> conPoly(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		float peri = arcLength(contours[i], true);
		approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
		//存储矩形边界框向量
		vector<Rect> boundRect(contours.size());
		//绘制矩形边界框，将含纳每一个独立的形状
		boundRect[i] = boundingRect(conPoly[i]);
		//将边界框打印在原图上
		rectangle(Img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);
	}
~~~

![](pic\\1.png)

### 之后要做的工作就是把每一个数字的框框（就是上图中的绿色部分给分割出来 存成模板
~~~c++
vector<vector<Point>> contours;
	vector< Vec4i > hierarchy;

	findContours(ImgInRange, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	cout << contours.size() << endl;

	vector<vector<Point>> conPoly(contours.size());
	vector<Card> Cards;
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
	}
	for (int i = 0; i < Cards.size(); i++) {
		imshow("test", Cards[i].mat);
		waitKey(0);
	}
	
~~~
按照上述代码可以得到这几张演示效果  
![](pic\\2.png)
![](pic\\3.png)
![](pic\\4.png)
![](pic\\5.png)
![](pic\\6.png)
![](pic\\7.png)
![](pic\\8.png)
![](pic\\9.png)
![](pic\\10.png)
![](pic\\11.png)

### 之后把上面这个代码封装一下变成函数Get_Template

## <font color="pink">特定操作---银行卡数字区域提取</font>
**闭操作的作用**
+ 消除值低于邻近点的孤立点，达到去除图像中噪声的作用；
+ 连接两个邻近的连通域；
+ 弥合较窄的间断和细长的沟壑；
+ 去除连通域内的小型空洞；
+ 和开运算一样也能够平滑物体的轮廓；
  
## <font color="pink">循环银行卡数字区域进行匹配</font>
使用到了MatchTemplate函数