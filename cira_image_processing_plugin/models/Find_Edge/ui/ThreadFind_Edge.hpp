#ifndef ThreadFind_Edge_HPP
#define ThreadFind_Edge_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadFind_Edge : public QThread
{
  Q_OBJECT
public:

  QString name = "Find_Edge";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im,mat_im2;

  bool isHaveError = false;

  ThreadFind_Edge() { }

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


    QString gradient_filter = param_js_data["ComboBox"].toString();
    int kernel = param_js_data["Kernel"].toInt();

    if(gradient_filter == "Sobel_X"){
      cv::cvtColor(mat_im,mat_im,cv::COLOR_BGR2GRAY);
      cv::Sobel(mat_im,mat_im,CV_8U,1,0,kernel);
      mat_im2 = mat_im.clone();
      cv::cvtColor(mat_im,mat_im,cv::COLOR_GRAY2BGR);
    }
    if(gradient_filter == "Sobel_Y"){
      cv::cvtColor(mat_im,mat_im,cv::COLOR_BGR2GRAY);
      cv::Sobel(mat_im,mat_im,CV_8U,0,1,kernel);
      mat_im2 = mat_im.clone();
      cv::cvtColor(mat_im,mat_im,cv::COLOR_GRAY2BGR);
    }
    if(gradient_filter == "Sobel_XY"){
      cv::cvtColor(mat_im,mat_im,cv::COLOR_BGR2GRAY);
      cv::Sobel(mat_im,mat_im,CV_8U,1,1,kernel);
      mat_im2 = mat_im.clone();
      cv::cvtColor(mat_im,mat_im,cv::COLOR_GRAY2BGR);
    }
    if(gradient_filter == "Laplacian"){
      cv::cvtColor(mat_im,mat_im,cv::COLOR_BGR2GRAY);
      cv::Laplacian(mat_im,mat_im,CV_8U,kernel);
      mat_im2 = mat_im.clone();
      cv::cvtColor(mat_im,mat_im,cv::COLOR_GRAY2BGR);
    }

    //Let’s compute the variance:
    cv::Scalar mean, stddev;
    cv::meanStdDev(mat_im2, mean, stddev, cv::Mat() );
    double variance = stddev.val[0] * stddev.val[0];

    jso["variance"] = variance;

    payload_js_data[name] = jso;

  }


};

#endif // ThreadFind_Edge_HPP
