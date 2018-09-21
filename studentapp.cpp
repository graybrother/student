#include "studentapp.h"


inline double dist2(cv::Point2f &a, cv::Point2f b)
{
    double x = a.x - b.x, y = a.y - b.y;
    return x*x + y*y;
}


void Student_Feature_Init(student_Feature_t &obj)
{
  /* Model structure alloc. */
 // object_Feature_t *obj = NULL;
 // obj = (object_Feature_t *)calloc(1, sizeof(*obj));

  /* Default parameters values. */
  obj.trustedValue       = -1;
  obj.notmoveCount    = 0;
  obj.isCurrentObj    = false;
  obj.isStandup=false;
  obj.tobeConfirmed=false;
  obj.haveFace=false;
  obj.startTimeout=0;
  obj.standupTimeout=0;
  obj.maxMoveTimeout=0;

  //

  obj.rect=cv::Rect(0,0,0,0);


  obj.points[0].clear();
  obj.points[1].clear();
  obj.initial.clear();
  obj.trackedPointNum=0;
  obj.moveX=0;
  obj.moveY=0;
  return;
}

void SetRawPoints(std::vector<cv::Point2f> &rawPoints, int width, int height, int *start,int *end)
{
    int i,j;
    cv::Point2f p;

    rawPoints.clear();
    for (j = 0; j < height; j+=YGAP){
        for (i = start[j]; i < end[j]; i+=XGAP){
            p.x=(float)i;
            p.y=(float)j;
            rawPoints.push_back(p);
        }
    }
    return;
}

void SetSTRawPoints(std::vector<cv::Point2f> &rawPoints, std::vector<cv::Point2f> &features, int start_y, int *start,int *end)
{
    int i,j;
    //cv::Point2f p;

    rawPoints.clear();
    for (i = 0; i < features.size(); i++)
    {
        j=features[i].y;
        if(j<start_y)
            continue;
        if(start[j]<=features[i].x && features[i].x<end[j])
            rawPoints.push_back(features[i]);
    }
    return;
}

void SetCannyRawPoints(std::vector<cv::Point2f> &rawPoints,cv::Mat &canny,int width, int height, int *start,int *end)
{
    int i,j,index;
    //cv::Point2f p;
    uint8_t *imgPtr=canny.data;
    cv::Point2f p;
    rawPoints.clear();

    for (j = 0;  j < height; j++)
    {
        index=j*width;
        for(i=start[j]; i<end[j]; i++)
        {
            if(*(imgPtr+index+i)==255)
            {
                p.x=(float)i;
                p.y=(float)j;
                rawPoints.push_back(p);
            }

        }
    }
    return;
}

void SetPoints(std::vector<cv::Point2f> &points, std::vector<cv::Point2f> &initial, cv::Rect &rect)
{
    int i,j;
    cv::Point2f p;
    int left=rect.x;
    int top=rect.y;
    int xgap=rect.width/10;
    int ygap=rect.height/20;


    points.clear();
    initial.clear();
    for (i = 0; i < 10; i++){
        for (j = 0; j < 20; j++){
            p.x=(float)(left+i*xgap);
            p.y=(float)(top+j*ygap);
            points.push_back(p);
            initial.push_back(p);
        }
    }
    return;
}


