#ifndef ThreadBlur_HPP
#define ThreadBlur_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadBlur : public QThread
{
  Q_OBJECT
public:

  QString name = "Blur";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im, mat_im2;

  bool isHaveError = false;

  ThreadBlur() { }

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

    int kernel = param_js_data["Kernel"].toInt();
    QString comboBox = param_js_data["ComboBox"].toString();
    //bool groupBox = param_js_data["GroupBox"].toBool();
    //int threshold_Bar = param_js_data["Threshold_Bar"].toInt();

    if(comboBox == "filter2D"){
      cv::Mat kernel2 = cv::Mat::ones(kernel,kernel, CV_64F);
      kernel2 = kernel2 / 25;
      cv::filter2D(mat_im, mat_im, -1 , kernel2, cv::Point(-1,-1), 0, 4);

    }
    if(comboBox == "Average_Blur"){
      cv::blur(mat_im, mat_im, cv::Size(kernel,kernel));

    }
    if(comboBox == "Gaussian_Blur"){
      cv::GaussianBlur(mat_im, mat_im, cv::Size(kernel,kernel), 0, 0);

    }
    if(comboBox == "Median_Blur"){
      cv::medianBlur(mat_im, mat_im, kernel);

    }
    if(comboBox == "BilateralFilter"){
      //mat_im2 = mat_im.clone();
      cv::bilateralFilter(mat_im, mat_im, kernel, 75, 75);

    }
    jso["Blurring_Method"] = comboBox;
    jso["Blurring_Kernel"] = kernel;



    payload_js_data[name] = jso;

  }


};

#endif // ThreadBlur_HPP
