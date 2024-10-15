#ifndef ThreadAffine_HPP
#define ThreadAffine_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadAffine : public QThread
{
  Q_OBJECT
public:

  QString name = "Affine";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadAffine() { }

  void run() {

    isHaveError = false;
    output_js_data = QJsonObject();
    QJsonObject jso;
    if(isUseImage) {
      if(mat_im.empty()) {
        jso["error"] = "No image";
        output_js_data[name] = jso;
        isHaveError = true;
        return;
      }
    }

    //********* run here **************
    int x1 = param_js_data["x1"].toInt();
    int y1 = param_js_data["y1"].toInt();
    int x2 = param_js_data["x2"].toInt();
    int y2 = param_js_data["y2"].toInt();
    int x3 = param_js_data["x3"].toInt();
    int y3 = param_js_data["y3"].toInt();
    int x4 = param_js_data["x4"].toInt();
    int y4 = param_js_data["y4"].toInt();
    int combo_type = param_js_data["combobox"].toInt();
    QString type_array[] = {"Perspective","Affine"};
//    QString type_array[] = {"Affine","Perspective"};
    QString type = type_array[combo_type];

    int x[] = {x1, x2, x3,x4};
    int min_x = x[0];
    for(int i =1;i<=3;i++){
      if(x[i]<min_x){
        min_x = x[i];
      }
    }
    int max_x = x[0];
    for(int i =1;i<=3;i++){
      if(x[i]>max_x){
        max_x = x[i];
      }
    }
    int y[] = {y1, y2, y3, y4};
    int min_y = y[0];
    for(int i =1;i<=3;i++){
      if(y[i]<min_y){
        min_y = y[i];
      }
    }
    int max_y = y[0];
    for(int i =1;i<=3;i++){
      if(y[i]>max_y){
        max_y = y[i];
      }
    }
    int x_diff = max_x-min_x;
    int y_diff = max_y-min_y;

    if(type=="Affine"){
      cv::Point2f srcTri[3];
      srcTri[0] = cv::Point2f(x1,y1);
      srcTri[1] = cv::Point2f(x2,y2);
      srcTri[2] = cv::Point2f(x3,y3);
      cv::Point2f dstTri[3];
      dstTri[0] = cv::Point2f(0.f,0.f);
      dstTri[1] = cv::Point2f(x_diff,0.f);
      dstTri[2] = cv::Point2f(0.f,y_diff);
      cv::Mat warp_mat = cv::getAffineTransform(srcTri,dstTri);
      cv::warpAffine(mat_im, mat_im, warp_mat, cv::Size(x_diff, y_diff));
    }else{
      cv::Point2f srcTri[3];
      srcTri[0] = cv::Point2f(x1,y1);
      srcTri[1] = cv::Point2f(x2,y2);
      srcTri[2] = cv::Point2f(x3,y3);
      srcTri[3] = cv::Point2f(x4,y4);
      cv::Point2f dstTri[3];
      dstTri[0] = cv::Point2f(0.f,0.f);
      dstTri[1] = cv::Point2f(x_diff,0.f);
      dstTri[2] = cv::Point2f(0.f,y_diff);
      dstTri[3] = cv::Point2f(x_diff,y_diff);
      cv::Mat warp_mat = cv::getPerspectiveTransform(srcTri,dstTri);
      cv::warpPerspective(mat_im, mat_im, warp_mat, cv::Size(x_diff, y_diff));
    }







//    jso["msg"] = "Hello World";
    jso["x_max"] = max_x;
    jso["x_min"] = min_x;
    jso["y_max"] = max_y;
    jso["y_min"] = min_y;
    jso["type"] = type;


    payload_js_data[name] = jso;

  }


};

#endif // ThreadAffine_HPP
