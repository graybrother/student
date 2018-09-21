#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <iostream>
#include <string.h>
#include "sys/time.h"

#include "studentapp.h"
#include "ysqface.h"

#include "highgui.hpp"

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
    double res_ratio=(double)height/360.;
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
    double xmove=0;
    double ymove=0;
    double sumXmove=0;
    double sumYmove=0;
    int sumNumber=0;

    double stepxmove=0;
    double stepymove=0;
    //float stepsumXmove=0;
    //float stepsumYmove=0;

    int max_count=8000; // maximum number of features to detect
    double qlevel=0.005; // quality level for feature detection
    double minDist=1.0; // min distance between two points
    std::vector<cv::Point2f> features; // detected features


    //add 3.1

    const double pi=acos(-1.0);
    double D;
    double angle=17.4;    //20.4;
    double alpha=angle*pi/180;
    double H=1.9;     //2.2;
    double H1=H-0.9;
    double H2=H1-0.25;
    double L1=H1/tan(alpha);
    double L2=H2/tan(alpha);

    int f_sen=6;   //control panel sensitivity
    int f_sen_track=f_sen;
    // f_sen=8-f_sen;  //used for lookup table


#define STABLE						(9)
#if (STABLE == 5)
    double stable_rate[STABLE] = {0.70588, 0.79412, 0.91176, 1.0, 1.0588};
#endif
#if (STABLE == 9)
    double stable_rate[STABLE] = {1.2/1.7, 1.3/1.7, 1.35/1.7, 1.4/1.7, 1.45/1.7, 1.5/1.7, 1.6/1.7,  1,  1.8/1.7};
