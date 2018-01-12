#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <iostream>
#include <string.h>
#include "sys/time.h"

#include "studentapp.h"



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

    float stepxmove=0;
    float stepymove=0;
    float stepsumXmove=0;
    float stepsumYmove=0;

    int max_count=8000; // maximum number of features to detect
    double qlevel=0.005; // quality level for feature detection
    double minDist=0.0; // min distance between two points
    std::vector<cv::Point2f> features; // detected features

    Point2f p;
    Point2f s1Start(0,355);
    Point2f s1End(5,359);
    Point2f s2Start(0,327);
    Point2f s2End(639,196);
    Point2f s3Start(0,10);
    Point2f s3End(639,136);
    Point2f s4Start(0,165);
    Point2f s4End(300,0);
//        Point2f s1Start(0,355);
//        Point2f s1End(5,359);
//        Point2f s2Start(60,359);
//        Point2f s2End(639,196);
//        Point2f s3Start(0,10);
//        Point2f s3End(639,176);
//        Point2f s4Start(0,133);
//        Point2f s4End(608,0);

//        Point2f s1Start(0,355);
//        Point2f s1End(5,359);
//        Point2f s2Start(600,359);
//        Point2f s2End(639,296);
//        Point2f s3Start(10,0);
//        Point2f s3End(639,130);
//        Point2f s4Start(0,120);
//        Point2f s4End(308,0);
    int startX1,startX4,endX2,endX3;


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
    std::vector<cv::Point2f> objCenter;
   // std::vector<cv::Point2f> classPoints[MAXSTUDENTNUM];

    std::vector<uchar> status; // status of tracked features
    std::vector<float> err; // error in tracking

    student_Feature_t student[MAXSTUDENTNUM];


    float seatFactor[16]={1.2,1.1,1.1,1.0,0.9,0.75,0.6,0.6,0.5,0.3,0.3,0.3,0.3,0.3,0.3,0.3};
    float factor;


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

        SetRawPoints(rawPoints,width,height,start,end);

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
            medianBlur(frame, frame, 3);

            frame.copyTo(outputFrame);

            // for first image of the sequence
            if(frame_prev.empty())
                frame.copyTo(frame_prev);

//            cv::goodFeaturesToTrack(frame, // the image
//            features, // the output detected features
//            max_count, // the maximum number of features
//            qlevel, // quality level
//            minDist); // min distance between two features
//            SetSTRawPoints(rawPoints,features,width,height,start,end);
            //draw raw points
            for(i=0; i<rawPoints.size(); i++)
            {
                cv::circle(inputFrame, rawPoints[i], 1,cv::Scalar(255,255,255),-1);
            }

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
       //     FindSTLKObj(lkPoints , objCenter,classPoints,rectsNum);
            std::cout<<"rectsNum="<<rectsNum<<std::endl;
            rects.clear();
            for(i=0; i<rectsNum; i++){
                rectemp.x=objCenter[i].x-20;
                if(rectemp.x<0)
                    rectemp.x=0;
                rectemp.y=objCenter[i].y-30;
                if(rectemp.y<0)
                    rectemp.y=0;
                rectemp.width=40;
                if((rectemp.x+rectemp.width)>width)
                    rectemp.width=width-rectemp.x;
                rectemp.height=60;
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
                        if(isMatched(student[k].rect,rects[i])){
                            matched[i]=1;
                            matchedNum++;
                            cout<<" obj matched k="<<k<<" rect="<<i<<endl;
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
                                index=objCenter[k].y+(320-objCenter[k].x)*0.17;
                                student[i].standupThresHold = (27+0.24*(index-180))/2;
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
                                    fabs(ymove)>0.2 && fabs(xmove)<student[k].lrThreshold){
                                //  keep this point in vector
                                student[k].initial[pointNum] = student[k].initial[i];
                                student[k].points[1][pointNum++] = student[k].points[1][i];
                                sumXmove += xmove;
                                sumYmove += ymove;
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
                        student[k].moveX=sumXmove/pointNum;
                        student[k].moveY=sumYmove/pointNum;
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
                        if(student[k].moveY>student[k].standupThresHold)  //this student is standing up
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
