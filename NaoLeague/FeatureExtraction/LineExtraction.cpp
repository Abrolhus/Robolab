#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <time.h>

using namespace cv;

int GREEN_THRESHOLD = 34;
int BINARY_TYPE = 0;
int MAX_BINARY_VALUE = 255;
double LINE_CONN_THERSHOLD = 3.0;
double LINE_ANGLE_THERSHOLD = 15.0;
double YEL_N_THR_R = 0.38;
double YEL_N_THR_B = 0.2;
double YEL_N_THR_G = 0.38;
double YEL_THR_R = 230;
double YEL_THR_B = 160;
double YEL_THR_G = 230;
double WHITE_THR = 200;
const double PI  =3.141592653589793238462;
void goalPostDetection(Mat);
double distance_point_line(int, int, int, int, int, int);
double int_dis_point_line(int, int, int, int, int, int);
double angle_bet_lines(int, int, int, int, int, int, int, int);
double distance_point_point(int, int, int, int);
vector<Vec3i> find_connections(vector<Vec4i>);
vector<Vec4i> reduce_lines(vector<Vec4i>);
Mat remove_background(Mat);
double line_angle(int, int, int, int);

int main(int argc, char** argv)
{
    Mat src, dst, color_dst, src_no_backgr;
    Mat src_line_binary;
    if( argc != 2 || !(src=imread(argv[1], 1)).data)
        return -1;
    
    Mat e;
    src.copyTo(e);
    imshow( "Detected", e );
    // Measuring performance
    clock_t startTime = clock();

    src_line_binary = remove_background(src);
    
    Canny( src_line_binary, dst, 50, 200, 3 );
    cvtColor( dst, color_dst, CV_GRAY2BGR );

    vector<Vec4i> lines;
    HoughLinesP( dst, lines, 1, CV_PI/300, 10, 20, 20 );

    for( size_t i = 0; i < lines.size(); i++ )
    {
        line( color_dst, Point(lines[i][0], lines[i][1]),
        Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 2, 8 );
    }
    
    vector<Vec3i> points;
    points = find_connections(lines);
    
    for( int i = 0; i < points.size(); i++ )
    {
        if(points[i][2] == 0)
        {
            circle(e, Point(points[i][0], points[i][1]), 5, Scalar(0,255,0), 2, 8, 0);
        }
        else if(points[i][2] == 1)
        {
            circle(e, Point(points[i][0], points[i][1]), 5, Scalar(255,0,0), 2, 8, 0);
        }
    }
    std::cout << double( clock() - startTime )*1000 / (double)CLOCKS_PER_SEC<< " ms." << std::endl;
    std::cout << "number of lines: " << lines.size() << std::endl;
    std::cout << "number of points: " << points.size() << std::endl;
    namedWindow( "Detected Lines", 1);
    imshow( "Detected Lines", e);
    waitKey(0);
    return 0;
}

Mat remove_background(Mat image){
    Mat green = cvCreateMat(image.rows, image.cols, CV_8U);
    Mat yellow = Mat::zeros(image.rows, image.cols, CV_8UC3);
    int count;
    bool background;
    double r, g, b, jg, jr, jb, justGreen;
    for(int j = 0; j < image.cols; j ++)
    {
        count = 0;
        background = true;
        for(int i = 0; i < image.rows; i ++)
        {
            // pixel values
            jr = (double)image.at<cv::Vec3b>(i,j)[2];
            jg = (double)image.at<cv::Vec3b>(i,j)[1];
            jb = (double)image.at<cv::Vec3b>(i,j)[0];
            // how much green is the specific pixel
            justGreen = max(jg - jr/2 - jb/2, 0.0);
            // normalized rgb values
            r = jr / (jg + jr + jb);
            g = jg / (jg + jr + jb);
            b = jb / (jg + jr + jb);

            if(r > YEL_N_THR_R && g > YEL_N_THR_G && b < YEL_N_THR_B)
            {
                yellow.at<cv::Vec3b>(i,j)[0] = 255;
                yellow.at<cv::Vec3b>(i,j)[1] = 255;
                yellow.at<cv::Vec3b>(i,j)[2] = 255;
            }
            else if(jr > YEL_THR_R && jg > YEL_THR_G && jb < YEL_THR_B)
            {
                yellow.at<cv::Vec3b>(i,j)[0] = 255;
                yellow.at<cv::Vec3b>(i,j)[1] = 255;
                yellow.at<cv::Vec3b>(i,j)[2] = 255;
            }

            if (background)
            {                                
                (green.data + green.step * i)[j] = 255;
                if (justGreen > 34)
                {                    
                    if(r > YEL_N_THR_R && g > YEL_N_THR_G && b < YEL_N_THR_B)
                    {
                        (green.data + green.step * i)[j] = 0;
                    }
                    else if(jr > YEL_THR_R && jg > YEL_THR_G && jb < YEL_THR_B)
                    {
                        (green.data + green.step * i)[j] = 0;
                    }
                    else
                    {
                        (green.data + green.step * i)[j] = 255;
                    }
                }
                else
                {
                    (green.data + green.step * i)[j] = 0;
                }
                if((green.data + green.step * i)[j] > 0)
                {
                    count ++;
                    image.at<cv::Vec3b>(i,j)[0] = 0;
                    image.at<cv::Vec3b>(i,j)[1] = 0;
                    image.at<cv::Vec3b>(i,j)[2] = 0;
                    background = (count <= 5);
                }
                else
                {
                    image.at<cv::Vec3b>(i,j)[0] = 0;
                    image.at<cv::Vec3b>(i,j)[1] = 0;
                    image.at<cv::Vec3b>(i,j)[2] = 0;
                }
            }
            else
            {
                //if almost white make it white...
                if(jr > WHITE_THR && jb > WHITE_THR && jg > WHITE_THR)
                {
                    image.at<cv::Vec3b>(i,j)[0] = 255;
                    image.at<cv::Vec3b>(i,j)[1] = 255;
                    image.at<cv::Vec3b>(i,j)[2] = 255;
                // else make it black...
                }else
                {
                    image.at<cv::Vec3b>(i,j)[0] = 0;
                    image.at<cv::Vec3b>(i,j)[1] = 0;
                    image.at<cv::Vec3b>(i,j)[2] = 0;
                }
            }
            
        }
    }
    goalPostDetection(yellow);
    return image;
}

