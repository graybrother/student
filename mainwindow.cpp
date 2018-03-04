#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <iostream>
#include <string.h>
#include "sys/time.h"

#include "studentapp.h"


#define RAWPOINTS 0
#define STPOINTS 1
#define CANNYPOINTS 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_student_pushButton_clicked()
{
    using namespace std;
    using namespace cv;

    int left=0;
    int top=0;
    int width=640;
    int height=360;
    double res_ratio=(double)height/360;
 //   int  CENTERx=width/2;
 //   int CENTERy=height/2;
    Rect processRange(left,top,width,height);

 //   bool haveScreenRange=true;
    int screenLeft=220-left;
    int screenTop=100-top;
    int screenWidth=125;
    int screenHeight=78;
    Rect screenRange(screenLeft,screenTop,screenWidth,screenHeight);


    int frameNumber = 1; /* The current frame number */
    Mat inputFrame;
    Mat colorFrame;
    Mat frame;                  /* Current gray frame. */
    Mat frame_prev; // previous gray-level image
    Mat outputFrame;
    Mat cannyFrame;

    int keyboard = 0;           /* Input from keyboard. Used to stop the program. Enter 'q' to quit. */
    int i,j,k,index;

    std::vector<Rect> rects;
    cv::Rect rectemp;
    int rectsNum=0;
    int matched[MAXRECTNUM];
    int matchedNum=0;

    TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 15, 0.01);
    Size winSize(7,7);


    int pointNum=0;
    float xmove=0;
    float ymove=0;
    float sumXmove=0;
    float sumYmove=0;
    int sumNumber=0;

    float stepxmove=0;
    float stepymove=0;
    float stepsumXmove=0;
    float stepsumYmove=0;

    int max_count=8000; // maximum number of features to detect
    double qlevel=0.005; // quality level for feature detection
    double minDist=1.0; // min distance between two points
    std::vector<cv::Point2f> features; // detected features

    Point2f p;
//    Point2f s1Start(0,355);
//    Point2f s1End(5,359);
//    Point2f s2Start(0,327);
//    Point2f s2End(639,196);
//    Point2f s3Start(0,10);
//    Point2f s3End(639,136);
//    Point2f s4Start(0,165);
//    Point2f s4End(300,0);
        Point2f s1Start(0,355);
        Point2f s1End(5,359);
        Point2f s2Start(60,359);
        Point2f s2End(639,196);
        Point2f s3Start(0,10);
        Point2f s3End(639,176);
        Point2f s4Start(0,133);
        Point2f s4End(608,0);

//        Point2f s1Start(0,530);
//        Point2f s1End(5,height-1);
//        Point2f s2Start(0,437);
//        Point2f s2End(width-1,287);
//        Point2f s3Start(20,0);
//        Point2f s3End(width-1,200);
//        Point2f s4Start(0,287);
//        Point2f s4End(300,0);
    int startX1,startX4,endX2,endX3;

    int start_y=100;
//add 3.1
    double D;
    const double pi=acos(-1.0);
    double alpha=20.1*pi/180;
    double H=2.33;
    double H1=H-0.9;
    double H2=H1-0.40;
    double L1=H1/tan(alpha);
    double L2=H2/tan(alpha);
    double y_temp;

    double f_k=-0.23;
    double f_b=81.4*res_ratio;
 //   f_b=f_b*rate;
    int f_sen=6;