#endif
    int         thr_stable[9]  = {10,      15,      20,       25,      30,       35,      40,       45, 50};

    //行，1.8米到3.0米，间隔0.1；共13行。		//
    //列，5°到30°，间隔1；共26组，52列。		//
    double F_KB[13][52] = {

        {	-0.4344, 95.1652, -0.4324, 98.1325, -0.4304, 101.0827, -0.4284, 104.0175, -0.4264, 106.9390, -0.4244, 109.8489, -0.4225, 112.7492, -0.4205, 115.6415, -0.4185, 118.5277, -0.4165, 121.4095, -0.4145, 124.2887, -0.4125, 127.1670, -0.4105, 130.0462, -0.4085, 132.9280, -0.4064, 135.8141, -0.4044, 138.7063, -0.4023, 141.6063, -0.4003, 144.5160, -0.3982, 147.4372, -0.3961, 150.3717, -0.3940, 153.3213, -0.3918, 156.2880, -0.3897, 159.2737, -0.3875, 162.2803, -0.3852, 165.3101, -0.3830, 168.3649 },
        {	-0.3892, 85.3773, -0.3870, 87.9792, -0.3849, 90.5610, -0.3828, 93.1244, -0.3806, 95.6712, -0.3785, 98.2030, -0.3764, 100.7215, -0.3743, 103.2283, -0.3722, 105.7250, -0.3701, 108.2131, -0.3680, 110.6941, -0.3659, 113.1696, -0.3637, 115.6410, -0.3616, 118.1098, -0.3595, 120.5776, -0.3573, 123.0457, -0.3552, 125.5156, -0.3530, 127.9889, -0.3508, 130.4669, -0.3486, 132.9512, -0.3464, 135.4433, -0.3441, 137.9447, -0.3419, 140.4569, -0.3396, 142.9815, -0.3373, 145.5200, -0.3349, 148.0742 },
        {	-0.3522, 77.3844, -0.3499, 79.6907, -0.3477, 81.9747, -0.3455, 84.2379, -0.3432, 86.4821, -0.3410, 88.7087, -0.3388, 90.9193, -0.3366, 93.1153, -0.3344, 95.2982, -0.3322, 97.4693, -0.3300, 99.6301, -0.3278, 101.7819, -0.3256, 103.9259, -0.3234, 106.0635, -0.3212, 108.1960, -0.3189, 110.3246, -0.3167, 112.4506, -0.3145, 114.5752, -0.3122, 116.6996, -0.3099, 118.8251, -0.3076, 120.9530, -0.3053, 123.0843, -0.3030, 125.2205, -0.3007, 127.3628, -0.2983, 129.5123, -0.2959, 131.6704 },
        {	-0.3214, 70.7376, -0.3190, 72.8007, -0.3167, 74.8396, -0.3144, 76.8559, -0.3121, 78.8513, -0.3098, 80.8272, -0.3075, 82.7849, -0.3052, 84.7260, -0.3030, 86.6516, -0.3007, 88.5632, -0.2984, 90.4618, -0.2962, 92.3488, -0.2939, 94.2254, -0.2916, 96.0926, -0.2894, 97.9517, -0.2871, 99.8037, -0.2848, 101.6497, -0.2825, 103.4908, -0.2802, 105.3280, -0.2779, 107.1625, -0.2756, 108.9952, -0.2732, 110.8272, -0.2709, 112.6594, -0.2685, 114.4930, -0.2661, 116.3289, -0.2637, 118.1681 },
        {	-0.2953, 65.1261, -0.2929, 66.9859, -0.2905, 68.8202, -0.2881, 70.6306, -0.2858, 72.4185, -0.2834, 74.1854, -0.2811, 75.9326, -0.2787, 77.6614, -0.2764, 79.3730, -0.2741, 81.0687, -0.2718, 82.7497, -0.2695, 84.4170, -0.2672, 86.0718, -0.2649, 87.7150, -0.2626, 89.3478, -0.2603, 90.9711, -0.2580, 92.5859, -0.2556, 94.1932, -0.2533, 95.7938, -0.2510, 97.3886, -0.2486, 98.9787, -0.2463, 100.5647, -0.2439, 102.1477, -0.2415, 103.7284, -0.2391, 105.3077, -0.2367, 106.8863 },
        {	-0.2730, 60.3277, -0.2705, 62.0158, -0.2681, 63.6773, -0.2656, 65.3137, -0.2632, 66.9264, -0.2608, 68.5170, -0.2584, 70.0865, -0.2561, 71.6365, -0.2537, 73.1679, -0.2514, 74.6820, -0.2490, 76.1799, -0.2467, 77.6627, -0.2444, 79.1312, -0.2420, 80.5867, -0.2397, 82.0299, -0.2374, 83.4618, -0.2351, 84.8833, -0.2327, 86.2951, -0.2304, 87.6983, -0.2280, 89.0935, -0.2257, 90.4815, -0.2233, 91.8631, -0.2210, 93.2390, -0.2186, 94.6099, -0.2162, 95.9766, -0.2137, 97.3397 },
        {	-0.2536, 56.1798, -0.2511, 57.7212, -0.2486, 59.2351, -0.2461, 60.7230, -0.2437, 62.1863, -0.2413, 63.6265, -0.2389, 65.0447, -0.2365, 66.4423, -0.2341, 67.8204, -0.2317, 69.1801, -0.2294, 70.5224, -0.2270, 71.8485, -0.2247, 73.1591, -0.2223, 74.4554, -0.2200, 75.7380, -0.2176, 77.0080, -0.2153, 78.2661, -0.2130, 79.5131, -0.2106, 80.7497, -0.2083, 81.9766, -0.2059, 83.1947, -0.2036, 84.4044, -0.2012, 85.6065, -0.1988, 86.8015, -0.1965, 87.9902, -0.1940, 89.1730 },
        {	-0.2367, 52.5601, -0.2342, 53.9751, -0.2316, 55.3619, -0.2291, 56.7220, -0.2266, 58.0567, -0.2242, 59.3675, -0.2218, 60.6556, -0.2193, 61.9223, -0.2169, 63.1687, -0.2146, 64.3958, -0.2122, 65.6048, -0.2098, 66.7965, -0.2075, 67.9720, -0.2051, 69.1321, -0.2028, 70.2775, -0.2005, 71.4093, -0.1981, 72.5280, -0.1958, 73.6344, -0.1935, 74.7293, -0.1911, 75.8132, -0.1888, 76.8869, -0.1864, 77.9508, -0.1841, 79.0056, -0.1817, 80.0518, -0.1794, 81.0899, -0.1770, 82.1205 },
        {	-0.2218, 49.3753, -0.2192, 50.6807, -0.2166, 51.9571, -0.2141, 53.2062, -0.2116, 54.4294, -0.2091, 55.6280, -0.2067, 56.8033, -0.2043, 57.9566, -0.2019, 59.0890, -0.1995, 60.2015, -0.1971, 61.2951, -0.1947, 62.3709, -0.1924, 63.4297, -0.1900, 64.4723, -0.1877, 65.4996, -0.1854, 66.5124, -0.1830, 67.5113, -0.1807, 68.4970, -0.1784, 69.4702, -0.1761, 70.4315, -0.1738, 71.3815, -0.1714, 72.3207, -0.1691, 73.2497, -0.1668, 74.1688, -0.1644, 75.0787, -0.1621, 75.9797 },
        {	-0.2085, 46.5528, -0.2059, 47.7623, -0.2033, 48.9423, -0.2008, 50.0945, -0.1983, 51.2202, -0.1958, 52.3209, -0.1933, 53.3978, -0.1909, 54.4522, -0.1885, 55.4852, -0.1861, 56.4978, -0.1837, 57.4911, -0.1813, 58.4659, -0.1790, 59.4233, -0.1767, 60.3639, -0.1743, 61.2887, -0.1720, 62.1983, -0.1697, 63.0934, -0.1674, 63.9746, -0.1651, 64.8427, -0.1628, 65.6981, -0.1605, 66.5415, -0.1582, 67.3732, -0.1559, 68.1939, -0.1536, 69.0038, -0.1513, 69.8035, -0.1489, 70.5934 },
        {	-0.1967, 44.0353, -0.1940, 45.1605, -0.1914, 46.2558, -0.1889, 47.3227, -0.1864, 48.3628, -0.1839, 49.3774, -0.1814, 50.3679, -0.1790, 51.3355, -0.1765, 52.2813, -0.1741, 53.2064, -0.1718, 54.1117, -0.1694, 54.9982, -0.1671, 55.8669, -0.1648, 56.7184, -0.1624, 57.5536, -0.1601, 58.3731, -0.1578, 59.1778, -0.1556, 59.9681, -0.1533, 60.7447, -0.1510, 61.5081, -0.1487, 62.2588, -0.1464, 62.9974, -0.1442, 63.7242, -0.1419, 64.4396, -0.1396, 65.1441, -0.1373, 65.8380 },
        {	-0.1860, 41.7768, -0.1834, 42.8276, -0.1807, 43.8479, -0.1782, 44.8396, -0.1756, 45.8040, -0.1731, 46.7426, -0.1707, 47.6568, -0.1682, 48.5478, -0.1658, 49.4167, -0.1634, 50.2645, -0.1611, 51.0924, -0.1587, 51.9012, -0.1564, 52.6917, -0.1541, 53.4649, -0.1518, 54.2214, -0.1495, 54.9620, -0.1472, 55.6873, -0.1450, 56.3979, -0.1427, 57.0945, -0.1404, 57.7774, -0.1382, 58.4473, -0.1359, 59.1045, -0.1337, 59.7496, -0.1315, 60.3828, -0.1292, 61.0045, -0.1270, 61.6150 },
        {	-0.1764, 39.7403, -0.1737, 40.7250, -0.1711, 41.6788, -0.1685, 42.6036, -0.1660, 43.5008, -0.1635, 44.3720, -0.1610, 45.2185, -0.1586, 46.0415, -0.1562, 46.8422, -0.1538, 47.6217, -0.1514, 48.3809, -0.1491, 49.1209, -0.1468, 49.8424, -0.1445, 50.5463, -0.1422, 51.2334, -0.1399, 51.9043, -0.1377, 52.5596, -0.1354, 53.2001, -0.1332, 53.8262, -0.1310, 54.4385, -0.1288, 55.0373, -0.1265, 55.6233, -0.1243, 56.1966, -0.1221, 56.7578, -0.1199, 57.3071, -0.1177, 57.8449 },

    };



    //add 3.2

    int rectemp_H,rectemp_W;

    int camera_height = (int)(H*100.0);		//	获取面板上相机高度，单位cm	//


    /* ---------------------------------------------	根据摄像机安装高度和计算的下倾角 查表	---------------------- */
    int camera_height_level = (int)((camera_height + 5) / 10);		//	换算为高度对应的级别	//

    int camera_height_N;
    if(camera_height_level<18)
        camera_height_N=18;
    else if(camera_height_level>28)
        camera_height_N=30;
    else
    {

        for (i = 18; i <= 30; i++)
        {
            if (camera_height_level == i)
                camera_height_N = i-18;

        }
    }
    int f_u_level = (int)(angle + 0.5);	//	换算角度对应的级别	//

    int f_u_N;

    if(f_u_level<5)
        f_u_N=5;
    else if(f_u_level>30)
        f_u_N=30;
    else
    {
        for (i = 5; i <= 30; i++)
        {
            if (f_u_level == i)
                f_u_N = i-5;

        }
    }
    double f_k = F_KB[camera_height_N][f_u_N * 2];
    double f_b = F_KB[camera_height_N][f_u_N * 2 + 1];
    double f_k_sen = f_k / 40 * thr_stable[8-f_sen];	//	原始数据基准为40cm，再转化为不同灵敏度不同分辨率对应的身高的增量，thr_stable[f_sen+4]为新数组内的身高增量数据	//
    double f_b_sen = f_b / 40 * thr_stable[8-f_sen];	//	40cm对应面板的2度灵敏	//
    f_b_sen=f_b_sen*res_ratio;

    cout<<" height_N="<<camera_height_N<<" u_N="<<f_u_N<<" f_k="<<f_k<<" f_b="<<f_b
       <<" f_k_sen="<<f_k_sen<<" f_b_sen="<<f_b_sen<<endl;

    //add 3.2
    double y_temp;
    int y_temp1,y_temp2;
    D=11;
    y_temp=(L1-D)*sin(alpha);
    //    cout<<"..............."<<y_temp<<" "<<L1<<" "<<D<<endl;
    y_temp=y_temp/sqrt(H1*H1+D*D-y_temp*y_temp);
    y_temp=y_temp*3600/2.75;
    y_temp1=height/2+y_temp/3;

    //   start_y=y_temp1;

    y_temp=(L2-D)*sin(alpha);
    y_temp=y_temp/sqrt(H2*H2+D*D-y_temp*y_temp);
    y_temp=y_temp*3600/2.75;
    y_temp2=height/2+y_temp/3;

    Point2f p;

       Point2f s1Start(0,350);
        Point2f s1End(100,359);
        Point2f s2Start(639,196);
        Point2f s2End(600,0);
        Point2f s3Start(639,10);
        Point2f s3End(0,136);
        Point2f s4Start(165,0);
        Point2f s4End(0,100);
    //    Point2f s1Start(0,355);
    //    Point2f s1End(5,359);
    //    Point2f s2Start(60,359);
    //    Point2f s2End(639,196);
    //    Point2f s3Start(0,10);
    //    Point2f s3End(639,176);
    //    Point2f s4Start(0,133);
    //    Point2f s4End(608,0);