void goalPostDetection(Mat yellow){
    Mat dst, color_dst;
    Canny( yellow, dst, 50, 200, 3 );
    cvtColor( dst, color_dst, CV_GRAY2BGR );

    vector<Vec4i> lines;
    HoughLinesP( dst, lines, 1, CV_PI/300, 5, 30, 10 );

    vector<Vec4i> lines_ver;
    vector<Vec4i> lines_hor;
    for( int i = 0; i < lines.size(); i++ )
    {
        std::cout << line_angle(lines[i][0],lines[i][1],lines[i][2],lines[i][3]) << std::endl;
        if(line_angle(lines[i][0],lines[i][1],lines[i][2],lines[i][3]) > 80 &&
            line_angle(lines[i][0],lines[i][1],lines[i][2],lines[i][3]) < 100){
            lines_ver.push_back(Vec4i(lines[i][0],lines[i][1],lines[i][2],lines[i][3]));
        }else{
            lines_hor.push_back(Vec4i(lines[i][0],lines[i][1],lines[i][2],lines[i][3]));
        }
    }

    Point pointUR = Point(yellow.rows, 0);
    Point pointDR = Point(0, 0);
    Point pointUL = Point(yellow.rows, yellow.cols);
    Point pointDL = Point(0, yellow.cols);

    for( size_t i = 0; i < lines_ver.size(); i++ )
    {
        // line( color_dst, Point(lines_ver[i][0], lines_ver[i][1]),
        // Point(lines_ver[i][2], lines_ver[i][3]), Scalar(0,0,255), 2, 8 );
        for( int p = 0; p < 2; p++ ){
            circle(color_dst, Point(lines_ver[i][2*p], lines_ver[i][2*p+1]), 5, Scalar(255,0,0), 2, 8, 0);
            // if(lines_ver[i][2*p] < pointUR[0])
            //     pointUR[0] = lines_ver[i][2*p];
            // if(lines_ver[i][2*p+1] > pointUR[1])
            //     pointUR[1] = lines_ver[i][2*p+1];

            // if(lines_ver[i][2*p] > pointDR[0])
            //     pointDR[0] = lines_ver[i][2*p];
            // if(lines_ver[i][2*p+1] > pointDR[1])
            //     pointDR[1] = lines_ver[i][2*p+1];

            // if(lines_ver[i][2*p] < pointUL[0])
            //     pointUL[0] = lines_ver[i][2*p];
            // if(lines_ver[i][2*p+1] < pointUL[1])
            //     pointUL[1] = lines_ver[i][2*p+1];

            if(lines_ver[i][2*p] > pointDL.x)
                pointDL.x = lines_ver[i][2*p];
            if(lines_ver[i][2*p+1] < pointDL.y)
                pointDL.y = lines_ver[i][2*p+1];

        }
    }

    // circle(color_dst, Point(pointUR[0], pointUR[1]), 5, Scalar(255,0,0), 2, 8, 0);
    // circle(color_dst, Point(pointUL[0], pointUL[1]), 5, Scalar(255,0,0), 2, 8, 0);
    // circle(color_dst, Point(pointDR[0], pointDR[1]), 5, Scalar(255,0,0), 2, 8, 0);
    circle(color_dst, pointDL, 5, Scalar(0,255,0), 2, 8, 0);

    imshow("posts2",color_dst);


    return;
}


double distance_point_point(int ax, int ay, int bx, int by)
{
    double dx = (ax-bx);
    double dy = (ay-by);
    return sqrt(dx*dx + dy*dy);
}

double distance_point_line(int x, int y, int x1, int y1, int x2, int y2)
{
    double A = x - x1;
    double B = y - y1;
    double C = x2 - x1;
    double D = y2 - y1;
    double dot = A * C + B * D;
    double len_sq = C * C + D * D;
    double param = dot / len_sq;
    double xx,yy;

    if(param < 0)
    {
        xx = x1;
        yy = y1;
    }
    else if(param > 1)
    {
        xx = x2;
        yy = y2;
    }
    else
    {
        xx = x1 + param * C;
        yy = y1 + param * D;
    }
    return distance_point_point(x,y,xx,yy);
}

