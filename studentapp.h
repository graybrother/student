#ifndef STUDENT_H
#define STUDENT_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "opencv.hpp"

#define RAWYMOVE  0.5
#define STANDUPTHRESHOLD 15   //y move threshold to determine a standup action
#define STANDUPTIME 20   //time duration to detect a standup action
#define STANDUPTIMEOUT 450  // time duration for a standup student
#define LKPOINTS   10   //lk tracking points threshold
#define MAXMOVETIME 8   //a time duration for a standup student to reach full standup
#define TOOFAST  3


#define MAXSTUDENTNUM   20
#define MAXRECTNUM   20
#define XGAP 3
#define YGAP 3

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
void FindLKObj(std::vector<cv::Point2f> &, std::vector<cv::Point2f> &, int &);
bool isMatched(cv::Rect &,cv::Rect &);

void SetSTRawPoints(std::vector<cv::Point2f> &rawpoints, std::vector<cv::Point2f> &features,int width, int height, int *start,int *end);
void FindSTLKObj(std::vector<cv::Point2f> &point , std::vector<cv::Point2f> &Center,std::vector<cv::Point2f> *classPoints,int &objNum);
//bool isSameRect(cv::Rect &,cv::Rect &);
//cv::Rect getRect(std::vector<cv::Point2f> &);
//void BubbleSort(std::vector<cv::Rect> &, int );
#endif // STUDENT_H