void FindLKObj(int width, int height,std::vector<cv::Point2f> &point , std::vector<cv::Point2i> &Center,int &objNum)
{
   // int T = 10;

    int xxx=16;
    int yyy=9;
    int xGap=width/xxx;
    int yGap=height/yyy;
    //int xxGap=xGap/2;
   // int xxGap=20;
   // int yyGap=yGap/2;

    int round_1_Num=0;
    int round_2_Num=0;
    int round_3_Num=0;
    int xmiddle[xxx];

    int y_1_Num=0;
    int y_2_Num=0;
    int y_3_Num=0;
    int ymiddle[yyy];

    int Num[xxx];
    int SumY[yyy];
    int SumX[xxx];
    int lastMiddle;
 //modefied 3.7
    int thresHold1=XNUMTHR;
    int thresHold2=YNUMTHR;

    std::vector<cv::Point2f> xclassPoints[xxx];
    //std::vector<cv::Point2f> yclassPoints[yyy];
    cv::Point2f p;

    int i,j,k;
    int dist1,dist2;
    int recleft[20],recright[20],rectop[20],recbottom[20];

    objNum=0;
    Center.clear();

    memset(Num,0,sizeof(Num));
    memset(SumX,0,sizeof(SumX));

    // x round 1;
    for(i = 0;i < point.size();i++)
    {
        k=(int)point[i].x/xGap;
        //   std::cout<<" x:"<<point[i].x<<" k:"<<k;
        Num[k]++;
        SumX[k]+=point[i].x;
    }
    for(k=0; k<xxx; k++){
        if(Num[k]>0)
        {
            xmiddle[round_1_Num]=SumX[k]/Num[k];
 //           std::cout<<" xmiddle="<<xmiddle[round_1_Num];
            round_1_Num++;
        }
    }
 //   std::cout<<" round_1_Num="<<round_1_Num<<std::endl;
    // x round 2
    memset(Num,0,sizeof(Num));
    memset(SumX,0,sizeof(SumX));
    for(i = 0;i < point.size();i++)
    {
        if(point[i].x<xmiddle[0])
        {
            Num[0]++;
            SumX[0]+=point[i].x;
        }
        else if(point[i].x>=xmiddle[round_1_Num-1])
        {
            Num[round_1_Num-1]++;
            SumX[round_1_Num-1]+=point[i].x;
        }
        else
        {
            for(k=1; k<round_1_Num; k++){
                if(point[i].x>=xmiddle[k-1] && point[i].x<xmiddle[k])
                {
                    dist1=abs(point[i].x-xmiddle[k-1]);
                    dist2=abs(point[i].x-xmiddle[k]);
                    if(dist1<=dist2)
                    {
                        Num[k-1]++;
                        SumX[k-1]+=point[i].x;
                    }
                    else
                    {
                    Num[k]++;
                    SumX[k]+=point[i].x;
                    }
                    break;
                   // xclassPoints[k].push_back(point[i]);
                }
            }
        }
//        for(k=1; k<round_1_Num; k++){
//            if(point[i].x>=(xmiddle[k]-xxGap) && point[i].x<(xmiddle[k]+xxGap)){
//                Num[k]++;
//                SumX[k]+=point[i].x;
//                break;
//               // xclassPoints[k].push_back(point[i]);
//            }
//        }

    }
    for(k=0; k<round_1_Num; k++){
        if(Num[k]>0){
            xmiddle[round_2_Num]=SumX[k]/Num[k];
            //     xclassPoints[round_2_Num]=xclassPoints[k];
 //           std::cout<<" xmiddle="<<xmiddle[round_2_Num];
            round_2_Num++;

        }
    }
 //   std::cout<<" round_2_Num="<<round_2_Num<<std::endl;

  //  std::cout<<"  round_2_Num:"<<round_2_Num<<std::endl;

    //  近的类合并
//     lastMiddle=-30;
//     for(k=0; k<round_2_Num; k++){
//         if((xmiddle[k]-lastMiddle)<20){
//             lastMiddle=xmiddle[k];
//             xmiddle[k]=0;
//         }
//         else{
//             lastMiddle=xmiddle[k];
//         }
//     }
//     for(k=0; k<round_2_Num; k++){
//         if(xmiddle[k]>0){
//             xmiddle[round_3_Num]=xmiddle[k];
//             round_3_Num++;
//         }
//     }
     memset(Num,0,sizeof(Num));
     memset(SumX,0,sizeof(SumX));
     for(i = 0;i < point.size();i++)
     {
         if(point[i].x<xmiddle[0])
         {
             Num[0]++;
             SumX[0]+=point[i].x;
             xclassPoints[0].push_back(point[i]);
         }
         else if(point[i].x>=xmiddle[round_2_Num-1])
         {
             Num[round_2_Num-1]++;
             SumX[round_2_Num-1]+=point[i].x;
             xclassPoints[round_2_Num-1].push_back(point[i]);
         }
         else
         {
             for(k=1; k<round_2_Num; k++){
                 if(point[i].x>=xmiddle[k-1] && point[i].x<xmiddle[k])
                 {
                     dist1=abs(point[i].x-xmiddle[k-1]);
                     dist2=abs(point[i].x-xmiddle[k]);
                     if(dist1<=dist2)
                     {
                         Num[k-1]++;
                         SumX[k-1]+=point[i].x;
                         xclassPoints[k-1].push_back(point[i]);
                     }
                     else
                     {
                         Num[k]++;
                         SumX[k]+=point[i].x;
                         xclassPoints[k].push_back(point[i]);
                     }

                     break;
                 }
             }
         }

     }
     for(k=0; k<round_2_Num; k++)
     {
         if(Num[k]>=thresHold1)
         {
             xmiddle[k]=SumX[k]/Num[k];
  //           std::cout<<" xmiddle="<<xmiddle[k]<<" Pnum="<<xclassPoints[k].size()
  //                   <<" num="<<Num[k];
         }
         else
             xmiddle[k]=0;     //this colomn does not do y class
     }
     std::cout<<"  round_2_Num:"<<round_2_Num<<std::endl;
    //end of classify at x dimension

   //start to do y dimension classify

    for(j=0; j<round_2_Num; j++){
        if(xmiddle[j]==0)
            continue;
        memset(Num,0,sizeof(Num));
        memset(SumY,0,sizeof(SumY));
        y_1_Num=0;
        y_2_Num=0;
        y_3_Num=0;
 //       for(k=0; k<yyy; k++)
 //           yclassPoints[k].clear();

        //first round y
        for(i = 0;i < xclassPoints[j].size();i++)
        {
            k=(int)xclassPoints[j][i].y/yGap;
            Num[k]++;
            SumY[k]+=xclassPoints[j][i].y;
        }
        for(k=0; k<yyy; k++){
            if(Num[k]>0)
            {
                ymiddle[y_1_Num]=SumY[k]/Num[k];
                y_1_Num++;
            }
        }
        //second round y
        memset(Num,0,sizeof(Num));
        memset(SumY,0,sizeof(SumY));
        for(i = 0;i < xclassPoints[j].size();i++)
        {
//            for(k=0; k<y_1_Num; k++){
//                if(xclassPoints[j][i].y>=(ymiddle[k]-yyGap)
//                        && xclassPoints[j][i].y<(ymiddle[k]+yyGap)){
//                    Num[k]++;
//                    SumY[k]+=xclassPoints[j][i].y;
//          //          yclassPoints[k].push_back(xclassPoints[j][i]);
//                }
//            }
            if(xclassPoints[j][i].y<ymiddle[0])
            {
                Num[0]++;
                SumY[0]+=xclassPoints[j][i].y;
   //             yclassPoints[0].push_back(xclassPoints[j][i]);
            }
            else if(xclassPoints[j][i].y>=ymiddle[y_1_Num-1])
            {
                Num[y_1_Num-1]++;
                SumY[y_1_Num-1]+=xclassPoints[j][i].y;
     //           yclassPoints[y_1_Num].push_back(xclassPoints[j][i]);
            }
            else
            {
                for(k=1; k<y_1_Num; k++){
                    if(xclassPoints[j][i].y>=ymiddle[k-1] && xclassPoints[j][i].y<ymiddle[k])
                    {
                        dist1=abs(xclassPoints[j][i].x-ymiddle[k-1]);
                        dist2=abs(xclassPoints[j][i].x-ymiddle[k]);
                        if(dist1<=dist2)
                        {
                            Num[k-1]++;
                            SumY[k-1]+=xclassPoints[j][i].y;
                        }
                        else
                        {
                            Num[k]++;
                            SumY[k]+=xclassPoints[j][i].y;
                        }
   //                     yclassPoints[k].push_back(xclassPoints[j][i]);
                        break;
                    }
                }
            }

        }
        for(k=0; k<y_1_Num; k++){
            if(Num[k]>0){
                ymiddle[y_2_Num]=SumY[k]/Num[k];
                y_2_Num++;
            }
        }
        //  近的类合并
//        lastMiddle=-40;
//        for(k=0; k<y_2_Num; k++){
//            if((ymiddle[k]-lastMiddle)<15){
//                lastMiddle=ymiddle[k];
//                ymiddle[k]=0;
//            }
//            else{
//                lastMiddle=ymiddle[k];
//            }
//        }

        //get a obj in this column
        memset(Num,0,sizeof(Num));
        memset(SumY,0,sizeof(SumY));
        memset(SumX,0,sizeof(SumX));
        for(i = 0;i < xclassPoints[j].size();i++)
        {
//            for(k=0; k<y_2_Num; k++){
//                if(ymiddle[k]>0){
//                    if(xclassPoints[j][i].y>=(ymiddle[k]-yyGap)
//                            && xclassPoints[j][i].y<(ymiddle[k]+yyGap)){
//                        Num[k]++;
//                        SumY[k]+=xclassPoints[j][i].y;
//                        SumX[k]+=xclassPoints[j][i].x;
//                      }
//                }
//            }
            if(xclassPoints[j][i].y<ymiddle[0])
            {
                Num[0]++;
                SumY[0]+=xclassPoints[j][i].y;
                SumX[0]+=xclassPoints[j][i].x;
         //       yclassPoints[0].push_back(xclassPoints[j][i]);
            }
            else if(xclassPoints[j][i].y>=ymiddle[y_2_Num-1])
            {
                Num[y_2_Num-1]++;
                SumY[y_2_Num-1]+=xclassPoints[j][i].y;
                SumX[y_2_Num-1]+=xclassPoints[j][i].x;
     //         yclassPoints[y_2_Num].push_back(xclassPoints[j][i]);
            }
            else
            {
                for(k=1; k<y_2_Num; k++){
                    if(xclassPoints[j][i].y>=ymiddle[k-1] && xclassPoints[j][i].y<ymiddle[k])
                    {
                        dist1=abs(xclassPoints[j][i].y-ymiddle[k-1]);
                        dist2=abs(xclassPoints[j][i].y-ymiddle[k]);
                        if(dist1<=dist2)
                        {
                            Num[k-1]++;
                            SumY[k-1]+=xclassPoints[j][i].y;
                            SumX[k-1]+=xclassPoints[j][i].x;
                        }
                        else
                        {
                            Num[k]++;
                            SumY[k]+=xclassPoints[j][i].y;
                            SumX[k]+=xclassPoints[j][i].x;
                        }
                        //             yclassPoints[k].push_back(xclassPoints[j][i]);
                        break;
                    }
                }
            }

        }
        //get a obj in this column
        for(k=0; k<y_2_Num; k++){
            if(Num[k]>thresHold2){
                p.x=SumX[k]/Num[k];
                p.y=SumY[k]/Num[k];
                Center.push_back(p);
                objNum++;
                std::cout<<" x="<<p.x<<" y="<<p.y;
            }
        }

    }//end of loop for y dimension classify
    std::cout<<" objNUm="<<objNum<<std::endl;

    return;
}

