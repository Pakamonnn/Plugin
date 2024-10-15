 #ifndef ThreadMatching_HPP
#define ThreadMatching_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>

class ThreadMatching : public QThread
{
  Q_OBJECT
public:

  QString name = "Matching";
  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;
  //QStringList array_img;
  bool isUseImage = true;
  cv::Mat mat_im,mat_im2,res;

  bool isHaveError = false;

  ThreadMatching() { }

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
    QString img = param_js_data["img"].toString();
    double threshold = param_js_data["Threshold"].toDouble();
    //mat_im2 = mat_im.clone();
    cv::cvtColor(mat_im,mat_im2,cv::COLOR_BGR2GRAY);
    cv::Mat templates = cv::imread(img.toStdString(),0);

    //cv::Mat res_32f(mat_im2.rows - templates.rows + 1, mat_im2.cols - templates.cols + 1, CV_32FC1);
    cv::matchTemplate(mat_im2,templates,res,cv::TM_CCOEFF_NORMED);


    //res_32f.convertTo(res, CV_8U, 255.0);

    //int size = ((templates.cols + templates.rows) / 4) * 2 + 1; //force size to be odd
    //cv::adaptiveThreshold(res, res, 255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, size, -128);
//    int count = 0;
//    QString a ;



//    while (true)
//    {
//      double minval, maxval;
//      cv::Point minloc, maxloc;
//      cv::minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);
//      jso["minval"] = minval;
//      jso["maxval"] = maxval;

//      a.append(QString("%1").arg(minloc));
//      jso["array"]=a;
//      if (maxval >= threshold)
//      {
//                  cv::rectangle(mat_im, maxloc,cv::Point(maxloc.x + templates.cols, maxloc.y + templates.rows),CV_RGB(0,255,0), 2);
//                  cv::floodFill(res , maxloc, 0);
//                  count+=1;
//                  jso["count"] = count;


//      }
//      else
//        break;
//    }

//    jso["threshold"] = threshold;
//    jso["path"] = img;

//    //jso["ddd"] = cv::minMaxLoc(mat_im, &minval, &maxval, &minloc, &maxloc);


    payload_js_data[name] = jso;

  }


};

#endif // ThreadMatching_HPP
