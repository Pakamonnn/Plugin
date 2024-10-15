#ifndef ThreadAdaptiveThs_HPP
#define ThreadAdaptiveThs_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadAdaptiveThs : public QThread
{
  Q_OBJECT
public:

  QString name = "AdaptiveThs";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadAdaptiveThs() { }

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

    QString Adaptive_Threshold = param_js_data["ComboBox"].toString();
    int blockSize = param_js_data["blockSize"].toInt();
    int c = param_js_data["c"].toInt();

    if(Adaptive_Threshold == "ADAPTIVE_THRESH_MEAN_C"){
      cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
      cv::adaptiveThreshold(mat_im,mat_im,255,cv::ADAPTIVE_THRESH_MEAN_C,cv::THRESH_BINARY,blockSize,c);
      cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);

      jso["Adaptive_Threshold_Method"] = Adaptive_Threshold;
      jso["blockSize"] = blockSize;
      jso["c"] = c;
    }
    else{
      cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
      cv::adaptiveThreshold(mat_im,mat_im,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C,cv::THRESH_BINARY,blockSize,c);
      cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);

      jso["Adaptive_Threshold_Method"] = Adaptive_Threshold;
      jso["blockSize"] = blockSize;
      jso["c"] = c;
    }



    payload_js_data[name] = jso;

  }


};

#endif // ThreadAdaptiveThs_HPP