void FindSTLKObj(std::vector<cv::Point2f> &point , std::vector<cv::Point2f> &Center,std::vector<cv::Point2f> *classPoints,int &objNum)
{
   // int T = 10;

    int xxx=10;
    int yyy=6;
    int xGap=64;
    int yGap=60;
    //int xxGap=xGap/2;
    int xxGap=20;
    int yyGap=yGap/2;

    int round_1_Num=0;
    int round_2_Num=0;
    int round_3_Num=0;
    int xmiddle[xxx];

    int y_1_Num=0;
    int y_2_Num=0;
    int y_3_Num=0;
    int ymiddle[yyy];

    int Num[xxx];
    int SumY[yyy];
    int SumX[xxx];
    int lastMiddle;
    int thresHold1=2;
    int thresHold2=2;
    std::vector<cv::Point2f> xclassPoints[xxx],yclassPoints[yyy];
    cv::Point2f p;

    int i,j,k;
    memset(Num,0,sizeof(Num));
    memset(SumX,0,sizeof(SumX));
    objNum=0;
    Center.clear();
    for(i=0; i<MAXSTUDENTNUM; i++)
    {
        classPoints[i].clear();
    }
    for(i=0; i<xxx; i++)
    {
        xclassPoints[i].clear();
    }
    for(i=0; i<yyy; i++)
    {
        yclassPoints[i].clear();
    }

    int recleft[10],recright[10],rectop[10],recbottom[10];

   // std::cout<<"pointnum="<<point.size()<<std::endl;
    for(i = 0;i < point.size();i++)
    {
        k=(int)point[i].x/xGap;
        //   std::cout<<" x:"<<point[i].x<<" k:"<<k;
        Num[k]++;
        SumX[k]+=point[i].x;
    }
    for(k=0; k<xxx; k++){
        if(Num[k]>0)
        {
            xmiddle[round_1_Num]=SumX[k]/Num[k];
            round_1_Num++;
        }
    }
   // std::cout<<"  round_1_Num:"<<round_3_Num<<std::endl;
    memset(Num,0,sizeof(Num));
    memset(SumX,0,sizeof(SumX));
    for(i = 0;i < point.size();i++)
    {
        for(k=0; k<round_1_Num; k++){
            if(point[i].x>=(xmiddle[k]-xxGap) && point[i].x<(xmiddle[k]+xxGap)){
                Num[k]++;
                SumX[k]+=point[i].x;
               // xclassPoints[k].push_back(point[i]);
            }
        }
    }
    for(k=0; k<round_1_Num; k++){
        if(Num[k]>=thresHold1){
            xmiddle[round_2_Num]=SumX[k]/Num[k];
            //     xclassPoints[round_2_Num]=xclassPoints[k];
            round_2_Num++;
        }
    }

  //  std::cout<<"  round_2_Num:"<<round_2_Num<<std::endl;

    //  近的类合并
     lastMiddle=-30;
     for(k=0; k<round_2_Num; k++){
         if((xmiddle[k]-lastMiddle)<20){
             lastMiddle=xmiddle[k];
             xmiddle[k]=0;
         }
         else{
             lastMiddle=xmiddle[k];
         }
     }
     for(k=0; k<round_2_Num; k++){
         if(xmiddle[k]>0){
             xmiddle[round_3_Num]=xmiddle[k];
             round_3_Num++;
         }
     }
     memset(Num,0,sizeof(Num));
     memset(SumX,0,sizeof(SumX));
     for(i = 0;i < point.size();i++)
     {
         for(k=0; k<round_3_Num; k++){
             if(point[i].x>=(xmiddle[k]-xxGap) && point[i].x<(xmiddle[k]+xxGap)){
                 Num[k]++;
                 SumX[k]+=point[i].x;
                 xclassPoints[k].push_back(point[i]);
              }
         }
     }
     for(k=0; k<round_3_Num; k++){
         xmiddle[k]=SumX[k]/Num[k];
         std::cout<<" xmiddle="<<xmiddle[k];
     }
  //    std::cout<<"  round_3_Num:"<<round_3_Num<<std::endl;
    //end of classify at x dimension

   //start to do y dimension classify

    for(j=0; j<round_3_Num; j++){
        memset(Num,0,sizeof(Num));
        memset(SumY,0,sizeof(SumY));
        y_1_Num=0;
        y_2_Num=0;
        y_3_Num=0;
        for(k=0; k<yyy; k++)
            yclassPoints[k].clear();

        //first round y
        for(i = 0;i < xclassPoints[j].size();i++)
        {
            k=(int)xclassPoints[j][i].y/yGap;
            Num[k]++;
            SumY[k]+=xclassPoints[j][i].y;
        }
        for(k=0; k<yyy; k++){
            if(Num[k]!=0)
            {
                ymiddle[y_1_Num]=SumY[k]/Num[k];
                y_1_Num++;
            }
        }
        //second round y
        memset(Num,0,sizeof(Num));
        memset(SumY,0,sizeof(SumY));
        for(i = 0;i < xclassPoints[j].size();i++)
        {
            for(k=0; k<y_1_Num; k++){
                if(xclassPoints[j][i].y>=(ymiddle[k]-yyGap)
                        && xclassPoints[j][i].y<(ymiddle[k]+yyGap)){
                    Num[k]++;
                    SumY[k]+=xclassPoints[j][i].y;
          //          yclassPoints[k].push_back(xclassPoints[j][i]);
                }
            }
        }
        for(k=0; k<y_1_Num; k++){
            if(Num[k]>=thresHold2){
                ymiddle[y_2_Num]=SumY[k]/Num[k];
                y_2_Num++;
            }
        }
        //  近的类合并
        lastMiddle=-40;
        for(k=0; k<y_2_Num; k++){
            if((ymiddle[k]-lastMiddle)<15){
                lastMiddle=ymiddle[k];
                ymiddle[k]=0;
            }
            else{
                lastMiddle=ymiddle[k];
            }
        }

        //get a obj in this column
        memset(Num,0,sizeof(Num));
        memset(SumY,0,sizeof(SumY));
        memset(SumX,0,sizeof(SumX));

        for(i = 0;i < xclassPoints[j].size();i++)
        {
            for(k=0; k<y_2_Num; k++){
                if(ymiddle[k]>0){
                    if(xclassPoints[j][i].y>=(ymiddle[k]-yyGap)
                            && xclassPoints[j][i].y<(ymiddle[k]+yyGap)){
                        Num[k]++;
                        SumY[k]+=xclassPoints[j][i].y;
                        SumX[k]+=xclassPoints[j][i].x;
                        classPoints[k].push_back(xclassPoints[j][i]);
                      }
                }
            }
        }
        for(k=0; k<y_2_Num; k++){
            if(Num[k]>0){
                p.x=SumX[k]/Num[k];
                p.y=SumY[k]/Num[k];
                Center.push_back(p);
                objNum++;
            }
        }

    }//end of loop for y dimension classify

    return;
}