double int_dis_point_line(int x, int y, int x1, int y1, int x2, int y2)
{
    double A = x - x1;
    double B = y - y1;
    double C = x2 - x1;
    double D = y2 - y1;
    double dot = A * C + B * D;
    double len_sq = C * C + D * D;
    double param = dot / len_sq;
    double xx,yy;

    if(param < 0)
    {
        xx = x1;
        yy = y1;
    }
    else if(param > 1)
    {
        xx = x2;
        yy = y2;
    }
    else
    {
        xx = x1 + param * C;
        yy = y1 + param * D;
    }
    return distance_point_point(x,y,(x1+x2)/2,(y1+y2)/2);
}

double line_angle(int x1, int y1, int x2, int y2)
{
    double angle = atan2(y1 - y2, x1 - x2);
    double res = angle * 180 / PI;
    return abs(res);
}

double angle_bet_lines(int a1x, int a1y, int b1x, int b1y, int a2x, int a2y, int b2x, int b2y)
{
    double angle1 = atan2(a1y - b1y, a1x - b1x);
    double angle2 = atan2(a2y - b2y, a2x - b2x);
    double res = (angle1-angle2) * 180 / PI;
    return abs(res); 
}

vector<Vec3i> find_connections(vector<Vec4i> lines)
{
    vector<Vec3i> points;
    for( int i = 0; i < lines.size(); i++ )
    {
        for( int j = 0; j < lines.size(); j++ )
        {
            if(i != j)
            {
                for( int p = 0; p < 2; p++ )
                {
                    // point's distance from intersection point is below the thershold
                    if(distance_point_line(lines[i][2*p], lines[i][2*p+1], lines[j][0], lines[j][1], lines[j][2], lines[j][3]) < LINE_CONN_THERSHOLD){
                        // intersection point is near line's start or end
                        if(abs(int_dis_point_line(lines[i][2*p], lines[i][2*p+1], lines[j][0], lines[j][1], lines[j][2], lines[j][3]) -
                            distance_point_point(lines[j][0], lines[j][1], lines[j][2], lines[j][3])/2) < LINE_CONN_THERSHOLD)
                        {
                            // angle between the two lines has to be larger than a thershold to not to be considered,
                            // a straight line.
                            if(angle_bet_lines(lines[j][0], lines[j][1], lines[j][2], lines[j][3],lines[i][0], lines[i][1], lines[i][2], lines[i][3]) > LINE_ANGLE_THERSHOLD)
                            {
                                points.push_back(Vec3i(lines[i][2*p], lines[i][2*p+1], 0));
                            }
                        }
                        else if(int_dis_point_line(lines[i][2*p], lines[i][2*p+1], lines[j][0], lines[j][1], lines[j][2], lines[j][3]) -
                            distance_point_point(lines[j][0], lines[j][1], lines[j][2], lines[j][3])/2 < -10.0)
                        {
                                if(angle_bet_lines(lines[j][0], lines[j][1], lines[j][2], lines[j][3],lines[i][0], lines[i][1], lines[i][2], lines[i][3]) > LINE_ANGLE_THERSHOLD)
                                {
                                    points.push_back(Vec3i(lines[i][2*p], lines[i][2*p+1], 1));
                                }       
                        }
                    }
                }  
            }
        }
    }
    return points;
}
vector<Vec4i> reduce_lines(vector<Vec4i> lines)
{
    vector<Vec4i> reduced_lines;
    for( int i = 0; i < lines.size(); i++)
    {
        if(reduced_lines.size() == 0)
        {
            reduced_lines.push_back(lines[i]);
        }
        else
        {
            bool found_same = false;
            for( int j = 0; j < lines.size(); j++)
            {
                // lines with approximately equal end points and start
                // points will be removed
                for( int p = 0; p < 2; p++ )
                {
                    for( int l = 0; l < 2; l++ )
                    {
                        // if points are too close
                        if(distance_point_point(lines[i][2*p], lines[i][2*p+1], reduced_lines[j][2*l], reduced_lines[j][2*l+1]) < LINE_CONN_THERSHOLD)
                        {
                            if(distance_point_point(lines[i][2*l], lines[i][2*l+1], reduced_lines[j][2*p], reduced_lines[j][2*p+1]) < LINE_CONN_THERSHOLD)
                            {
                                // if already stored line is longer than the current, keep it and discard the current
                                double stored_line_l = distance_point_point(lines[j][0], lines[j][1], reduced_lines[j][2], reduced_lines[j][3]);
                                double current_line_l = distance_point_point(lines[i][0], lines[i][1], reduced_lines[i][2], reduced_lines[i][3]);
                                found_same = true;
                            }
                        }
                    }
                }
            }
            if (found_same == false)
            {
                reduced_lines.push_back(lines[i]);
            }
        }
    }
    return reduced_lines;
}