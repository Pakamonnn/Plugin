#ifndef ThreadBinaryImage_HPP
#define ThreadBinaryImage_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>
#include <opencv2/opencv.hpp>


class ThreadBinaryImage : public QThread
{
  Q_OBJECT
public:

  QString name = "BinaryImage";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadBinaryImage() { }

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

    int threshold = param_js_data["Threshold"].toInt();
    QString comboBox = param_js_data["ComboBox"].toString();
    bool groupBox = param_js_data["GroupBox"].toBool();
    int threshold_Bar = param_js_data["Threshold_Bar"].toInt();

    if(groupBox == true){

      if(comboBox == "Binary"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold_Bar,255, cv::THRESH_BINARY);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "Invert_Binary"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold_Bar,255, cv::THRESH_BINARY_INV);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "TRUNC"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold_Bar,255, cv::THRESH_TRUNC);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "TOZERO"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold_Bar,255, cv::THRESH_TOZERO);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "Invert_TOZERO"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold_Bar,255, cv::THRESH_TOZERO_INV);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      jso["Threshold"] = threshold_Bar;

    }
    else{

      if(comboBox == "Binary"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold,255, cv::THRESH_BINARY);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "Invert_Binary"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold,255, cv::THRESH_BINARY_INV);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "TRUNC"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold,255, cv::THRESH_TRUNC);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "TOZERO"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold,255, cv::THRESH_TOZERO);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      if(comboBox == "Invert_TOZERO"){
        cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
        cv::threshold(mat_im,mat_im,threshold,255, cv::THRESH_TOZERO_INV);
        cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);
      }
      jso["Threshold"] = threshold;

    }

    jso["Thresholding_Method"] = comboBox;
    payload_js_data[name] = jso;

  }


};

#endif // ThreadBinaryImage_HPP
