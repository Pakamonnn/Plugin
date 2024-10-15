#ifndef ThreadBGRtoHSV_HPP
#define ThreadBGRtoHSV_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadBGRtoHSV : public QThread
{
  Q_OBJECT
public:

  QString name = "BGRtoHSV";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadBGRtoHSV() { }

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

    cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2HSV);
    jso["HSV"] = "HSV";


    payload_js_data[name] = jso;

  }


};

#endif // ThreadBGRtoHSV_HPP
