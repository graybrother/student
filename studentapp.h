#ifndef STUDENT_H
#define STUDENT_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "opencv.hpp"

#define RAWYMOVE  0.8
#define STANDUPTHRESHOLD 15   //y move threshold to determine a standup action
#define STANDUPTIME 90   //time duration to detect a standup action
#define STANDUPTIMEOUT 450  // time duration for a standup student
#define LKPOINTS   10   //lk tracking points threshold
#define MAXMOVETIME 8   //a time duration for a standup student to reach full standup
#define TOOFAST  3


#define MAXSTUDENTNUM   30
#define MAXRECTNUM   30
#define XGAP 4
#define YGAP 4

#define MATCHAREA  800

typedef struct student_Feature
{
    int objID;
    int trustedValue;
    int notmoveCount;
    int seatPosition;  //前后分级 9级；
    float standupThresHold;
    float lrThreshold;
    bool isCurrentObj;
    bool isStandup;
    int standupTimeout;
    int maxMoveTimeout;

   //for correspondent Rect from vibe
   // int rectIndex;
    //points for lk
    std::vector<cv::Point2f> points[2];
    //initial points
    std::vector<cv::Point2f> initial;

    //data of tracking result
    int trackedPointNum;
    int rectIndex;
    cv::Rect initialRect;
    cv::Rect rect;
 //   cv::Point2f center;

    float moveX;
    float moveY;
    float maxMoveY;
}student_Feature_t;


void Student_Feature_Init(student_Feature_t &);
void SetRawPoints(std::vector<cv::Point2f> &, int, int, int *start, int *end);
void SetPoints(std::vector<cv::Point2f> &, std::vector<cv::Point2f> &,cv::Rect &);
void FindLKObj(std::vector<cv::Point2f> &, std::vector<cv::Point2i> &, int &);
bool isMatched(cv::Rect &,cv::Rect &);

void SetSTRawPoints(std::vector<cv::Point2f> &rawpoints, std::vector<cv::Point2f> &features,int start_y, int *start,int *end);
void FindSTLKObj(std::vector<cv::Point2f> &point , std::vector<cv::Point2f> &Center,std::vector<cv::Point2f> *classPoints,int &objNum);

void SetCannyRawPoints(std::vector<cv::Point2f> &, cv::Mat &,int , int , int *,int *);

int part(std::vector<cv::Rect>& rects, std::vector<int>& labels,double eps);
bool ssimRects(cv::Rect &r1,cv::Rect &r2,double eps);

void groupPoints(std::vector<cv::Point2i>& pointList, int groupThreshold, int eps);

//bool isSameRect(cv::Rect &,cv::Rect &);
//cv::Rect getRect(std::vector<cv::Point2f> &);
//void BubbleSort(std::vector<cv::Rect> &, int );
#endif // STUDENT_H
