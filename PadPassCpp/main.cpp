#include <iostream>
#include <map>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <time.h>
#include "keras_model.h"

using namespace std;
using namespace cv;
using namespace keras;
static int count_digit=0;
KerasModel m("/Users/wzq/keras2cpp/model_handwrite.nnet", true);
Mat resizeimg(Mat rawimg);
Mat resize_resize(Mat rawimage){
    float fx=480.0 /(float) rawimage.size().width;
    float fy=640.0 /(float) rawimage.size().height;
    fx=fy=min(fx,fy);
    Mat result;
    resize(rawimage,result,Size(),fx,fy,INTER_CUBIC);
    return result;
}
bool sort_cmp_x_greater(const Rect &rect1, const Rect &rect2){
    if (rect1.x<rect2.x)return true;
    return false;
}
bool sort_cmp_y_greater(const Rect &rect1, const Rect &rect2){
    if (rect1.y<rect2.y)return true;
    return false;
}
void slice_first_line(Mat frame,vector<Rect> pos_rect_left,vector<Rect> pos_rect_right,vector<Mat> &image_digit){
    int arg_line=0;
    float x=pos_rect_left[arg_line].x+pos_rect_left[arg_line].width;
    float y=(pos_rect_left[arg_line].y+pos_rect_right[arg_line].y)/2-5;
    float length=0;
    float height=0;
    for (int i = 0; i < 2; ++i) {
        length+=(pos_rect_right[i].x-pos_rect_left[i].x-pos_rect_left[i].width);
    }
    length=(length)/2;
    length = (length) /3;

    height+=pos_rect_left[1].y-y+0.77*pos_rect_left[1].height;
    height+=pos_rect_right[1].y-y+0.77*pos_rect_right[1].height;
    height=height/2;

    Mat frame_copy=frame.clone();
    int x1=(int)x,y1=(int)y,height1=(int)height,length1=(int)length;

    for (int i = 0; i < 3; ++i) {
        Rect rect=Rect(x1+ length1*i,y1,length1,height1);
        Mat mat(frame_copy,rect);
        image_digit.push_back(mat);
    }

}
void slice_second_line(Mat frame,vector<Rect> pos_rect_left,vector<Rect> pos_rect_right,vector<Mat> &image_digit){
    int arg_line=1;
    float x=pos_rect_left[arg_line].x+pos_rect_left[arg_line].width;
    float y=(pos_rect_left[arg_line].y+pos_rect_right[arg_line].y+0.75*pos_rect_left[arg_line].height+0.75*pos_rect_right[arg_line].height)/2;
    float length=0;
    float height=0;
    for (int i = 1; i < 3; ++i) {
        length+=(pos_rect_right[i].x-pos_rect_left[i].x-pos_rect_left[i].width);
    }
    length=(length)/2;
    length = (length) /3;

    height+=pos_rect_left[3].y-y+0.29*pos_rect_left[3].height;
    height+=pos_rect_right[3].y-y+0.29*pos_rect_right[3].height;
    height=height/2;

    Mat frame_copy=frame.clone();
    int x1=(int)x,y1=(int)y,height1=(int)height,length1=(int)length;

    for (int i = 0; i < 3; ++i) {
        Rect rect=Rect(x1+ length1*i,y1,length1,height1);
        Mat mat(frame_copy,rect);
        image_digit.push_back(mat);
    }

}
void slice_third_line(Mat frame,vector<Rect> pos_rect_left,vector<Rect> pos_rect_right,vector<Mat> &image_digit){
    int arg_line=3;
    float x=pos_rect_left[arg_line].x+pos_rect_left[arg_line].width;
    float y=(pos_rect_left[arg_line].y+pos_rect_right[arg_line].y+0.25*pos_rect_left[arg_line].height+0.25*pos_rect_right[arg_line].height)/2;
    float length=0;
    float height=0;
    for (int i = 3; i < 5; ++i) {
        length+=(pos_rect_right[i].x-pos_rect_left[i].x-pos_rect_left[i].width);
    }
    length=(length)/2;
    length = (length) /3;

    height+=pos_rect_left[4].y-y+1.5*pos_rect_left[4].height;
    height+=pos_rect_right[4].y-y+1.5*pos_rect_right[4].height;
    height=height/2;

    Mat frame_copy=frame.clone();
    int x1=(int)x,y1=(int)y,height1=(int)height,length1=(int)length;

    for (int i = 0; i < 3; ++i) {
        Rect rect=Rect(x1+ length1*i,y1,length1,height1);
        Mat mat(frame_copy,rect);
        image_digit.push_back(mat);
        //imshow(to_string(i),mat);
    }

}
void process(Mat frame){
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<Rect> pos_rect;

    //对图像大小进行处理
    frame=resize_resize(frame);
    GaussianBlur(frame,frame,Size(5, 5),1);
    Mat hsv,mask;
    cvtColor(frame,hsv,COLOR_BGR2HSV);
    Scalar lower_white=Scalar(180, 0, 0);
    Scalar upper_white=Scalar(255, 255, 255);
    inRange(frame,lower_white,upper_white,mask);

    //边缘检测寻找矩形
    findContours( mask.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    for (int i = 0; i < contours.size(); ++i) {
        vector<Point> approx;
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.04, true);
        if(approx.size()==4||approx.size()==5||approx.size()==6){
            Rect rect=boundingRect(contours[i]);
            int x=rect.x,y=rect.y,w=rect.width,h=rect.height;
            int area=w*h;

            if(area<100 || area > 300 || h>w || ((float)w/(float)h)<=1)continue;//通过大小进行过滤
            Mat mask_rect = Mat(mask, rect);
            Scalar mask_rect_average=mean(mask_rect);
            if(mask_rect_average[0]<205)continue;//通过平均颜色进行过滤
            pos_rect.push_back(rect);
            //rectangle(frame,rect,Scalar(0,0,255),2);
        }
    }
    if(pos_rect.size()!=10){
        cout<<"cannot find location rectangle"<<endl;
        return;
    }
    //将左右两边定位位点分开,并排序
    sort(pos_rect.begin(),pos_rect.end(),sort_cmp_x_greater);
    vector<Rect> pos_rect_left(pos_rect.begin(),pos_rect.begin()+5),pos_rect_right(pos_rect.begin()+5,pos_rect.end());
    sort(pos_rect_left.begin(),pos_rect_left.end(),sort_cmp_y_greater);
    sort(pos_rect_right.begin(),pos_rect_right.end(),sort_cmp_y_greater);


    vector<Mat> image_digit;

    //切割出九宫格
    slice_first_line(frame,pos_rect_left,pos_rect_right,image_digit);
    slice_second_line(frame,pos_rect_left,pos_rect_right,image_digit);
    slice_third_line(frame,pos_rect_left,pos_rect_right,image_digit);
    vector<Mat> image_digit_final;
    //对切割出的图片进行处理获取纯数字
    for (int i = 0; i < 9; ++i) {
        Mat edge;
        Canny(image_digit[i],edge, 500, 2000,5);
        findContours(edge.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
        Mat copy = image_digit[i].clone();
        Mat im_gray;

        cvtColor(copy,im_gray,COLOR_BGR2GRAY);
        Mat im_th;
        threshold(im_gray,im_th,230,255,THRESH_BINARY_INV);

        map<int,Mat> temp_dict;
        int max_index=-1;
        for (int j = 0; j < contours.size(); ++j) {
            Rect rect=boundingRect(contours[j]);
            int area=rect.width*rect.height;
            if(rect.width<4||rect.height<5||copy.size().width-rect.width<3)continue;
            max_index=max(max_index,area);
            rect.y+=3;
            rect.x+=2;
            rect.height-=5;
            rect.width-=5;
            Mat img_temp(im_th,rect);
            temp_dict[area]=img_temp;
        }
        if(max_index==-1){
            cout<<"cannot find max image"<<endl;
            continue;
        }
        Mat image_max=temp_dict[max_index];
        image_digit_final.push_back(image_max);
    }
    count_digit=0;
    for (int i = 0; i < 9; ++i) {

        resizeimg(image_digit_final[i]);
        count_digit++;
    }
    //利用卷积神经网络进行识别

}
int predict(cv::Mat img){   //预测数字 conv
//    cv::bitwise_not(img, img);

    vector<vector<vector<float>>> data;
    vector<vector<float>> d;
    for (int i = 0; i < img.rows; i++) {
        vector<float> r;
        for (int j = 0; j < img.cols; j++) {
            uchar temp=img.at<uchar>(i, j);
            r.push_back(temp/255);
        }
        d.push_back(r);
    }
    data.push_back(d);

    DataChunk * dc = new DataChunk2D();
    dc->set_data(data); //Mat 转 DataChunk

    vector<float> predictions = m.compute_output(dc);
//    predictions[10] = 0;

    auto max = max_element(predictions.begin(), predictions.end());
    int index = (int)distance(predictions.begin(), max);
    imshow(to_string(count_digit)+"-"+to_string(index),img);
//    waitKey(0);
    return index;
}
Mat resizeimg(Mat rawimg){
    int width=28;
    cv::Mat outimg(width, width, CV_8U, 255);



    outimg = cv::Scalar(0,0,0);
    float fc = (float)width / rawimg.cols;
    float fr = (float)width / rawimg.rows;
    fc = min(fc, fr);
    fr = fc;
    cv::Size size;
    size.width = rawimg.cols * fc;
    size.height = rawimg.rows * fr;
    if(size.width == 0 || size.height == 0)return outimg;
    cv::resize(rawimg, rawimg, size);
    int w = rawimg.cols, h = rawimg.rows;
    int x = (width - w)/2, y = (width - h)/2;
    rawimg.copyTo(outimg(cv::Rect(x, y, w, h)));
    //imshow("test",outimg);
    //waitKey(0);
    predict(outimg);
    return outimg;



}
void recognize(Mat mat){
    Mat resized;


    vector<vector<vector<float>>> digit(1, vector<vector<float>>(28, vector<float>(28)));



}
int main() {

    Mat frame=imread("/Users/wzq/RoboMaster/PadPass/test/796.jpg");
    clock_t tStart;
//    tStart= clock();
////    m.compute_output(sample);
//    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    tStart= clock();
    process(frame);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    imshow("frame",frame);
    waitKey(0);

//    tStart= clock();
//    process(frame);
//    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    return 0;
}