bool isMatched(cv::Rect &rect1,cv::Rect &rect2)
{
    cv::Rect rect=rect1 & rect2;
    int minarea;
    minarea=rect1.area();
    if(minarea>rect2.area())
        minarea=rect2.area();
    return (rect.area()> minarea/3);
}

bool ssimRects(cv::Rect &r1,cv::Rect &r2,double eps)
{
    double delta = eps*(std::min(r1.width, r2.width) + std::min(r1.height, r2.height))*0.5;
    return std::abs(r1.x - r2.x) <= delta &&
            std::abs(r1.y - r2.y) <= delta &&
            std::abs(r1.x + r1.width - r2.x - r2.width) <= delta &&
            std::abs(r1.y + r1.height - r2.y - r2.height) <= delta;
}



int part(std::vector<cv::Rect>& rects, std::vector<int>& labels,double eps)
{
    int i, j, N = (int)rects.size();
   // const cv::Rect* vec = &rects[0];

    int PARENT=0;
    int RANK=1;

    std::vector<int> _nodes(N*2);
    int (*nodes)[2] = (int(*)[2])&_nodes[0];

    // The first O(N) pass: create N single-vertex trees
    for(i = 0; i < N; i++)
    {
        nodes[i][PARENT]=-1;
        nodes[i][RANK] = 0;
    }

    // The main O(N^2) pass: merge connected components
    for( i = 0; i < N; i++ )
    {
        int root = i;

        // find root
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        std::cout<<" i="<<i<<"parrent,rank "<<nodes[i][PARENT]<<" "<<nodes[i][RANK]
                   <<" root="<<root;

        for( j = 0; j < N; j++ )
        {
            if( i == j || !ssimRects(rects[i], rects[j],eps))
                continue;
            int root2 = j;

            while( nodes[root2][PARENT] >= 0 )
                root2 = nodes[root2][PARENT];

            if( root2 != root )
            {
                // unite both trees
                int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
                if( rank > rank2 )
                    nodes[root2][PARENT] = root;
                else
                {
                    nodes[root][PARENT] = root2;
                    nodes[root2][RANK] += rank == rank2;
                    root = root2;
                }
                CV_Assert( nodes[root][PARENT] < 0 );

                int k = j, parent;

                // compress the path from node2 to root
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }

                // compress the path from node to root
                k = i;
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }
            }

        }
        for(j=0; j<N; j++)
        {
        std::cout<<" j="<<j<<", "<<nodes[j][PARENT]<<" "<<nodes[j][RANK]
                   <<" root="<<root;
        }
        std::cout<<std::endl;
    }

    // Final O(N) pass: enumerate classes
    labels.resize(N);
    int nclasses = 0;

    for( i = 0; i < N; i++ )
    {
        int root = i;
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        // re-use the rank as the class label
        if( nodes[root][RANK] >= 0 )
            nodes[root][RANK] = ~nclasses++;
        labels[i] = ~nodes[root][RANK];
    }

    return nclasses;
}

