#ifndef ThreadCanny_Edge_HPP
#define ThreadCanny_Edge_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadCanny_Edge : public QThread
{
  Q_OBJECT
public:

  QString name = "Canny_Edge";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im,mat_im2;

  bool isHaveError = false;

  ThreadCanny_Edge() { }

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

    int lower_threshold = param_js_data["Lower"].toInt();
    int upper_threshold = param_js_data["Upper"].toInt();
    int kernel = param_js_data["Kernel"].toInt();

    cv::cvtColor(mat_im,mat_im, cv::COLOR_BGR2GRAY);
    cv::Canny(mat_im,mat_im,lower_threshold,upper_threshold,kernel);
    mat_im2 = mat_im.clone();
    cv::cvtColor(mat_im,mat_im, cv::COLOR_GRAY2BGR);

    //Let’s compute the variance:
    cv::Scalar mean, stddev;
    cv::meanStdDev(mat_im2, mean, stddev, cv::Mat() );
    double variance = stddev.val[0] * stddev.val[0];

    jso["variance"] = variance;

    payload_js_data[name] = jso;

  }


};

#endif // ThreadCanny_Edge_HPP
