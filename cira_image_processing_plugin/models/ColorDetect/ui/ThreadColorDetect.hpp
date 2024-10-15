#ifndef ThreadColorDetect_HPP
#define ThreadColorDetect_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadColorDetect : public QThread
{
  Q_OBJECT
public:

  QString name = "ColorDetect";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im,mat_im_copy,mat_mask;

  bool isHaveError = false;

  ThreadColorDetect() { }

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

    mat_im_copy = mat_im.clone();
    //mat_im_copy2 = mat_im_copy.clone();

    bool group_Box = param_js_data["group_Box"].toBool();

    int lower_H_Box = param_js_data["lower_H_Box"].toInt();
    int lower_S_Box = param_js_data["lower_S_Box"].toInt();
    int lower_V_Box = param_js_data["lower_V_Box"].toInt();
    int upper_H_Box = param_js_data["upper_H_Box"].toInt();
    int upper_S_Box = param_js_data["upper_S_Box"].toInt();
    int upper_V_Box = param_js_data["upper_V_Box"].toInt();

    int lower_H_Bar = param_js_data["lower_H_Bar"].toInt();
    int lower_S_Bar = param_js_data["lower_S_Bar"].toInt();
    int lower_V_Bar = param_js_data["lower_V_Bar"].toInt();
    int upper_H_Bar = param_js_data["upper_H_Bar"].toInt();
    int upper_S_Bar = param_js_data["upper_S_Bar"].toInt();
    int upper_V_Bar = param_js_data["upper_V_Bar"].toInt();

    if(group_Box == true){

      cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2HSV);
      cv::Scalar minHSV = cv::Scalar(lower_H_Bar,lower_S_Bar,lower_V_Bar);
      cv::Scalar maxHSV = cv::Scalar(upper_H_Bar,upper_S_Bar,upper_V_Bar);

      cv::inRange(mat_im, minHSV, maxHSV,mat_im);
      mat_mask = mat_im.clone();
      cv::bitwise_and(mat_im_copy,mat_im_copy,mat_im, mat_mask);
      cv::cvtColor(mat_mask,mat_mask, cv::COLOR_GRAY2BGR);

    }
    else{

      cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2HSV);
      cv::Scalar minHSV = cv::Scalar(lower_H_Box,lower_S_Box,lower_V_Box);
      cv::Scalar maxHSV = cv::Scalar(upper_H_Box,upper_S_Box,upper_V_Box);

      cv::inRange(mat_im, minHSV, maxHSV,mat_im);
      mat_mask = mat_im.clone();
      cv::bitwise_and(mat_im_copy,mat_im_copy,mat_im, mat_mask);
      cv::cvtColor(mat_mask,mat_mask, cv::COLOR_GRAY2BGR);

    }

    //jso["mat_bit"] = mat_null;


    payload_js_data[name] = jso;

  }


};

#endif // ThreadColorDetect_HPP
