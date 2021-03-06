#pragma once
#include "SIFT_feature.h"
#include "kdtree.h"
#include "minpq.h"

class SIFT_opt_tracker
{
public:
	SIFT_opt_tracker(void);
	~SIFT_opt_tracker(void);
	SIFT_opt_tracker(SIFT_feature* Sfeat,IplImage* preF,int Sfeat_num_fp,Rect trackingRect)
	{
		tracking_template = Sfeat;
		//this->Sfeat_num = Sfeat_num_fp;
		kd_root = kdtree_build(Sfeat->GetFeat(0),tracking_template->GetLength());
		this->TrackingWindow.upper = trackingRect.upper-cvRound(TRACKING_WINDOW_SIZE*trackingRect.height);
		this->TrackingWindow.left = trackingRect.left-cvRound(TRACKING_WINDOW_SIZE*trackingRect.width);
		this->TrackingWindow.width = trackingRect.width+cvRound(TRACKING_WINDOW_SIZE*trackingRect.width);
		this->TrackingWindow.height = trackingRect.height+cvRound(TRACKING_WINDOW_SIZE*trackingRect.height);
		for (int i=0;i<tracking_template->GetLength();i++)
		{
			optflow.push_back(Point2D(tracking_template->GetFeat(i)->y+trackingRect.upper,tracking_template->GetFeat(i)->x+trackingRect.left));
		}
		this->preFrame = cvCloneImage(preF);
	};
	bool tracking(ImageHandler* imhdr,SIFT_feature *Sfeat, int Sfeat_num_fp, Rect *trackingwindow,Rect *trackingRect);
	Point2D CaculatePointVector(SIFT_feature_unit sfu,IplImage ipim);
	/*Point2D GetCentroid();
	double GetDensity();*/

private:
	//int Sfeat_num;
	kd_node *kd_root;
	SIFT_feature *tracking_template;
	IplImage* preFrame;
	Rect TrackingWindow;
	vector<Point2D> optflow;
};
