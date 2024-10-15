#ifndef ThreadHistogram_HPP
#define ThreadHistogram_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>


class ThreadHistogram : public QThread
{
  Q_OBJECT
public:

  QString name = "Histogram";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im;
  cv::Mat mat_im2;

  bool isHaveError = false;

  ThreadHistogram() { }

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

    bool histogram = param_js_data["histogram"].toBool();
    bool clahe = param_js_data["clahe"].toBool();
    int cliplimit = param_js_data["cliplimit"].toInt();
    int index = param_js_data["gridsize"].toInt();
    QString graph = param_js_data["graph"].toString();
    mat_im2 = mat_im.clone();

    if(histogram==true){

      cv::cvtColor(mat_im,mat_im,cv::COLOR_BGR2GRAY);
      cv::equalizeHist(mat_im,mat_im);

    }else if(clahe==true){
      int gridsize1[] = {8,16,32,64,128,256};
      int gridsize = gridsize1[index];
      jso["box"] = gridsize;
      jso["limit"] = cliplimit;
      cv::cvtColor(mat_im,mat_im,cv::COLOR_BGR2GRAY);
      cv::Ptr<cv::CLAHE> Clahe = cv::createCLAHE();
      Clahe->setClipLimit(cliplimit);
      Clahe->setTilesGridSize(cv::Size(gridsize, gridsize));
      Clahe->apply(mat_im, mat_im);


    }