//add 3.2
    static double stable_rate[9] = {1.2/1.7, 1.3/1.7, 1.35/1.7, 1.4/1.7, 1.45/1.7, 1.5/1.7, 1.6/1.7,  1,  1.8/1.7};
    static int 	  thr_stable[9]  = {10,      15,      20,       25,      30,       35,      40,       45, 50};
    int rectemp_H,rectemp_W;

    int start[height];
    int end[height];
    //caclulate this lookup table;
    float s1Alpha=(s1End.x-s1Start.x)/(s1End.y-s1Start.y);
    float s2Alpha=(s2End.x-s2Start.x)/(s2Start.y-s2End.y);
    float s3Alpha=(s3End.x-s3Start.x)/(s3End.y-s3Start.y);
    float s4Alpha=(s4End.x-s4Start.x)/(s4Start.y-s4End.y);

    for(j=0; j<height; j++)
    {
        //get the start x of lookup table
        //s1
        if(j<=s1Start.y)
            startX1=0;
        else
        {
            if(j< s1End.y)
            {
                startX1=s1Start.x+(j-s1Start.y)*s1Alpha;
            }
            else
                startX1=width;
        }
        //s4
        if(j<=s4End.y )
            startX4=width;
        else
        {
            if(j<s4Start.y)
                startX4=s4Start.x+(s4Start.y-j)*s4Alpha;
            else
                startX4=0;
        }

        if(startX1>=startX4)
            start[j]=startX1;
        else
            start[j]=startX4;
        //get the end x of looktable
        //s2
        if(j<=s2End.y)
            endX2=width;
        else
        {
            if(j<s2Start.y)
            {
                endX2=s2Start.x+(s2Start.y-j)*s2Alpha;
            }
            else
                endX2=0;
        }
        //s3
        if(j<=s3Start.y)
            endX3=0;
        else
        {
            if(j<s3End.y)
            {
                endX3=s3Start.x+(j-s3Start.y)*s3Alpha;
            }
            else
                endX3=width;
        }

        if(endX2<=endX3)
            end[j]=endX2;
        else
            end[j]=endX3;

    }//end of lookup table

    std::vector<cv::Point2f> rawPoints; // 粗光流
    std::vector<cv::Point2f> lkPoints;
    std::vector<cv::Point2i> objCenter;
   // std::vector<cv::Point2f> classPoints[MAXSTUDENTNUM];

    std::vector<uchar> status; // status of tracked features
    std::vector<float> err; // error in tracking

    student_Feature_t student[MAXSTUDENTNUM];


 //   float seatFactor[16]={1.2,1.1,1.1,1.0,0.9,0.75,0.6,0.6,0.5,0.3,0.3,0.3,0.3,0.3,0.3,0.3};
 //   float factor;


    int objNum=     0;
    int currentID=  -1;

    int  outputTimeout=0;


    //-----------------------测试时间
    struct timeval tsBegin,tsEnd;
    long runtimes;

    for(i=0; i< MAXSTUDENTNUM; i++) {
        Student_Feature_Init(student[i]);
    }


    QString  ipaddress;

    if (filename.isEmpty())
    {
        filename=ui->ip_lineEdit->text();
        if(filename.isEmpty())
        {
            ui->warning_lineEdit->setText("Filename is empty,please open a file or input URL");
            return;
        }
    }
    //  filename="rtsp://192.168.100.182/2";

    VideoCapture capture(filename.toStdString());
    //   VideoCapture capture("rtsp://192.168.100.182/2");
    if(!capture.isOpened()){
        ui->warning_lineEdit->setText("File open error,please open video file first");
        return ;
    }
        namedWindow("Frame");
        namedWindow("Gray");

        namedWindow("Tracking");

#if RAWPOINTS
        SetRawPoints(rawPoints,width,height,start,end);