//            Point2f s1Start(0,530);
//            Point2f s1End(5,height-1);
//            Point2f s2Start(0,437);
//            Point2f s2End(width-1,287);
//            Point2f s3Start(20,0);
//            Point2f s3End(width-1,200);
//            Point2f s4Start(0,287);
//            Point2f s4End(300,0);
//    Point2f s1Start(0,40);
//    Point2f s1End(639,40);
//    Point2f s2Start(639,100);
//    Point2f s2End(0,100);
//    Point2f s3Start(639,60);
//    Point2f s3End(0,60);
//    Point2f s4Start(50,0);
//    Point2f s4End(100,359);


    int startX[4]={0};
    int endX[4]={0};


    int start_y=0;
    //int start_y=y_temp1;

    int start[height];
    int end[height];
    //caclulate this lookup table;
    float s1Alpha=(s1End.x-s1Start.x)/(s1End.y-s1Start.y);
    float s2Alpha=(s2End.x-s2Start.x)/(s2End.y-s2Start.y);
    float s3Alpha=(s3End.x-s3Start.x)/(s3End.y-s3Start.y);
    float s4Alpha=(s4End.x-s4Start.x)/(s4End.y-s4Start.y);

    for(j=0; j<height; j++)
    {
        //get the start x of lookup table
        //s1
        if(s1Start.y<s1End.y)
        {
            if(j<=s1Start.y)
                startX[0]=0;

            else
                startX[0]=s1Start.x+(j-s1Start.y)*s1Alpha;

            endX[0]=width;
        }
        else if(s1Start.y==s1End.y)
        {
            if(j<=s1Start.y)
                startX[0]=0;
            else
                startX[0]=width;

                endX[0]=width;
        }
        else
        {
            if(j<=s1End.y)
                endX[0]=width;
            else if(j>s1End.y && j<=s1Start.y)//
                endX[0]=s1Start.x+(j-s1Start.y)*s1Alpha;
            else
                endX[0]=0;

            startX[0]=0;
        }
        //s4
        if(j>=s4End.y )
            startX[3]=0;
        else
        {
            startX[3]=s4Start.x+(j-s4Start.y)*s4Alpha;

        }
        endX[3]=width;
        //s2
        if(s2End.y<s2Start.y)
        {
            if(j>=s2Start.y )
                endX[1]=width;
            else
            {
                endX[1]=s2Start.x+(j-s2Start.y)*s2Alpha;

            }
            startX[1]=0;
        }
        else if(s2End.y==s2Start.y)
        {
            if(j>=s2Start.y )
                endX[1]=width;
            else
                endX[1]=0;

            startX[1]=0;
        }
        else
        {
            if(j<=s2Start.y )
                startX[1]=width;
            else if(j>s2Start.y && j<=s2End.y)
                startX[1]=s2Start.x+(j-s2Start.y)*s2Alpha;
            else
                startX[1]=0;

            endX[1]=width;
        }

        //s3
        if(s3End.y<s3Start.y)
        {
            if(j>=s3Start.y )
                endX[2]=width;
            else
            {
                endX[2]=s3Start.x+(j-s3Start.y)*s3Alpha;

            }
            startX[2]=0;
        }
        else if(s3End.y==s3Start.y)
        {
            if(j>=s3Start.y )
                endX[2]=width;
            else
                endX[2]=0;

            startX[2]=0;
        }
        else
        {
            if(j<=s3Start.y )
                startX[2]=width;
            else if(j>s3Start.y && j<=s3End.y)
                startX[2]=s3Start.x+(j-s3Start.y)*s3Alpha;
            else
                startX[2]=0;

            endX[2]=width;
        }

        int tempstart=0;
        int tempend=width;
        for(i=0; i<4; i++)
        {
            if(startX[i]>tempstart)
                tempstart=startX[i];
            if(endX[i]<tempend)
                tempend=endX[i];
        }
        start[j]=tempstart;
        end[j]=tempend;


    }//end of lookup table

    std::vector<cv::Point2f> rawPoints; // 粗光流
    std::vector<cv::Point2f> lkPoints;
    std::vector<cv::Point2i> objCenter;
    // std::vector<cv::Point2f> classPoints[MAXSTUDENTNUM];

    std::vector<uchar> status; // status of tracked features
    std::vector<float> err; // error in tracking
    //add 3.10
    cv::Mat frameMat[STANDUPTIME];
    int framePtr=0;
    int tempPtr,tempPtr_1,tempPtr_2;

    cv::Mat frameDiff1[MAXSTUDENTNUM];
    cv::Mat frameDiff2[MAXSTUDENTNUM];
    cv::Mat tempDiff,tempDiff1,tempDiff2;

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
    if(!capture.isOpened())
    {
        ui->warning_lineEdit->setText("File open error,please open video file first");
        return ;
    }
    namedWindow("Frame");
    namedWindow("Gray");

    namedWindow("Tracking");
    namedWindow("FrameDiff1");
    namedWindow("FrameDiff2");
    namedWindow("FrameDiff");
    // creat a trackbar
    cv::createTrackbar( "fsen", "Tracking", &f_sen_track, 8, 0 );

