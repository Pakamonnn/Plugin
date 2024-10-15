#ifndef ThreadRotation_HPP
#define ThreadRotation_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadRotation : public QThread
{
  Q_OBJECT
public:

  QString name = "Rotation";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadRotation() { }

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

    int degree = param_js_data["Degree"].toInt();
    int degree_Bar = param_js_data["SliderBar"].toInt();
    bool checkbox = param_js_data["CheckBox"].toBool();

    if(checkbox){
      mat_im = rotateKeepScale(mat_im,degree_Bar);
      jso["Rotation_Degree"] = degree_Bar;
    }
    else{
      mat_im = rotateKeepScale(mat_im,degree);
      jso["Rotation_Degree"] = degree;
    }





    payload_js_data[name] = jso;

  }
  cv::Mat rotateKeepScale(const cv::Mat &input,int degree)
  {
    cv::Mat output;


    cv::Point2f center((input.cols-1)/2.0, (input.rows-1)/2.0);
    cv::Mat rot = cv::getRotationMatrix2D(center, degree, 1.0);
    // determine bounding rectangle, center not relevant
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), input.size(), degree).boundingRect2f();
    // adjust transformation matrix
    rot.at<double>(0,2) += bbox.width/2.0 - input.cols/2.0;
    rot.at<double>(1,2) += bbox.height/2.0 - input.rows/2.0;

    cv::warpAffine(input, output, rot, bbox.size());

    return output;
  }


};

#endif // ThreadRotation_HPP