    if (graph == "RGB color"){
      std::vector<cv::Mat> bgrChannels;
      cv::split(mat_im2, bgrChannels); // Split the image into B, G, R channels

      int histSize = 256; // Number of bins
      float range[] = { 0, 256 }; // Range of pixel values
      const float* histRange = { range };
      bool uniform = true; // Indicates if the histogram is uniform
      bool accumulate = false; // Indicates if the histogram is accumulated

      cv::Mat histB, histG, histR;
      // Calculate the histograms for each channel
      cv::calcHist(&bgrChannels[0], 1, nullptr, cv::Mat(), histB, 1, &histSize, &histRange, uniform, accumulate);
      cv::calcHist(&bgrChannels[1], 1, nullptr, cv::Mat(), histG, 1, &histSize, &histRange, uniform, accumulate);
      cv::calcHist(&bgrChannels[2], 1, nullptr, cv::Mat(), histR, 1, &histSize, &histRange, uniform, accumulate);

      // Plot the histograms
      int histWidth = 512;
      int histHeight = 400;
      cv::Mat histImage(histHeight, histWidth, CV_8UC3, cv::Scalar(255,255,255)); // Create a blank image for the histogram

      cv::normalize(histB, histB, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
      cv::normalize(histG, histG, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
      cv::normalize(histR, histR, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

      int binWidth = cvRound(static_cast<double>(histWidth) / histSize);
      // Draw the histograms
      for (int i = 1; i < histSize; i++)
      {
        cv::line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(histB.at<float>(i - 1))),
                 cv::Point(binWidth * i, histHeight - cvRound(histB.at<float>(i))), cv::Scalar(255, 0, 0), 2); // Blue channel
        cv::putText(histImage,"B channel,",cv::Point(10,30), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(255, 0, 0), 2.0);
        cv::line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(histG.at<float>(i - 1))),
                 cv::Point(binWidth * i, histHeight - cvRound(histG.at<float>(i))), cv::Scalar(0, 255, 0), 2); // Green channel
        cv::putText(histImage,"G channel,",cv::Point(150,30), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 255, 0), 2.0);
        cv::line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(histR.at<float>(i - 1))),
                 cv::Point(binWidth * i, histHeight - cvRound(histR.at<float>(i))), cv::Scalar(0, 0, 255), 2); // Red channel
        cv::putText(histImage,"R channel",cv::Point(290,30), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0, 0, 255), 2.0);
      }
      mat_im2 = histImage;

    }else if(graph == "HSV color"){
      std::vector<cv::Mat> hsvChannels;
      cv::cvtColor(mat_im2, mat_im2, cv::COLOR_BGR2HSV);
      cv::split(mat_im2, hsvChannels); // Split the image into B, G, R channels

      int histSize = 256; // Number of bins
      float range[] = { 0, 256 }; // Range of pixel values
      const float* histRange = { range };
      bool uniform = true; // Indicates if the histogram is uniform
      bool accumulate = false; // Indicates if the histogram is accumulated

      cv::Mat histH, histS, histV;
      // Calculate the histograms for each channel
      cv::calcHist(&hsvChannels[0], 1, nullptr, cv::Mat(), histH, 1, &histSize, &histRange, uniform, accumulate);
      cv::calcHist(&hsvChannels[1], 1, nullptr, cv::Mat(), histS, 1, &histSize, &histRange, uniform, accumulate);
      cv::calcHist(&hsvChannels[2], 1, nullptr, cv::Mat(), histV, 1, &histSize, &histRange, uniform, accumulate);

      // Plot the histograms
      int histWidth = 512;
      int histHeight = 400;
      cv::Mat histImage(histHeight, histWidth, CV_8UC3, cv::Scalar(255,255,255)); // Create a blank image for the histogram

      cv::normalize(histH, histH, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
      cv::normalize(histS, histS, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());
      cv::normalize(histV, histV, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

      int binWidth = cvRound(static_cast<double>(histWidth) / histSize);
      // Draw the histograms
      for (int i = 1; i < histSize; i++)
      {
        cv::line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(histH.at<float>(i - 1))),
                 cv::Point(binWidth * i, histHeight - cvRound(histH.at<float>(i))), cv::Scalar(172, 66, 255), 2); // H channel
        cv::putText(histImage,"H channel,",cv::Point(10,30), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(172, 66, 255), 2.0);
        cv::line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(histS.at<float>(i - 1))),
                 cv::Point(binWidth * i, histHeight - cvRound(histS.at<float>(i))), cv::Scalar(67,255,34), 2); // S channel
        cv::putText(histImage,"S channel,",cv::Point(150,30), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(67,255,34), 2.0);
        cv::line(histImage, cv::Point(binWidth * (i - 1), histHeight - cvRound(histV.at<float>(i - 1))),
                 cv::Point(binWidth * i, histHeight - cvRound(histV.at<float>(i))), cv::Scalar(255, 219, 66), 2); // V channel
        cv::putText(histImage,"V channel",cv::Point(290,30), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(255, 219, 66), 2.0);
      }
      mat_im2 = histImage;


    }else if(graph == "Histogram equalization"){
      cv::cvtColor(mat_im2,mat_im2,cv::COLOR_BGR2GRAY);
      cv::equalizeHist(mat_im2,mat_im2);
      // Calculate the histogram
      std::vector<int> hist(256, 0);
      for (int i = 0; i < mat_im2.rows; i++)
      {
        for (int j = 0; j < mat_im2.cols; j++)
        {
          int pixel_value = static_cast<int>(mat_im2.at<uchar>(i, j));
          hist[pixel_value]++;
        }
      }
      // Find the maximum frequency in the histogram
      int max_freq = *std::max_element(hist.begin(), hist.end());
      // Set up the histogram visualization parameters
      int hist_width = 512;
      int hist_height = 400;
      int bin_width = cvRound(static_cast<double>(hist_width) / hist.size());
      cv::Mat hist_image(hist_height, hist_width, CV_8UC3, cv::Scalar(255,255,255));
      // Draw the histogram
      for (int i = 0; i < hist.size(); i++)
      {
        int bar_height = cvRound(static_cast<double>(hist[i]) / max_freq * hist_height);
        cv::rectangle(hist_image, cv::Point(i * bin_width, hist_height - 1),
                      cv::Point((i + 1) * bin_width - 1, hist_height - bar_height),
                      cv::Scalar(0,0,0), -1);
      }
      mat_im2 = hist_image;

    }else if(graph=="Grayscale"){
      cv::cvtColor(mat_im2,mat_im2,cv::COLOR_BGR2GRAY);
      // Calculate the histogram
      std::vector<int> hist(256, 0);
      for (int i = 0; i < mat_im2.rows; i++)
      {
        for (int j = 0; j < mat_im2.cols; j++)
        {
          int pixel_value = static_cast<int>(mat_im2.at<uchar>(i, j));
          hist[pixel_value]++;
        }
      }
      // Find the maximum frequency in the histogram
      int max_freq = *std::max_element(hist.begin(), hist.end());
      // Set up the histogram visualization parameters
      int hist_width = 512;
      int hist_height = 400;
      int bin_width = cvRound(static_cast<double>(hist_width) / hist.size());
      cv::Mat hist_image(hist_height, hist_width, CV_8UC3, cv::Scalar(255,255,255));
      // Draw the histogram
      for (int i = 0; i < hist.size(); i++)
      {
        int bar_height = cvRound(static_cast<double>(hist[i]) / max_freq * hist_height);
        cv::rectangle(hist_image, cv::Point(i * bin_width, hist_height - 1),
                      cv::Point((i + 1) * bin_width - 1, hist_height - bar_height),
                      cv::Scalar(0,0,0), -1);
      }
      mat_im2 = hist_image;
    }else{
      int gridsize1[] = {8,16,32,64,128,256};
      int gridsize = gridsize1[index];
      jso["box"] = gridsize;
      jso["limit"] = cliplimit;
      cv::cvtColor(mat_im2,mat_im2,cv::COLOR_BGR2GRAY);
      cv::Ptr<cv::CLAHE> Clahe = cv::createCLAHE();
      Clahe->setClipLimit(cliplimit);
      Clahe->setTilesGridSize(cv::Size(gridsize, gridsize));
      Clahe->apply(mat_im2, mat_im2);
      // Calculate the histogram
      std::vector<int> hist(256, 0);
      for (int i = 0; i < mat_im2.rows; i++)
      {
        for (int j = 0; j < mat_im2.cols; j++)
        {
          int pixel_value = static_cast<int>(mat_im2.at<uchar>(i, j));
          hist[pixel_value]++;
        }
      }
      // Find the maximum frequency in the histogram
      int max_freq = *std::max_element(hist.begin(), hist.end());
      // Set up the histogram visualization parameters
      int hist_width = 512;
      int hist_height = 400;
      int bin_width = cvRound(static_cast<double>(hist_width) / hist.size());
      cv::Mat hist_image(hist_height, hist_width, CV_8UC3, cv::Scalar(255,255,255));
      // Draw the histogram
      for (int i = 0; i < hist.size(); i++)
      {
        int bar_height = cvRound(static_cast<double>(hist[i]) / max_freq * hist_height);
        cv::rectangle(hist_image, cv::Point(i * bin_width, hist_height - 1),
                      cv::Point((i + 1) * bin_width - 1, hist_height - bar_height),
                      cv::Scalar(0,0,0), -1);
      }
      mat_im2 = hist_image;

    }



    jso["msg"] = "Hello World";
    jso["ooo"] = "ddd";


    payload_js_data[name] = jso;

  }


};

#endif // ThreadHistogram_HPP
