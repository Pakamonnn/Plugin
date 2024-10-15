#ifndef ThreadResize_HPP
#define ThreadResize_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadResize : public QThread
{
  Q_OBJECT
public:

  QString name = "Resize";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;

  bool isHaveError = false;

  ThreadResize() { }

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

    int w = param_js_data["W"].toInt();
    int h = param_js_data["H"].toInt();
    QString w_str = QString::number(w);
    QString h_str = QString::number(h);
    bool checkbox = param_js_data["CheckBox"].toBool();

    if(checkbox){
      cv::Scalar clr(0, 0, 0);
      mat_im = resizeKeepAspectRatio(mat_im, cv::Size(w, h), clr);
      jso["Resize"] = "Resize to "+w_str+" x "+h_str+" and Padding";
    }
    else{
      cv::resize(mat_im, mat_im, cv::Size(w, h));
      jso["Resize"] = "Resize to "+w_str+" x "+h_str;
    }
    jso["w"] = w_str;
    jso["h"] = h_str;
    payload_js_data[name] = jso;

  }
  cv::Mat resizeKeepAspectRatio(const cv::Mat &input, const cv::Size &dstSize, const cv::Scalar &bgcolor)
  {
      cv::Mat output;

      double h1 = dstSize.width * (input.rows/(double)input.cols);
      double w2 = dstSize.height * (input.cols/(double)input.rows);
      if( h1 <= dstSize.height) {
          cv::resize( input, output, cv::Size(dstSize.width, h1));
      } else {
          cv::resize( input, output, cv::Size(w2, dstSize.height));
      }

      int top = (dstSize.height-output.rows) / 2;
      int down = (dstSize.height-output.rows+1) / 2;
      int left = (dstSize.width - output.cols) / 2;
      int right = (dstSize.width - output.cols+1) / 2;

      cv::copyMakeBorder(output, output, top, down, left, right, cv::BORDER_CONSTANT, bgcolor );

      return output;
  }


};

#endif // ThreadResize_HPP