#if RAWPOINTS
    SetRawPoints(rawPoints,width,height,start,end);
#endif

    int frameInterV=0;
    while ((char)keyboard != 'q' && (char)keyboard != 27)
    {
        /* Read the current frame. */
        if (!capture.read(inputFrame))
        {
            ui->warning_lineEdit->setText("end of File ");
            capture.release();

            destroyAllWindows();
            return;
        }

        // wait and reset frameNumber
        if (frameNumber < 10)
        {
            ++frameNumber;
            continue;
        }
        if(frameNumber>10000000)
            frameNumber=20;
        //add 3.7

        if(frameInterV==FRAMEINTERV)
            frameInterV=0;
        if(frameInterV>0)
        {
            frameNumber++;
            frameInterV++;
            continue;
        }
        frameInterV++;
        //get a ROI mat
        //      Mat roiframe= inputFrame(processRange);
        inputFrame.copyTo(colorFrame);

        //convert to gray
        cvtColor(colorFrame, frame, CV_BGR2GRAY);
        //media blur
        // medianBlur(frame, frame, 3);
        // blur(frame,frame,Size(3,3));

        frame.copyTo(outputFrame);
        frame.copyTo(frameMat[framePtr]);
        framePtr++;
        if(framePtr==STANDUPTIME)
            framePtr=0;

        // for first image of the sequence
        if(frame_prev.empty())
            frame.copyTo(frame_prev);
        //if f_sen_track changed
        if(f_sen!=f_sen_track)
        {
            f_sen=f_sen_track;
            f_k_sen = f_k / 40 * thr_stable[8-f_sen];
            f_b_sen = f_b / 40 * thr_stable[8-f_sen];
            f_b_sen=f_b_sen*res_ratio;

        }
        std::cout<<"f_sen="<<f_sen<<std::endl;
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

        cv::line(outputFrame,Point2f(0,y_temp1),Point2f(width-1,y_temp1),cv::Scalar(255,255,255));

        cv::line(outputFrame,Point2f(0,y_temp2),Point2f(width-1,y_temp2),cv::Scalar(255,255,255));

              for(j=0; j<height; j++)
              {
                  if(start[j]<end[j])
                      cv::line(colorFrame,Point2f(start[j],j),Point2f(end[j],j),Scalar(0,0,255));
              }
       // show the current frame and shield areas
          imshow("Frame", colorFrame);

        //test the time
        gettimeofday(&tsBegin, NULL);

        cout<<"************************"<<endl;

        if(rawPoints.size()<=0)
        {
            cout<<"zero points....."<<endl;
            waitKey(33);
            continue;
        }
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
        rectsNum=0;
        //  xmove=0;
        //  ymove=0;

        for(i= 0; i < lkPoints.size(); i++ ) {
            //  do we keep this point?
            if(status[i]){
                // xmove=rawPoints[i].x-lkPoints[i].x;
                ymove=rawPoints[i].y-lkPoints[i].y;
                xmove=rawPoints[i].x-lkPoints[i].x;
                //if points moved upwards,keep this point in vector
                if (ymove>RAWYMOVE){
                    if(fabs(ymove)<MAXLKMOVEY && fabs(xmove)<MAXLKMOVEX)
                    {
                        lkPoints[pointNum++] = lkPoints[i];
                        cv::circle(outputFrame, lkPoints[i], 1,cv::Scalar(255,255,255),-1);
                    }
                }
            }
        }
        cout<<"LK num="<<pointNum<<endl;
        //modeifed 3.7
        rectsNum=0;
        if(pointNum>XNUMTHR)
        {
            lkPoints.resize(pointNum);
            objCenter.clear();
            FindLKObj(width,height,lkPoints,objCenter,rectsNum);
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
                rectemp_H = rectemp_H * stable_rate[8-f_sen] * 1.5;

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
                                student[i].standupThresHold = (f_b_sen+f_k_sen*index)*2/3;
                                student[i].lrThreshold = student[i].standupThresHold*3;
                                student[i].rect=rects[k];
                                student[i].initialRect=rects[k];
                                student[i].trustedValue =STANDUPTIME;  //a new added obj needed to confirm
                                //add 3.10
                                student[i].startTimeout=STARTTIMEOUT;
                                student[i].framePtr=framePtr;

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
        }//end of if(pointNum>XNUMTHR

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

        // 2. do lk track for a student
        for(k=0; k<MAXSTUDENTNUM; k++)
        {
            if(student[k].trustedValue>-1 && !student[k].tobeConfirmed && !student[k].isStandup
                    && student[k].trackedPointNum>=LKPOINTS) //need to tracking
            {
                cv::rectangle(colorFrame, student[k].rect, cv::Scalar(255,255,255), 2);

                cv::calcOpticalFlowPyrLK(
                            frame_prev, frame, // 2 consecutive images
                            student[k].points[0], // input point positions in first image
                        student[k].points[1], // output point positions in the 2nd image
                        status, // tracking success
                        err,
                        winSize,1,termcrit,0,0.01); // tracking error
                //  a.loop over the tracked points to reject some
                pointNum=0;
                //  xmove=0;
                //  ymove=0;
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
                                //modifay 3.2
                                fabs(ymove)>0.2 && fabs(xmove)<student[k].lrThreshold)
                            //fabs(xmove)<student[k].lrThreshold)
                        {
                            stepxmove=student[k].points[1][i].x-student[k].points[0][i].x;
                            stepymove=student[k].points[1][i].y-student[k].points[0][i].y;
                            if(fabs(stepxmove)<=MAXLKMOVEX && fabs(stepymove)<=MAXLKMOVEY)
                            {
                                //  keep this point in vector
                                student[k].initial[pointNum] = student[k].initial[i];
                                student[k].points[1][pointNum++] = student[k].points[1][i];
                                //add 3.1
                                if(ymove>student[k].moveY/4)
                                {
                                    sumXmove += xmove;
                                    sumYmove += ymove;
                                    sumNumber++;
                                    cv::circle(colorFrame, student[k].points[1][i], 1,cv::Scalar(255,255,255),-1);

                                }

                                //cv::circle(colorFrame, student[k].points[1][i], 1,cv::Scalar(255,255,255),-1);
                            }
                        }

                    }  //end of if(status[i]
                }  // end of for(all lk points)
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
                    //                    else
                    //                    {
                    //                        student[k].moveX=0;
                    //                        student[k].moveY=0;
                    //                    }
                    //  student[k].rect.x+=stepsumXmove/pointNum;
                    //   student[k].rect.y+=stepsumYmove/pointNum;
                    student[k].rect.x=student[k].initialRect.x-student[k].moveX;
                    student[k].rect.y=student[k].initialRect.y-student[k].moveY;
                    student[k].trustedValue--;

                    std::cout<<" k= "<<k
                            <<" tv="<<student[k].trustedValue<<" y="<<student[k].moveY
                           <<" x="<<student[k].moveX
                          <<" threshold="<<student[k].standupThresHold
                         <<" sumNumber="<<sumNumber<<std::endl;
                    //  factor=seatFactor[student[k].seatPosition];
                    //  factor=0.4;
                    //add 3.9
                    student[k].startTimeout--;
                    if(student[k].startTimeout==0)
                    {
                        if(student[k].moveY<student[k].standupThresHold/4)
                        {
                            Student_Feature_Init(student[k]);
                            cout<<"starttime out k="<<k<<endl;
                            continue;
                        }
                        else
                        {
                            pointNum=0;
                            for( int i= 0; i < student[k].points[1].size(); i++ )
                            {
                                ymove=student[k].initial[i].y-student[k].points[1][i].y;
                                if(ymove>student[k].standupThresHold/4)
                                {
                                    student[k].initial[pointNum] = student[k].initial[i];
                                    student[k].points[1][pointNum++] = student[k].points[1][i];
                                }
                            }
                            if(pointNum<LKPOINTS)
                            {
                                Student_Feature_Init(student[k]);
                                cout<<"not enough lk pointsafter starttime. k="<<k<<endl;
                                continue;
                            }
                            student[k].points[1].resize(pointNum);
                            student[k].initial.resize(pointNum);
                            student[k].trackedPointNum=pointNum;
                            cout<<" able after starttime lkpoint="<<pointNum<<endl;
                        }
                    }

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
                            cout<<"stand up too soon, k="<<k<<endl;
                            continue;
                        }
                        else
                        {
                            //modified 3.10
                            // student[k].isStandup=true;
                            student[k].tobeConfirmed=true;
                            //  student[k].standupTimeout=STANDUPTIMEOUT;
                            student[k].maxMoveTimeout=MAXMOVETIME;
                            student[k].maxMoveY=student[k].moveY;
                            //  objNum++;
                            cout<<" a student tobe confirmed, k="<<k<<endl;
                        }
                    }

                    std::swap(student[k].points[1], student[k].points[0]);

                }
            }
        }//end of not confirmed student

        //add 3.10  add a tobe confirm process
        //3.do lk for tobeconfirmed student, record maxMoveY
        for(k=0; k<MAXSTUDENTNUM; k++)
        {
            if(student[k].tobeConfirmed && !student[k].isStandup
                    && student[k].trackedPointNum>=LKPOINTS_UP) //need to tracking
            {
                if(student[k].maxMoveTimeout==MAXMOVETIME)
                {
                    student[k].maxMoveTimeout--;
                    continue;
                }
                else
                {
                    cv::rectangle(colorFrame, student[k].rect, cv::Scalar(255,0,0), 2);
                    //do lk for standup student
                    cv::calcOpticalFlowPyrLK(
                                frame_prev, frame, // 2 consecutive images
                                student[k].points[0], // input point positions in first image
                            student[k].points[1], // output point positions in the 2nd image
                            status, // tracking success
                            err,
                            winSize,1,termcrit,0,0.01); // tracking error
                    //  a.loop over the tracked points to reject some
                    pointNum=0;
                    sumXmove=0;
                    sumYmove=0;
                    float lkleft=1000.;
                    float lkright=0;
                    float lktop=1000.;
                    float lkbottom=0;
                    for( int i= 0; i < student[k].points[1].size(); i++ )
                    {
                        //  do we keep this point?
                        if(status[i])
                        {

                            stepxmove=student[k].points[1][i].x-student[k].points[0][i].x;
                            stepymove=student[k].points[1][i].y-student[k].points[0][i].y;
                            if(fabs(stepxmove)<=MAXLKMOVEX && fabs(stepymove)<=MAXLKMOVEY)
                            {
                                xmove=student[k].initial[i].x-student[k].points[1][i].x;
                                ymove=student[k].initial[i].y-student[k].points[1][i].y;

                                student[k].initial[pointNum] = student[k].initial[i];
                                student[k].points[1][pointNum++] = student[k].points[1][i];
                                sumXmove += xmove;
                                sumYmove += ymove;
                                //if first time,record lk rect
                                if(student[k].maxMoveTimeout==MAXMOVETIME)
                                {
                                    if(lkleft>student[k].points[1][i].x)
                                        lkleft=student[k].points[1][i].x;
                                    if(lkright<student[k].points[1][i].x)
                                        lkright=student[k].points[1][i].x;
                                    if(lktop>student[k].points[1][i].y)
                                        lktop=student[k].points[1][i].y;
                                    if(lkbottom<student[k].points[1][i].y)
                                        lkbottom=student[k].points[1][i].y;
                                }
                                cv::circle(colorFrame, student[k].points[1][i], 1,cv::Scalar(255,0,0),-1);
                            }

                        }

                    }
                    if(pointNum<LKPOINTS_UP)
                    {
                        Student_Feature_Init(student[k]);

                    }
                    //height or width too small, remove this obj
                    else if(student[k].maxMoveTimeout==MAXMOVETIME)
                    {
                        if((lkright-lkleft)<student[k].rect.width/6.
                                || (lkbottom-lktop)<student[k].rect.height/6.)
                        {
                            Student_Feature_Init(student[k]);
                            cout<<" widht or height too small, k="<<k<<endl;
                        }
                    }
                    else
                    {
                        student[k].points[1].resize(pointNum);
                        student[k].initial.resize(pointNum);
                        student[k].trackedPointNum=pointNum;
                        student[k].moveX=sumXmove/pointNum;
                        student[k].moveY=sumYmove/pointNum;
                        student[k].rect.x=student[k].initialRect.x-student[k].moveX;
                        student[k].rect.y=student[k].initialRect.y-student[k].moveY;
                        student[k].maxMoveTimeout--;
                        if(student[k].maxMoveTimeout>0)
                        {
                            if(student[k].maxMoveY<student[k].moveY)
                                student[k].maxMoveY=student[k].moveY;
                            if(student[k].maxMoveY>student[k].standupThresHold*1.5)
                                student[k].maxMoveY=student[k].standupThresHold*1.5;
                        }

                        //check if there is a face here, set a flag
                        student[k].haveFace=true;
                        if(student[k].haveFace)
                        {
                            student[k].isStandup=true;
                            student[k].standupTimeout=STANDUPTIMEOUT;
                            objNum++;
                            cout<<" a student standup, k="<<k<<endl;
                        }
                        if(student[k].maxMoveTimeout==0 &&  !student[k].isStandup)
                        {
                            Student_Feature_Init(student[k]);
                            cout<<" timeout,this student not comfirmed, k="<<k<<endl;
                            continue;
                        }

                        std::cout<<" k="<<k
                                <<" timeout="<<student[k].standupTimeout<<" y="<<student[k].moveY
                               <<"  x="<<student[k].moveX
                              <<"  maxmove="<<student[k].maxMoveY
                             <<" pointNum="<<pointNum
                            <<" threshold="<<student[k].standupThresHold<<std::endl;
                        std::swap(student[k].points[1], student[k].points[0]);
                    }
                } //end of else
            } //if a tobe confirmed student
        }//end of do lk truacking for tobe confirmed student


        // 4. do lk track for standup student
        for(k=0; k<MAXSTUDENTNUM; k++)
        {
            if(student[k].isStandup
                    && student[k].trackedPointNum>=LKPOINTS_UP) //need to tracking
            {
                if(student[k].standupTimeout==STANDUPTIMEOUT)
                {
                    //do frame diff for test
                    //1. frame diff current
                    tempPtr=framePtr;
                    tempPtr_1=framePtr-1;
                    if(tempPtr_1<0)
                        tempPtr_1=tempPtr_1+STANDUPTIME;
                    tempPtr_2=framePtr-2;
                    if(tempPtr_2<0)
                        tempPtr_2=tempPtr_2+STANDUPTIME;
                    std::cout<<" tempPtr="<<tempPtr<<" tempPtr_1="<<tempPtr_1
                            <<" tempPtr_2="<<tempPtr_2<<std::endl;

                    cv::absdiff(frameMat[tempPtr],frameMat[tempPtr_1],tempDiff1);
                    cv::threshold(tempDiff1,tempDiff1,5,255,THRESH_BINARY);

                    cv::absdiff(frameMat[tempPtr_1],frameMat[tempPtr_2],tempDiff2);
                    cv::threshold(tempDiff2,tempDiff2,5,255,THRESH_BINARY);

                    cv::bitwise_and(tempDiff1,tempDiff2,frameDiff1[k]);

                    //   cv::erode(frameDiff1[k],frameDiff1[k],cv::Mat());
                    //   cv::dilate(frameDiff1[k],frameDiff1[k],cv::Mat());

                    //   cv::dilate(segmentationMap,segmentationMap,element,Point(-1,-1),7);
                    //   cv::erode(segmentationMap,segmentationMap,cv::Mat(),Point(-1,-1),6);

                    //2.frame diff at start time
                    tempPtr=framePtr-(STANDUPTIME-student[k].trustedValue)+2;
                    if(tempPtr<0)
                        tempPtr=tempPtr+STANDUPTIME;
                    tempPtr_1=framePtr-1;
                    if(tempPtr_1<0)
                        tempPtr_1=tempPtr_1+STANDUPTIME;
                    tempPtr_2=framePtr-2;
                    if(tempPtr_2<0)
                        tempPtr_2=tempPtr_2+STANDUPTIME;

                    //                    std::cout<<" trustvalue="<<student[k].trustedValue
                    //                            <<" tempPtr="<<tempPtr
                    //                            <<" tempPtr_1="<<tempPtr_1
                    //                            <<" tempPtr_2="<<tempPtr_2<<std::endl;
                    cv::absdiff(frameMat[tempPtr],frameMat[tempPtr_1],tempDiff1);
                    cv::threshold(tempDiff1,tempDiff1,5,255,THRESH_BINARY);

                    cv::absdiff(frameMat[tempPtr_1],frameMat[tempPtr_2],tempDiff2);
                    cv::threshold(tempDiff2,tempDiff2,5,255,THRESH_BINARY);

                    cv::bitwise_and(tempDiff1,tempDiff2,frameDiff2[k]);

                    //  cv::erode(frameDiff2[k],frameDiff2[k],cv::Mat());
                    //  cv::dilate(frameDiff2[k],frameDiff2[k],cv::Mat());


                    cv::absdiff(frameDiff1[k],frameDiff2[k],tempDiff);
                    cv::threshold(tempDiff,tempDiff,5,255,THRESH_BINARY);

                    //                   cv::bitwise_or(frameDiff1[k],frameDiff2[k],tempDiff);

                    cv::rectangle(frameDiff1[k], student[k].rect, cv::Scalar(200), 2);
                    cv::rectangle(frameDiff2[k], student[k].initialRect, cv::Scalar(128), 2);

                    cv::imshow("FrameDiff1",frameDiff1[k]);
                    cv::imshow("FrameDiff2",frameDiff2[k]);
                    cv::imshow("FrameDiff",tempDiff);

                    //waitKey();

                    student[k].standupTimeout--;
                    continue;
                }
                else
                {
                    //do lk for standup student
                    cv::calcOpticalFlowPyrLK(
                                frame_prev, frame, // 2 consecutive images
                                student[k].points[0], // input point positions in first image
                            student[k].points[1], // output point positions in the 2nd image
                            status, // tracking success
                            err,
                            winSize,1,termcrit,0,0.01); // tracking error
                    //  a.loop over the tracked points to reject some
                    pointNum=0;
                    //  xmove=0;
                    //  ymove=0;
                    sumXmove=0;
                    sumYmove=0;
                    //  stepxmove=0;
                    //  stepymove=0;
                    //   stepsumXmove=0;
                    //   stepsumYmove=0;
                    for( int i= 0; i < student[k].points[1].size(); i++ ) {
                        //  do we keep this point?
                        if(status[i]){
                            // xmove=student[k].points[0][i].x-student[k].points[1][i].x;
                            //  ymove=student[k].points[0][i].y-student[k].points[1][i].y;
                            stepxmove=student[k].points[1][i].x-student[k].points[0][i].x;
                            stepymove=student[k].points[1][i].y-student[k].points[0][i].y;
                            if(fabs(stepxmove)<=MAXLKMOVEX && fabs(stepymove)<=MAXLKMOVEY)
                            {
                                xmove=student[k].initial[i].x-student[k].points[1][i].x;
                                ymove=student[k].initial[i].y-student[k].points[1][i].y;
                                //add 3.9                       //  keep this point in vector
                                if(student[k].standupTimeout==STANDUPTIMEOUT-1)
                                {
                                    if(ymove>student[k].standupThresHold/2)
                                    {
                                        student[k].initial[pointNum] = student[k].initial[i];
                                        student[k].points[1][pointNum++] = student[k].points[1][i];
                                        sumXmove += xmove;
                                        sumYmove += ymove;
                                        cv::circle(colorFrame, student[k].points[1][i], 1,cv::Scalar(0,0,255),-1);
                                    }
                                }
                                else
                                {
                                    student[k].initial[pointNum] = student[k].initial[i];
                                    student[k].points[1][pointNum++] = student[k].points[1][i];
                                    sumXmove += xmove;
                                    sumYmove += ymove;
                                    cv::circle(colorFrame, student[k].points[1][i], 1,cv::Scalar(0,0,255),-1);
                                }
                                //                            stepsumXmove += stepxmove;
                                //                            stepsumYmove += stepymove;
                            }
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
                            if(student[k].maxMoveY>student[k].standupThresHold*1.5)
                                student[k].maxMoveY=student[k].standupThresHold*1.5;
                        }

                        std::cout<<" k="<<k
                                <<" timeout="<<student[k].standupTimeout<<" y="<<student[k].moveY
                               <<"  x="<<student[k].moveX
                              <<"  maxmove="<<student[k].maxMoveY
                             <<" pointNum="<<pointNum
                            <<" threshold="<<student[k].standupThresHold<<std::endl;
                        std::swap(student[k].points[1], student[k].points[0]);
                    }
                }//end of else(student[k].standupTimeout
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
                    && student[k].moveY<(student[k].maxMoveY-student[k].standupThresHold*2/3))
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
                                            tr("Vedio Files(*.mp4 *.avi *.mkv *.yuv *.wmv)"));

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

