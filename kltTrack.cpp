#include <iostream>
#include <fstream>
#include <cv.h>
#include <highgui.h>
#include <vector>
#include <sstream>

#include "klt.h"
#include "pnmio.h"
#include "kltTrack.h"


using namespace std;
using namespace cv;

int kltTrack(string vidPath, int startFrm, int numFrmsToTrk, string trksPath)
{
    string postfix = ".avi";
    int postfixPos = vidPath.find(postfix);
    char vidNumChar[10];
    vidPath.copy(vidNumChar, 3, (postfixPos - 3));
    vidNumChar[3]='\0';
    string vidNumStr(vidNumChar);
	stringstream convert(vidNumStr);

	int vidNum = -1;
	if ( !(convert >> vidNum) )
		vidNum = -1;
	//cout<<"vidNum: "<<vidNum<<endl;

	trksPath += vidNumStr;
	trksPath += ".tracks";

	VideoCapture capture(vidPath);
	if (!capture.isOpened()) {
		cout << "Error in opening the video";
		return -1;
	}

	int endFrm = startFrm + numFrmsToTrk - 1;
	cout << endFrm << endl;
	int frmsNum = capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "frames number:" << frmsNum << endl;
	if(endFrm > frmsNum)
	{
		endFrm = frmsNum;
		numFrmsToTrk = endFrm - startFrm + 1;
	}

	unsigned char *img1, *img2;
	KLT_TrackingContext tc;
	KLT_FeatureList fl;
	KLT_FeatureTable ft;
	int numFeat = 300;

    tc = KLTCreateTrackingContext();
	fl = KLTCreateFeatureList(numFeat);
	ft = KLTCreateFeatureTable(numFrmsToTrk, numFeat);
	tc->sequentialMode = TRUE;
	tc->writeInternalImages = FALSE;
	tc->affineConsistencyCheck = 2;

//	CvCapture* capture2 = 0;
//	capture2 = cvCreateFileCapture(vidPath.c_str());
//	double fps = cvGetCaptureProperty (capture2, CV_CAP_PROP_FPS);
//
//	CvSize size = cvSize((int)cvGetCaptureProperty(capture2, CV_CAP_PROP_FRAME_WIDTH),
//			(int)cvGetCaptureProperty( capture2, CV_CAP_PROP_FRAME_HEIGHT));
//
//	CvVideoWriter *writer = cvCreateVideoWriter("Output.avi",CV_FOURCC('M','J','P','G'), fps, size);

	vector<track> tracks;
	Mat currentFrm, grayCurFrm;
	for(int i = 0; i < startFrm; ++i)
		capture >> currentFrm;
	cvtColor(currentFrm, grayCurFrm, CV_BGR2GRAY);
	int ncols = currentFrm.cols, nrows = currentFrm.rows;

	img1 = (unsigned char *) malloc(ncols * nrows * sizeof(unsigned char));
	img2 = (unsigned char *) malloc(ncols * nrows * sizeof(unsigned char));
	int idx = 0;
	for (int i = 0; i < grayCurFrm.rows; ++i)
	{
		for (int j = 0; j < grayCurFrm.cols; ++j)
		{
			img1[idx] = grayCurFrm.at<uchar>(i, j);
			idx++;
		}
	}

	KLTSelectGoodFeatures(tc, img1, ncols, nrows, fl);
	KLTStoreFeatureList(fl, ft, 0);


	VideoWriter record("OUTPUT.avi", CV_FOURCC('D','I','V','X'), 5, currentFrm.size(), true);
	//VideoWriter record("OUTPUT.avi", CV_FOURCC('P','I','M,'I'), 5, originalFrame.size(), true);
	if( !record.isOpened() ) {
			printf("VideoWriter failed to open!\n");
			return -1;
	}

	for (int frmIdx = 1; frmIdx < numFrmsToTrk; ++frmIdx)
	{
		cout << frmIdx << endl;
		capture >> currentFrm;
		cvtColor(currentFrm, grayCurFrm, CV_BGR2GRAY);

		int idx = 0;
		for (int i = 0; i < grayCurFrm.rows; ++i)
		{
			for (int j = 0; j < grayCurFrm.cols; ++j)
			{
				img2[idx] = grayCurFrm.at<uchar>(i, j);
				idx++;
			}
		}

		KLTTrackFeatures(tc, img1, img2, ncols, nrows, fl);
		KLTReplaceLostFeatures(tc, img2, ncols, nrows, fl);
		KLTStoreFeatureList(fl, ft, frmIdx);

		/**
		 * Creating the Video
		 */
//		for (int i = 0; i < ft->nFeatures; i++) {
//			for (int j = frmIdx;
//					j >= 0 && ft->feature[i][j]->val >= 0; j--) {
//				if (ft->feature[i][j]->val > 0) {
//					circle(currentFrm,
//							Point(ft->feature[i][j]->x,
//									ft->feature[i][j]->y), 4,
//							CV_RGB(0, 255, 0), -1);
//
//					break;
//				} else if (ft->feature[i][j]->val == 0) {
//					circle(currentFrm,
//							Point(ft->feature[i][j]->x,
//									ft->feature[i][j]->y), 2,
//							CV_RGB(255, 0, 0), -1);
//				}
//
//				if (ft->feature[i][j]->val < 0)
//					cout << "#########Negative##########" << endl;
//			}
//		}


		for (int n = 0; n < ft->nFeatures; ++n)
		{
//			cout << ft->feature[n][frmIdx - 1]->val << endl;
//			cout << " " << (ft->feature[n][frmIdx - 1]->val == 0 )<< endl;
			struct track trk;
			//trk.trackId = n;
			if (ft->feature[n][frmIdx]->val == 0)
			{
//				struct track trk;
				trk.endFrm = frmIdx + startFrm + 1;
				trk.endPos.x = ft->feature[n][frmIdx]->x;
				trk.endPos.y = ft->feature[n][frmIdx]->y;
				int startIdx = frmIdx - 1;
				for (; startIdx >= 0 && ft->feature[n][startIdx]->val == 0; --startIdx);
				if(startIdx >= 0 && ft->feature[n][startIdx]->val > 0)
				{
					trk.startPos.x = ft->feature[n][startIdx]->x;
					trk.startPos.y = ft->feature[n][startIdx]->y;
					trk.startFrm = startIdx + startFrm + 1;

//					line(currentFrm, trk.startPos, trk.endPos, CV_RGB(255, 0, 0), 1, 8, 0);
//					circle(currentFrm, trk.endPos, 2, CV_RGB(255, 0, 0), -1);
					Point2d vec(trk.startPos.x - trk.endPos.x, trk.startPos.y - trk.endPos.y);
					if(norm(vec) >= 5)
					{
						line(currentFrm, trk.startPos, trk.endPos, CV_RGB(255, 0, 0), 1, 8, 0);
						circle(currentFrm, trk.endPos, 2, CV_RGB(255, 0, 0), -1);
					}
				}

				if(frmIdx == (numFrmsToTrk - 1) )
					tracks.push_back(trk);

//				if(ft->feature[n][frmIdx]->val == 0)// if the track ends, push it into tracks
//					tracks.push_back(trk);
			}
			else // ft->feature[n][frmIdx]->val > 0: old track ends and new track starts
			{
				//trk.trackId = n * 1000;
				if(ft->feature[n][frmIdx - 1]->val ==0)
				{
					trk.endFrm = frmIdx + startFrm;
					trk.endPos.x = ft->feature[n][frmIdx - 1]->x;
					trk.endPos.y = ft->feature[n][frmIdx - 1]->y;

					int startIdx = frmIdx - 2;
					for (; startIdx >= 0 && ft->feature[n][startIdx]->val == 0; --startIdx);
					if(startIdx >= 0 && ft->feature[n][startIdx]->val > 0)
					{
						trk.startPos.x = ft->feature[n][startIdx]->x;
						trk.startPos.y = ft->feature[n][startIdx]->y;
						trk.startFrm = startIdx + startFrm + 1;

						tracks.push_back(trk);

					}

				}

			}
		}

//		imshow(vidPath, currentFrm);
//		record << currentFrm;
//
////		string frmName = "kltImages/";        // string which will contain the result
////		ostringstream convert;   // stream used for the conversion
////		convert << frmIdx;      // insert the textual representation of 'Number' in the characters in the stream
////
////		frmName = frmName + convert.str() + ".jpg";
////		imwrite(frmName, currentFrm);
//		cvWaitKey(2);

	}


	ofstream fout(trksPath.c_str());
	cout << tracks.size() << endl;
	for(unsigned int i = 0; i < tracks.size(); ++i)
	{
		//fout << tracks[i].trackId << " ";
		fout << tracks[i].startFrm << " " << tracks[i].endFrm << " " << tracks[i].startPos.x << " ";
		fout << tracks[i].startPos.y << " " << tracks[i].endPos.x << " " << tracks[i].endPos.y << endl;
	}

	fout.close();

	//KLTWriteFeatureTable(ft, "features.txt", "%5.1f");
//	cvReleaseCapture(&capture2);
//	cvReleaseVideoWriter(&Writer);
	free(img1);
	free(img2);
	KLTFreeFeatureTable(ft);
	KLTFreeFeatureList(fl);
	KLTFreeTrackingContext(tc);
	return 1;
}

int main(int argc, char* argv[]) {
	if(argc != 3)
	{
		//cout<<argc<<endl;
		cout<<"Wrong arguments!"<<endl;
		return 0;
	}

	string vidPath(argv[1]);
	int startFrm = atoi(argv[2]);

	cout<<vidPath<<endl;
	clock_t t;
	t = clock();
//	int startFrm  = 60; //100;
	int numFrmsToTrk = 45;
	string trksPath = "Tracks/video";
	kltTrack(vidPath, startFrm, numFrmsToTrk, trksPath);
	//int kltRet = kltTrack(vidPath);//, lineDir, mosPathStr);
	t = clock() - t;
	printf ("KLT took (%f seconds).\n",((float)t)/CLOCKS_PER_SEC);
}
