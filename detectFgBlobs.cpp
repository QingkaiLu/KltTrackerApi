#include <iostream>
#include <string>

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

using namespace std;
using namespace cv;

void HSVtoRGB(float hsv[3], float rgb[3]) {
	float tmp1 = hsv[2] * (1-hsv[1]);
	float tmp2 = hsv[2] * (1-hsv[1] * (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) ));
	float tmp3 = hsv[2] * (1-hsv[1] * (1 - (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) )));
	switch((int)(hsv[0] / 60)) {
		case 0:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp3 ;
			rgb[2] = tmp1 ;
			break;
		case 1:
			rgb[0] = tmp2 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp1 ;
			break;
		case 2:
			rgb[0] = tmp1 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp3 ;
			break;
		case 3:
			rgb[0] = tmp1 ;
			rgb[1] = tmp2 ;
			rgb[2] = hsv[2] ;
			break;
		case 4:
			rgb[0] = tmp3 ;
			rgb[1] = tmp1 ;
			rgb[2] = hsv[2] ;
			break;
		case 5:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp1 ;
			rgb[2] = tmp2 ;
			break;
		default:
			cout << "What!? Inconceivable!\n";
	}

}


void detectFgBlobs(){ //need to be tested

	for(int frameIndex = 1; frameIndex <= 416; ++frameIndex)
	{
		ostringstream convertFrmIdx;
		convertFrmIdx << frameIndex;
		convertFrmIdx.str();

		string fgImgPathStr = "fgImages/Game01_video0003.avi_" + convertFrmIdx.str() + ".bmp";
		cout << fgImgPathStr << endl;
		Mat fgImg = imread(fgImgPathStr.c_str(), CV_LOAD_IMAGE_COLOR);

		vector<vector<int> > pixelsBlobIdx;
		vector<int> pixelsBlobIdxOneRow(fgImg.cols, -1);
		for(int i = 0; i < fgImg.rows; ++i)
				pixelsBlobIdx.push_back(pixelsBlobIdxOneRow);
//		for(int i = 0; i < 100; ++i)
//			for(int j = 0; j < 100; ++j)
//			{
//				cout << pixelsBlobIdx[i][j] <<" ";
//			}
		vector<vector<CvPoint> > blobClasses;
		for(int i = 0; i < fgImg.rows; ++i)
			for(int j = 0; j < fgImg.cols; ++j)
//		for(int i = 0; i < 100; ++i)
//			for(int j = 0; j < 1000; ++j)
			{
				Point3_<uchar>* p = fgImg.ptr<Point3_<uchar> >(i,j);
				if(int(p->z) != 255)
					continue;
//				p->x //B
//				p->y //G
//				p->z //R
				bool existClassedNb = false;
				int classIdx = -1;
				if(i > 1 && j > 1 && !existClassedNb)
				{
					//cout << "###" << endl;
					Point3_<uchar>* pNb = fgImg.ptr<Point3_<uchar> >(i - 1, j - 1);
					if(int(pNb->z) == 255)
					{
						existClassedNb = true;
						classIdx = pixelsBlobIdx[i - 1][j - 1];
					}
				}

				if(i > 1 && !existClassedNb)
				{
					Point3_<uchar>* pNb = fgImg.ptr<Point3_<uchar> >(i - 1, j);
					if(int(pNb->z) == 255)
					{
						existClassedNb = true;
						classIdx = pixelsBlobIdx[i - 1][j];
					}
				}

				if(i > 1 && (j + 1) < fgImg.cols && !existClassedNb)
				{
					Point3_<uchar>* pNb = fgImg.ptr<Point3_<uchar> >(i - 1, j + 1);
					if(int(pNb->z) == 255)
					{
						existClassedNb = true;
						classIdx = pixelsBlobIdx[i - 1][j + 1];
					}
				}

				if(j > 1 && !existClassedNb)
				{
					Point3_<uchar>* pNb = fgImg.ptr<Point3_<uchar> >(i, j - 1);
					if(int(pNb->z) == 255)
					{
						existClassedNb = true;
						classIdx = pixelsBlobIdx[i][j - 1];
					}
				}

				if(existClassedNb)
				{
					CvPoint pnt;
					pnt.x = i;
					pnt.y = j;
					blobClasses[classIdx].push_back(pnt);
					pixelsBlobIdx[i][j] = classIdx;
//					p->x = uchar(0);
//					p->y = uchar(0);
//					p->z = uchar(0);
				}
				else
				{
					vector<CvPoint> blobCls;
					CvPoint pnt;
					pnt.x = i;
					pnt.y = j;
					blobCls.push_back(pnt);
					blobClasses.push_back(blobCls);
					pixelsBlobIdx[i][j] = blobClasses.size() - 1;
//					p->x = uchar(0);
//					p->y = uchar(0);
//					p->z = uchar(0);
				}
//				if(int(p->z) == 255)
//				{
//					p->x = uchar(0);
//					p->y = uchar(0);
//					p->z = uchar(0);
//				}

			}


//		Mat fgGrayImg;
//		cvtColor(fgImg, fgGrayImg, CV_BGR2GRAY);
//
//		for(int i = 0; i < fgGrayImg.rows; ++i)
//			for(int j = 0; j < fgGrayImg.rows; ++j)
//				cout << fgGrayImg.at<int>(i, j)  << " ";

		cout << "blobClasses " << blobClasses.size() << endl;

		vector<vector<CvPoint> > bigBlobClasses;

		for(unsigned int i = 0; i < blobClasses.size(); ++i)
		{
			if(blobClasses[i].size() > 100)
				bigBlobClasses.push_back(blobClasses[i]);
		}
		cout << "bigBlobClasses " << bigBlobClasses.size() << endl;

		for(unsigned int i = 0; i < bigBlobClasses.size(); ++i)
		{
			float hsv[3], rgb[3];
			hsv[0] = 240.0 * (1.0 - float(i) / float(bigBlobClasses.size()) );
			hsv[1] = 1.0;
			hsv[2] = 1.0;
			HSVtoRGB(hsv, rgb);
			//CV_RGB(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255),1,8,0
			for(unsigned int j = 0; j < bigBlobClasses[i].size(); ++j)
			{
				Point3_<uchar>* p = fgImg.ptr<Point3_<uchar> >(bigBlobClasses[i][j].x, bigBlobClasses[i][j].y);
				p->x = uchar(rgb[2] * 255);
				p->y = uchar(rgb[1] * 255);
				p->z = uchar(rgb[0] * 255);
			}

		}

		string FIdxStr;
		if(frameIndex < 10)
			FIdxStr = "fgBlobsImages/Game01_video0003_00" + convertFrmIdx.str() + ".bmp";
		else if(frameIndex < 100)
			FIdxStr = "fgBlobsImages/Game01_video0003_0" + convertFrmIdx.str() + ".bmp";
		else
			FIdxStr = "fgBlobsImages/Game01_video0003_" + convertFrmIdx.str() + ".bmp";

		imwrite(FIdxStr, fgImg);
		imshow("Game01_video0003", fgImg);
		cvWaitKey(2);
	}

}

//int main()
//{
//	detectFgBlobs();
//	return 0;
//}