#endif
        while ((char)keyboard != 'q' && (char)keyboard != 27) {
            /* Read the current frame. */
            if (!capture.read(inputFrame)) {
                ui->warning_lineEdit->setText("end of File ");
                capture.release();

                destroyAllWindows();
                return;
            }

            // wait and reset frameNumber
            if (frameNumber < 10){
                ++frameNumber;
                continue;
            }
            if(frameNumber>10000000)
                frameNumber=20;

            //get a ROI mat
            //      Mat roiframe= inputFrame(processRange);
            inputFrame.copyTo(colorFrame);

            //convert to gray
            cvtColor(colorFrame, frame, CV_BGR2GRAY);
            //media blur
           // medianBlur(frame, frame, 3);
            blur(frame,frame,Size(3,3));

            frame.copyTo(outputFrame);

            // for first image of the sequence
            if(frame_prev.empty())
                frame.copyTo(frame_prev);
#if STPOINTS
            cv::goodFeaturesToTrack(frame, // the image
            features, // the output detected features
            max_count, // the maximum number of features
            qlevel, // quality level
            minDist); // min distance between two features
            SetSTRawPoints(rawPoints,features,start_y,start,end);
#endif

#if CANNYPOINTS
           cv::Canny(frame,cannyFrame,100,250);
           SetCannyRawPoints(rawPoints,cannyFrame,width,height,start,end);
#endif
            //draw raw points
//            for(i=0; i<rawPoints.size(); i++)
//            {
//                cv::circle(inputFrame, rawPoints[i], 1,cv::Scalar(255,255,255),-1);
//            }
            std::cout<<" raw points number="<<rawPoints.size()<<std::endl;
             //draw factor line
        //    for(i=0; i<height; i+=20)
        //    {
        //        cv::line(inputFrame,Point2f(0,(height-i)),Point2f(639,(height-i)),cv::Scalar(255,0,0));
        //    }
            //draw shield line
           // std::cout<<s1Alpha<<" "<<s2Alpha<<" "<<s3Alpha<<" "<<s4Alpha<<std::endl;
            cv::line(outputFrame,s1Start,s1End,cv::Scalar(255,255,255));
            cv::line(outputFrame,s2Start,s2End,cv::Scalar(255,255,255));
            cv::line(outputFrame,s3Start,s3End,cv::Scalar(255,255,255));
            cv::line(outputFrame,s4Start,s4End,cv::Scalar(255,255,255));

       //add 3.2
            D=6;
            y_temp=(L1-D)*sin(alpha);
            cout<<"..............."<<y_temp<<" "<<L1<<" "<<D<<endl;
            y_temp=y_temp/sqrt(H1*H1+D*D-y_temp*y_temp);
            y_temp=y_temp*3600/2.75;
            y_temp=height/2+y_temp/3;
            cout<<"..............."<<y_temp<<endl;
            cv::line(outputFrame,Point2f(0,y_temp),Point2f(width-1,y_temp),cv::Scalar(255,255,255));

            y_temp=(L2-D)*sin(alpha);
            y_temp=y_temp/sqrt(H2*H2+D*D-y_temp*y_temp);
            y_temp=y_temp*3600/2.75;
            y_temp=height/2+y_temp/3;
            cv::line(outputFrame,Point2f(0,y_temp),Point2f(width-1,y_temp),cv::Scalar(255,255,255));
            cout<<"..............."<<y_temp<<endl;

            //      for(j=0; j<height; j++)
            //      {
            //          if(start[j]<end[j])
            //              cv::line(colorFrame,Point2f(start[j],j),Point2f(end[j],j),Scalar(0,0,255));
            //      }
            //show the current frame and shield areas
            //  imshow("Frame", colorFrame);
            //test the time
            gettimeofday(&tsBegin, NULL);

            cout<<"************************"<<endl;


            cv::calcOpticalFlowPyrLK(
                        frame_prev, frame, // 2 consecutive images
                        rawPoints, // input point positions in first image
                        lkPoints, // output point positions in the 2nd image
                        status, // tracking success
                        err,
                        winSize,1,termcrit,0,0.01); // tracking error

            gettimeofday(&tsEnd, NULL);//-----------------------测试时间
            runtimes=1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+tsEnd.tv_usec-tsBegin.tv_usec;
            cout<<"raw lk time: "<<runtimes<<endl;

            pointNum=0;
            xmove=0;
            ymove=0;

            for(i= 0; i < lkPoints.size(); i++ ) {
                //  do we keep this point?
                if(status[i]){
                    xmove=rawPoints[i].x-lkPoints[i].x;
                    ymove=rawPoints[i].y-lkPoints[i].y;
                    //if points moved upwards,keep this point in vector
                    if (ymove>RAWYMOVE){
                        lkPoints[pointNum++] = lkPoints[i];
                        cv::circle(outputFrame, lkPoints[i], 1,cv::Scalar(255,255,255),-1);
                    }
                }
            }
            cout<<"LK num="<<pointNum<<endl;
            lkPoints.resize(pointNum);
            objCenter.clear();
            FindLKObj(lkPoints,objCenter,rectsNum);
            std::cout<<"rectsNum="<<rectsNum<<std::endl;
            for(i=0; i<objCenter.size(); i++)
            {
                cv::circle(inputFrame, objCenter[i], 6,cv::Scalar(255,255,255),-1);
            }
            groupPoints(objCenter,1,1200);
            for(i=0; i<objCenter.size(); i++)
            {
                cv::circle(inputFrame, objCenter[i], 3,cv::Scalar(0,0,255),-1);
            }
       //     FindSTLKObj(lkPoints , objCenter,classPoints,rectsNum);
            rectsNum=objCenter.size();
            std::cout<<"part-rectsNum="<<rectsNum<<std::endl;
            rects.clear();
            for(i=0; i<rectsNum; i++){
               // if(objCenter[i].y<start_y)

                rectemp_H = f_b + f_k * (height - (double)objCenter[i].y);
             //   std::cout<<" f_b="<<f_b<<" rate="<<res_ratio<<" f_k="<<f_k<<" H="<<rectemp_H<<endl;
                //转成40cm
                //rectemp_H = rectemp_H * 4 /(temp_sensitivity + 1);
                //rectemp_H = rectemp_H * 40 / thr_stable[temp_sensitivity + 4];
                rectemp_H = rectemp_H * stable_rate[f_sen] * 1.5;

                if(rectemp_H < 35){
                    rectemp_H = 35;
                }

                rectemp_W = rectemp_H/1.2;

                rectemp.x=objCenter[i].x-rectemp_W/2;
                if(rectemp.x<0)
                    rectemp.x=0;
                rectemp.y=objCenter[i].y-rectemp_H/2;
                if(rectemp.y<start_y)  //modify 3.1
                    rectemp.y=start_y;
                rectemp.width=rectemp_W;
                if((rectemp.x+rectemp.width)>width)
                    rectemp.width=width-rectemp.x;
                rectemp.height=rectemp_H;
                if((rectemp.y+rectemp.height)>height)
                    rectemp.height=height-rectemp.y;
                rects.push_back(rectemp);

                std::cout<<" x="<<objCenter[i].x<<" y="<<objCenter[i].y;
                cv::rectangle(outputFrame, rects[i], cv::Scalar(255), 2);
            }

           if(rectsNum>MAXRECTNUM)
                rectsNum=MAXRECTNUM;

            matchedNum=0;
            for(i=0;i<MAXRECTNUM; i++)
            {
                matched[i]=0;
            }


            //match already matched Student to rects
            for(k=0; k<MAXSTUDENTNUM; k++)
            {
                if(student[k].trustedValue>-1)
                {
                    for(i=0; i<rectsNum; i++)
                    {
                        if(matched[i]==0)
                        {
                            if(isMatched(student[k].rect,rects[i])){
                                matched[i]=1;
                                matchedNum++;
                                cout<<" obj matched k="<<k<<" rect="<<i<<endl;
                            }
                        }
                    }
                 }
            }

            //if there are new rects that not matched to a student , added to student
            if(matchedNum<rectsNum)  {
                for(k=0; k<rectsNum; k++){
                    //not matched rect
                    if(matched[k]==0){
                        for(i=0; i<MAXSTUDENTNUM; i++){
                            if(student[i].trustedValue== -1)    //already mateched obj,continue
                            {
                              //  student[i].seatPosition=(height-objCenter[k].y)/20;
                              //  factor=seatFactor[student[i].seatPosition];
                              //  student[i].standupThresHold=factor* STANDUPTHRESHOLD;
                               // index=objCenter[k].y+(320-objCenter[k].x)*0.17;
                                index=height-objCenter[k].y;
                                student[i].standupThresHold = (f_b+f_k*index)/40*thr_stable[f_sen]*2/3;
                                student[i].lrThreshold = student[i].standupThresHold*1.8;
                                student[i].rect=rects[k];
                                student[i].initialRect=rects[k];
                                student[i].trustedValue =STANDUPTIME;  //a new added obj needed to confirm
//                                for(j=0; j<classPoints[k].size();j++)
//                                {
//                                    student[k].points[0].push_back(classPoints[k][j]);
//                                    student[k].initial.push_back(classPoints[k][j]);
//                                }
//                                student[k].trackedPointNum=classPoints[k].size();
                                matchedNum++;
                                cout<<"new obj added i="<<i<<"  rect="<<k<<endl;
                                break;
                            }
                        }
                    }
                }
            }
            cout<<" matched number="<<matchedNum<<endl;
            //begin to do lk tracking
            // 1. if new added obj, lk points must be added

            for(k=0; k<MAXSTUDENTNUM; k++)
            {
                if( student[k].trustedValue==STANDUPTIME)
                {
                    SetPoints(student[k].points[0],student[k].initial,student[k].rect);
                    student[k].trackedPointNum=200;
                }
            }

            // 2. do lk track for not confirmed student
            for(k=0; k<MAXSTUDENTNUM; k++)
            {
                if(student[k].trustedValue>-1 && !student[k].isStandup
                        && student[k].trackedPointNum>=LKPOINTS) //need to tracking
                {
                    cv::rectangle(colorFrame, student[k].rect, cv::Scalar(255,0,0), 2);

                    cv::calcOpticalFlowPyrLK(
                                frame_prev, frame, // 2 consecutive images
                                student[k].points[0], // input point positions in first image
                            student[k].points[1], // output point positions in the 2nd image
                            status, // tracking success
                            err); // tracking error
                    //  a.loop over the tracked points to reject some
                    pointNum=0;
                    xmove=0;
                    ymove=0;
                    sumXmove=0;
                    sumYmove=0;
                    sumNumber=0;  //add 3.1
//                    stepxmove=0;
//                    stepymove=0;
//                    stepsumXmove=0;
//                    stepsumYmove=0;

                    for( int i= 0; i < student[k].points[1].size(); i++ ) {
                        //  do we keep this point?
                        if(status[i]){
                            // xmove=student[k].points[0][i].x-student[k].points[1][i].x;
                            //  ymove=student[k].points[0][i].y-student[k].points[1][i].y;

                            xmove=student[k].initial[i].x-student[k].points[1][i].x;
                            ymove=student[k].initial[i].y-student[k].points[1][i].y;
//                            stepxmove=student[k].points[1][i].x-student[k].points[0][i].x;
//                            stepymove=student[k].points[1][i].y-student[k].points[0][i].y;
                            if (// if point has moved
 //modifay 3.2                  fabs(ymove)>0.2 && fabs(xmove)<student[k].lrThreshold)
                                fabs(xmove)<student[k].lrThreshold)
                            {
                                //  keep this point in vector
                                student[k].initial[pointNum] = student[k].initial[i];
                                student[k].points[1][pointNum++] = student[k].points[1][i];
   //add 3.1
                                if(ymove>student[k].moveY/3)
                                {
                                    sumXmove += xmove;
                                    sumYmove += ymove;
                                    sumNumber++;
                                }

//                                stepsumXmove += stepxmove;
//                                stepsumYmove += stepymove;
                                cv::circle(colorFrame, student[k].points[1][i], 1,cv::Scalar(255,255,255),-1);
                            }

                        }
                    }
                    cout<<"lk num="<<pointNum;
                    if(pointNum<LKPOINTS)
                    {
                        Student_Feature_Init(student[k]);
                        cout<<" not enough lkpoints delete k="<<k<<endl;
                    }
                    else
                    {
                        student[k].points[1].resize(pointNum);
                        student[k].initial.resize(pointNum);
                        student[k].trackedPointNum=pointNum;
     //add 3.1
                        if(sumNumber>0)
                        {
                        student[k].moveX=sumXmove/sumNumber;
                        student[k].moveY=sumYmove/sumNumber;
                        }
                        else
                        {
                            student[k].moveX=0;
                            student[k].moveY=0;
                        }
                      //  student[k].rect.x+=stepsumXmove/pointNum;
                     //   student[k].rect.y+=stepsumYmove/pointNum;
                        student[k].rect.x=student[k].initialRect.x-student[k].moveX;
                        student[k].rect.y=student[k].initialRect.y-student[k].moveY;
                        student[k].trustedValue--;

                        std::cout<<" k= "<<k
                                <<" tv="<<student[k].trustedValue<<" y="<<student[k].moveY
                               <<" x="<<student[k].moveX
                              <<" threshold="<<student[k].standupThresHold<<std::endl;
                        //  factor=seatFactor[student[k].seatPosition];
                        //  factor=0.4;
                        if(student[k].trustedValue==0 )
                        {
                            Student_Feature_Init(student[k]);
                            cout<<"up time out k="<<k<<endl;
                            continue;
                        }
       //modify 3.1
                        if(student[k].moveY>student[k].standupThresHold
                               /* ||(student[k].rect.y<(start_y-3) && student[k].moveY>student[k].standupThresHold/2)*/)  //this student is standing up
                        {
                            if(STANDUPTIME-student[k].trustedValue<=TOOFAST)
                                //standup too soon,not a legal student standup
                            {
                                Student_Feature_Init(student[k]);
                                continue;
                            }
                            else
                            {
                                student[k].isStandup=true;
                                student[k].standupTimeout=STANDUPTIMEOUT;
                                student[k].maxMoveTimeout=MAXMOVETIME;
                                student[k].maxMoveY=student[k].moveY;
                                objNum++;
                                cout<<" a student stand up k="<<k;
                            }
                        }

                        std::swap(student[k].points[1], student[k].points[0]);

                    }
                }
            }//end of not confirmed student

            // 3. do lk track for standup student
            for(k=0; k<MAXSTUDENTNUM; k++)
            {
                if(student[k].isStandup
                        && student[k].trackedPointNum>=LKPOINTS) //need to tracking
                {
                    cv::calcOpticalFlowPyrLK(
                                frame_prev, frame, // 2 consecutive images
                                student[k].points[0], // input point positions in first image
                            student[k].points[1], // output point positions in the 2nd image
                            status, // tracking success
                            err); // tracking error
                    //  a.loop over the tracked points to reject some
                    pointNum=0;
                    xmove=0;
                    ymove=0;
                    sumXmove=0;
                    sumYmove=0;
                    stepxmove=0;
                    stepymove=0;
                    stepsumXmove=0;
                    stepsumYmove=0;
                    for( int i= 0; i < student[k].points[1].size(); i++ ) {
                        //  do we keep this point?
                        if(status[i]){
                            // xmove=student[k].points[0][i].x-student[k].points[1][i].x;
                            //  ymove=student[k].points[0][i].y-student[k].points[1][i].y;
                            xmove=student[k].initial[i].x-student[k].points[1][i].x;
                            ymove=student[k].initial[i].y-student[k].points[1][i].y;
//                            stepxmove=student[k].points[1][i].x-student[k].points[0][i].x;
//                            stepymove=student[k].points[1][i].y-student[k].points[0][i].y;
                            //  keep this point in vector
                            student[k].initial[pointNum] = student[k].initial[i];
                            student[k].points[1][pointNum++] = student[k].points[1][i];
                            sumXmove += xmove;
                            sumYmove += ymove;
//                            stepsumXmove += stepxmove;
//                            stepsumYmove += stepymove;
                        }

                    }
                    if(pointNum<LKPOINTS)
                    {
                        Student_Feature_Init(student[k]);
                        objNum--;
                    }
                    else
                    {
                        student[k].points[1].resize(pointNum);
                        student[k].initial.resize(pointNum);
                        student[k].trackedPointNum=pointNum;
                        student[k].moveX=sumXmove/pointNum;
                        student[k].moveY=sumYmove/pointNum;
                      //  student[k].rect.x+=stepsumXmove/pointNum;
                      //  student[k].rect.y+=stepsumYmove/pointNum;
                        student[k].rect.x=student[k].initialRect.x-student[k].moveX;
                        student[k].rect.y=student[k].initialRect.y-student[k].moveY;
                        student[k].standupTimeout--;
                        student[k].maxMoveTimeout--;
                        if(student[k].maxMoveTimeout>0)
                        {
                            if(student[k].maxMoveY<student[k].moveY)
                                student[k].maxMoveY=student[k].moveY;
                        }

                        std::cout<<" timeout="<<student[k].standupTimeout<<" y="<<student[k].moveY
                                <<"  x="<<student[k].moveX
                               <<"  maxmove="<<student[k].maxMoveY
                              <<" pointNum="<<pointNum
                             <<" threshold="<<student[k].standupThresHold<<std::endl;
                        std::swap(student[k].points[1], student[k].points[0]);
                    }
                }
            }//end of do lk truacking for standup student

            //策略
            for(k=0; k<MAXSTUDENTNUM; k++)
            {
                //standup timeout reached,remove this standup student
                if(student[k].isStandup && student[k].standupTimeout==0)
                {
                    Student_Feature_Init(student[k]);
                    objNum--;
                    continue;
                }

                // student has sit down
                if(student[k].isStandup
                        && student[k].moveY<(student[k].maxMoveY-student[k].standupThresHold/2))
                {
                    Student_Feature_Init(student[k]);
                    objNum--;
                    continue;
                }

                //student walk upwards, remove this student;
                if(student[k].isStandup
                        && student[k].moveY>(student[k].maxMoveY+student[k].standupThresHold))
                {
                    Student_Feature_Init(student[k]);
                    objNum--;
                    continue;
                }

                //student walk left or right, remove this student;
                if(student[k].isStandup
                        && fabs(student[k].moveX)>student[k].lrThreshold)
                {
                    Student_Feature_Init(student[k]);
                    objNum--;
                    continue;
                }

            }

            for(k=0; k<MAXSTUDENTNUM; k++)
            {
                if(student[k].isStandup)
                    cv::rectangle(colorFrame, student[k].rect, cv::Scalar(0,0,255), 2);

            }
            std::cout<<"student"<<objNum<<std::endl;

            cv::imshow("Frame",inputFrame);
            cv::imshow("Gray",outputFrame);
            cv::imshow("Tracking",colorFrame);




            gettimeofday(&tsEnd, NULL);//-----------------------测试时间
            // long runtimes;
            runtimes=1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+tsEnd.tv_usec-tsBegin.tv_usec;
            cout<<"time: "<<runtimes<<endl;

            cv::swap(frame_prev, frame);
            frameNumber++;
            keyboard = waitKey();

        } //end of while

        /* Delete capture object. */
        capture.release();

        destroyAllWindows();
        return ;
}