#define DETECT_BUFFER_SIZE 0x20000
enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };

cv::Rect2i roiRect;
int rectState=SET;
cv::Point2i startP;

void face_on_mouse( int event, int x, int y, int flags, void* para )
{
    // TODO add bad args check
    using namespace cv;
    switch( event )
    {
    case EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
        {
                rectState = IN_PROCESS;
                roiRect = cv::Rect( x, y, 1, 1 );


        }
        break;
    case EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
        {

        }
        break;
    case EVENT_LBUTTONUP:
        if( rectState == IN_PROCESS )
        {
            roiRect =cv::Rect( cv::Point(roiRect.x, roiRect.y), cv::Point(x,y) );
            rectState = SET;

        }

        break;
    case EVENT_RBUTTONUP:

        {

        }
        break;
    case EVENT_MOUSEMOVE:
        if( rectState == IN_PROCESS )
        {
            roiRect = cv::Rect( cv::Point(roiRect.x, roiRect.y), cv::Point(x,y) );

        }

        break;
    }
}
void MainWindow::on_face_pushButton_clicked()
{
    cv::Mat inputFrame,gray;
    QString  ipaddress;

    //-----------------------测试时间
    struct timeval tsBegin,tsEnd;
    long runtimes;
    cv::Rect2i rect;
    int roiLeft,roiTop,roiWidth,roiHeight;

    roiLeft=10;
    roiTop=10;
    roiWidth=900;
    roiHeight=500;
    roiRect.x=roiLeft;
    roiRect.y=roiTop;
    roiRect.width=roiWidth;
    roiRect.height=roiHeight;

    unsigned char* imgPtr;


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

    cv::VideoCapture capture(filename.toStdString());
    //   VideoCapture capture("rtsp://192.168.100.182/2");
    if(!capture.isOpened())
    {
        ui->warning_lineEdit->setText("File open error,please open video file first");
        return ;
    }

    unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
    if (!pBuffer)
    {
        std::cout<<"Can not alloc buffer"<<std::endl;
        return ;
    }


    int * pResults = NULL;
    int num_faces;

    cv::namedWindow("showFrame");
  //  cv::namedWindow("Gray");

    int frameInterV=0;
    int frameNumber=0;
    int keyboard;

    cv::setMouseCallback( "showFrame", face_on_mouse, 0 );

    while ((char)keyboard != 'q' && (char)keyboard != 27)
    {
        /* Read the current frame. */
        if (!capture.read(inputFrame)) {
            ui->warning_lineEdit->setText("end of File ");
            capture.release();

            cv::destroyAllWindows();
            return;
        }

        // wait and reset frameNumber
        if (frameNumber < 10){
            ++frameNumber;
            continue;
        }
        if(frameNumber>10000000)
            frameNumber=20;


       if(frameInterV==1)
                frameInterV=0;
       if(frameInterV>0)
       {
           frameNumber++;
           frameInterV++;
           continue;
       }
       frameInterV++;



       cv::cvtColor(inputFrame, gray, CV_BGR2GRAY);

       gettimeofday(&tsBegin, NULL);

       pResults = NULL;

       cv::rectangle(inputFrame, roiRect, cv::Scalar(255, 0, 0), 2);

       if(rectState!=SET || roiRect.width<30 || roiRect.height<30)
       {
           cv::imshow("showFrame",inputFrame);
           keyboard=cv::waitKey(33);
           continue;
       }
       imgPtr=(unsigned char*)(gray.ptr(0))+gray.cols*roiRect.y+roiRect.x;
       pResults = facedetect_frontal_surveillance(pBuffer, imgPtr, roiRect.width, roiRect.height, (int)gray.step, 1.2f, 2, 20, 80, 0);
       num_faces = (pResults ? *pResults : 0);
       //print the detection results
       //resultfile << filename << endl;
       std::cout <<"number of faces="<<num_faces << std::endl;
       for (int i = 0; i < num_faces; i++)
       {
           short * p = ((short*)(pResults + 1)) + 142 * i;
           int x = p[0]+roiLeft;
           int y = p[1]+roiTop;
           int w = p[2];
           int h = p[3];
           int neighbors = p[4];
           int angle = p[5];
           std::cout<< x << " " << y << " " << w << " " << h << " " << neighbors << std::endl;
           rect.x=roiRect.x+x;
           rect.y=roiRect.y+y;
           rect.width=w;
           rect.height=h;
           cv::rectangle(inputFrame, rect, cv::Scalar(0, 0, 255), 2);
       }

       cv::imshow("showFrame",inputFrame);

       gettimeofday(&tsEnd, NULL);//-----------------------测试时间
       // long runtimes;
       runtimes=1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+tsEnd.tv_usec-tsBegin.tv_usec;
       std::cout<<"time: "<<runtimes<<std::endl;

       keyboard=cv::waitKey(5);
    }
       //release the buffer
       free(pBuffer);
       cv::destroyAllWindows();


}