inline int dist2(cv::Point2i &a, cv::Point2i &b)
{
    int x = a.x - b.x;
    int y = a.y - b.y;
    return x*x + y*y;
}

inline bool ssimPoints(cv::Point2i &a,cv::Point2i &b,int eps)
{
    int x = a.x - b.x;
    int y = a.y - b.y;
    x=x*x + y*y;
    return (x<eps);
}

int partPoints(std::vector<cv::Point2i>& points, std::vector<int>& labels,int eps)
{
    int i, j, N = (int)points.size();
   // const cv::Rect* vec = &rects[0];

    int PARENT=0;
    int RANK=1;

    std::vector<int> _nodes(N*2);
    int (*nodes)[2] = (int(*)[2])&_nodes[0];

    // The first O(N) pass: create N single-vertex trees
    for(i = 0; i < N; i++)
    {
        nodes[i][PARENT]=-1;
        nodes[i][RANK] = 0;
    }

    // The main O(N^2) pass: merge connected components
    for( i = 0; i < N; i++ )
    {
        int root = i;

        // find root
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        std::cout<<" i="<<i<<"parrent,rank "<<nodes[i][PARENT]<<" "<<nodes[i][RANK]
                   <<" root="<<root;

        for( j = 0; j < N; j++ )
        {
            if( i == j || !ssimPoints(points[i], points[j],eps))
                continue;
            int root2 = j;

            while( nodes[root2][PARENT] >= 0 )
                root2 = nodes[root2][PARENT];

            if( root2 != root )
            {
                // unite both trees
                int rank = nodes[root][RANK], rank2 = nodes[root2][RANK];
                if( rank > rank2 )
                    nodes[root2][PARENT] = root;
                else
                {
                    nodes[root][PARENT] = root2;
                    nodes[root2][RANK] += rank == rank2;
                    root = root2;
                }
                CV_Assert( nodes[root][PARENT] < 0 );

                int k = j, parent;

                // compress the path from node2 to root
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }

                // compress the path from node to root
                k = i;
                while( (parent = nodes[k][PARENT]) >= 0 )
                {
                    nodes[k][PARENT] = root;
                    k = parent;
                }
            }

        }
        for(j=0; j<N; j++)
        {
        std::cout<<" j="<<j<<", "<<nodes[j][PARENT]<<" "<<nodes[j][RANK]
                   <<" root="<<root;
        }
        std::cout<<std::endl;
    }

    // Final O(N) pass: enumerate classes
    labels.resize(N);
    int nclasses = 0;

    for( i = 0; i < N; i++ )
    {
        int root = i;
        while( nodes[root][PARENT] >= 0 )
            root = nodes[root][PARENT];
        // re-use the rank as the class label
        if( nodes[root][RANK] >= 0 )
            nodes[root][RANK] = ~nclasses++;
        labels[i] = ~nodes[root][RANK];
    }

    return nclasses;
}

void groupPoints(std::vector<cv::Point2i>& pointList, int groupThreshold, int eps)
{
    int i,nlabels;
    if( groupThreshold <= 0 || pointList.empty() )
    {
       return;
    }
    std::vector<int> labels;
    int nclass = partPoints(pointList, labels, eps);
    std::vector<cv::Point2i> rpoints(nclass);
    std::vector<int> rweights(nclass, 0);
    nlabels = (int)labels.size();

    for( i = 0; i < nlabels; i++ )
    {
        int cls = labels[i];
        rpoints[cls].x += pointList[i].x;
        rpoints[cls].y += pointList[i].y;
        rweights[cls]++;
    }
    pointList.resize(nclass);
    for( i = 0; i < nclass; i++ )
    {
        cv::Point2i p = rpoints[i];
        float s = 1.f/rweights[i];
        pointList[i].x = p.x*s;
        pointList[i].y= p.y*s;

    }
}