void MainWindow::on_openvideo_pushButton_clicked()
{

    filename = QFileDialog::getOpenFileName(this, tr("select video file"), ".",
                                              tr("Vedio Files(*.mp4 *.avi *.mkv *.yuv)"));

    if (filename.isEmpty()) {
        ui->warning_lineEdit->setText("file open does not success !!!");
        return;
    }
    ui->file_lineEdit->setText(filename);
    ui->warning_lineEdit->clear();


     return;
}

void MainWindow::on_par_pushButton_clicked()
{
    using namespace cv;
    cv::Mat colorFrame(600,800,CV_8UC3,cv::Scalar(0,0,0));
    std::vector<cv::Rect> rects;
    std::vector<int> labels;
    cv::Rect temprect;
    int N=5;
    double eps=0.7;
    int i,j,nlabels,step=100;
    cv::namedWindow("colorFrame");
    rects.resize(0);
    for(i=0,j=20; i<N; i++)
    {
        temprect.x=j;
        temprect.y=j;
        temprect.width=50;
        temprect.height=50;
        rects.push_back(temprect);
        j+=step;
    }
    rects[2].x=rects[1].x+30;
    rects[2].y=rects[1].y-10;
    rects[3].x=rects[0].x+23;
    rects[3].y=rects[0].x+23;
    for(i=0; i<N; i++)
    {
    cv::rectangle(colorFrame,rects[i],cv::Scalar(255,255,255),3);
    }
    int nclass=part(rects,labels,eps);
    std::cout<<" nclass="<<nclass<<std::endl;

    std::vector<cv::Rect> rrects(nclass);
    std::vector<int> rweights(nclass, 0);
    nlabels = (int)labels.size();

    for( i = 0; i < nlabels; i++ )
    {
        int cls = labels[i];
        rrects[cls].x += rects[i].x;
        rrects[cls].y += rects[i].y;
        rrects[cls].width += rects[i].width;
        rrects[cls].height += rects[i].height;
        rweights[cls]++;
    }
    for( i = 0; i < nclass; i++ )
    {
        cv::Rect r = rrects[i];
        float s = 1.f/rweights[i];
        rrects[i] = Rect(saturate_cast<int>(r.x*s),
             saturate_cast<int>(r.y*s),
             saturate_cast<int>(r.width*s),
             saturate_cast<int>(r.height*s));
        cv::rectangle(colorFrame,rrects[i],cv::Scalar(0,0,255),1);
    }

    cv::imshow("colorFrame",colorFrame);
    cv::waitKey();
    destroyAllWindows();

    return;
}

