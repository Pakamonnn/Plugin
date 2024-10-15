#ifndef ThreadMorphology_HPP
#define ThreadMorphology_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadMorphology : public QThread
{
  Q_OBJECT
public:

  QString name = "Morphology";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadMorphology() { }

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

    QString morphological = param_js_data["ComboBox"].toString();
    int kernel = param_js_data["Kernel"].toInt();
    int iterations = param_js_data["Iterations"].toInt();

    if(morphological=="Erosion"){

      cv::erode(mat_im, mat_im, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);

    }
    if(morphological=="Dilation"){

      cv::dilate(mat_im, mat_im, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);
    }
    if(morphological=="Opening"){

      cv::morphologyEx(mat_im, mat_im, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);

    }
    if(morphological=="Closing"){

      cv::morphologyEx(mat_im, mat_im, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);

    }
    if(morphological=="Gradient"){

      cv::morphologyEx(mat_im, mat_im, cv::MORPH_GRADIENT, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);

    }
    if(morphological=="TopHat"){

      cv::morphologyEx(mat_im, mat_im, cv::MORPH_TOPHAT, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);

    }
    if(morphological=="BlackHat"){

      cv::morphologyEx(mat_im, mat_im, cv::MORPH_BLACKHAT, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel, kernel)), cv::Point( -1, -1), iterations);

    }

    jso["Morphological_Method"] = morphological;
    jso["Morphological_Kernel"] = kernel;
    jso["Morphological_Iterations"] = iterations;


    payload_js_data[name] = jso;

  }


};

#endif // ThreadMorphology_HPP
