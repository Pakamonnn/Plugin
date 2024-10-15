#ifndef ThreadFlip_HPP
#define ThreadFlip_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadFlip : public QThread
{
  Q_OBJECT
public:

  QString name = "Flip";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadFlip() { }

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

    QString combo_box = param_js_data["ComboBox"].toString();

    if(combo_box == "Vertical_Flip")
    {
        cv::flip(mat_im,mat_im,0);
    }
    if(combo_box == "Horizontal_Flip")
    {
        cv::flip(mat_im,mat_im,1);
    }
    if(combo_box == "Vertical & Horizontal_Flip")
    {
        cv::flip(mat_im,mat_im,-1);
    }

    jso["Flip"] = combo_box;


    payload_js_data[name] = jso;

  }


};

#endif // ThreadFlip_HPP
