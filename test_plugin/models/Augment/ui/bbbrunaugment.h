#ifndef BBBRUNAUGMENT_H
#define BBBRUNAUGMENT_H

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>
#include <QDir>
#include <QJsonArray>
#include <jsoncpp/json/json.h>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <QRandomGenerator>
#include <ros/rate.h>
#include <QTextStream>
#include <QJsonArray>
#include <QMessageBox>
#include <QObject>

class bbbrunaugment
{
public:
  bbbrunaugment(const QJsonObject& params);

private:
  QString name = "Augment";
  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;
  QJsonObject gt_data;
  QJsonArray gt_all_data;
  QJsonObject obj_label_defect;
  QJsonDocument jsonDoc;

  cv::Mat mat_im;
  std::vector<std::string> name_bg, path_bg;
  std::vector<std::vector<int>> clicked_X, clicked_Y;
  int countt, xx ,yy;
  int bg_count;
  int x_position, y_position;
  QString name_label;
  bool Flip_input, Gaussian_input;
  cv::Mat image, rotated, overlay_rotated, overlay_image_n, overlaydone;
  QString strRotated, strScale, strWeightX, strWeightY, strFlip, strGaussian, strShearX, strShearY;
  int step_degree, ScalingInput, WeightXInput, WeightYInput;
  double shearX_angle_start, shearX_step_angle, shear_Xangle, shearY_angle_start, shearY_step_angle, shear_Yangle;
  int Kernel_input;
  QString PathSave, positionconfiguration;
  int bg_index;
  QString typeNoise, strSAVEPositionconfiguration;

  bool wait_n;

  std::tuple<cv::Mat, int, int, cv::Mat, std::string> overlay_sp(cv::Mat image) {
    cv::Mat img_bg = cv::imread(path_bg[bg_index]);
    QString qname = QString::fromStdString(name_bg[bg_index]);
    cv::Mat img_bgN(img_bg.size(), CV_MAKE_TYPE(img_bg.depth(), 4));
    int from_to[] = { 0,0, 1,1, 2,2, -1,3 };
    cv::mixChannels(&img_bg,1,&img_bgN,1,from_to,4);
    QString filename_bg = QFileInfo(qname).baseName();
    int center_x = image.cols / 2;
    int center_y = image.rows / 2;
    int xx = x_position - center_x;
    int yy = y_position - center_y;
    std::string name_bg = filename_bg.toStdString() + "_";

    if (yy < 0 || xx < 0 || yy + image.rows > img_bgN.rows || xx + image.cols > img_bgN.cols) {
      for (int i = 100; i > 0; --i) {
            double scale_factor = static_cast<double>(i) / 100.0;
            cv::Size dim(static_cast<int>(image.cols * scale_factor), static_cast<int>(image.rows * scale_factor));
            cv::Mat image2;
            std::cout << i << " " << dim << std::endl;
            cv::resize(image, image2, dim);
            center_x = image2.cols / 2;
            center_y = image2.rows / 2;
            xx = x_position - center_x;
            yy = y_position - center_y;
            if (yy < 0 || xx < 0 || yy + image2.rows > img_bgN.rows || xx + image2.cols > img_bgN.cols) {
                //std::cout <<  " dddd " << std::endl;
                //continue;
            } else {
                //std::cout <<  " pppp " << std::endl;
                cv::Mat overlayROI = img_bgN(cv::Rect(xx, yy, image2.cols, image2.rows));
                image2.copyTo(overlayROI, image2(cv::Rect(0, 0, image2.cols, image2.rows)));
                //cv::imshow("img_bgN", img_bgN);
                //cv::imshow("noise",image2);
                //cv::waitKey(1000);
                return std::make_tuple(img_bgN, xx, yy, image2, name_bg);
            }
        }
    } else {
        cv::Mat overlayROI = img_bgN(cv::Rect(xx, yy, image.cols, image.rows));
        image.copyTo(overlayROI, image(cv::Rect(0, 0, image.cols, image.rows)));
        //cv::imshow("img_bgN", img_bgN);
        //cv::imshow("noise",image);
        //cv::waitKey(1000);
        return std::make_tuple(img_bgN, xx, yy, image, name_bg);
    }

  }

  QJsonObject writeGT(cv::Mat image, const std::string& name_img) {
    cv::Mat overlaydone, image2, img_g, binary, closing;
    std::string name_bg;
    int xxx, yyy;
    std::tie(overlaydone, xxx, yyy, image2, name_bg) = overlay_sp(image);

    cv::cvtColor(image2, img_g, cv::COLOR_BGR2GRAY);
    cv::threshold(img_g, binary, 0, 255, cv::THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(binary, closing, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 1);

    std::vector<std::vector<cv::Point>> contours_edges;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(closing, contours_edges, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

    QJsonArray obj_label_defect_array;

    cv::Mat image_c = overlaydone.clone();
    for (size_t cnt = 0; cnt < contours_edges.size(); ++cnt) {
        cv::Rect rect = cv::boundingRect(contours_edges[cnt]);
        int x = rect.x;
        int y = rect.y;
        int w = rect.width;
        int h = rect.height;
        int x_start = x + xxx;
        int y_start = y + yyy;
        int x_stop = x + xxx + w;
        int y_stop = y + yyy + h;
        int cx = (x_start + x_stop) / 2;
        int cy = (y_start + y_stop) / 2;
        cv::rectangle(image_c, cv::Point(x_start, y_start), cv::Point(x_stop, y_stop), cv::Scalar(0, 255, 0), 2);
        cv::circle(image_c, cv::Point(x_start, y_start), 1, cv::Scalar(255,0,0),3);
        cv::circle(image_c, cv::Point(x_stop, y_stop), 1, cv::Scalar(0,0,255),3);

        QJsonObject obj_label_defect;
            obj_label_defect["label"] = name_label;
            obj_label_defect["center"] = QString::fromStdString(std::to_string(cx) + "," + std::to_string(cy));
            obj_label_defect["bbox"] = QString::fromStdString(std::to_string(x_start) + "," + std::to_string(y_start) + "," + std::to_string(w) + "," + std::to_string(h));

            obj_label_defect_array.append(obj_label_defect);
        }

    QJsonObject gt_data;
    gt_data["filename"] = QString::fromStdString(name_img);
    gt_data["obj_array"] = obj_label_defect_array;
    gt_all_data.append(gt_data);

    return  gt_data ;
 }

  class MouseHandler {
  public:
    static std::vector<int> x_all;
    static std::vector<int> y_all;
    static int numberOfPosition, numberClicked;
    static int bg_countRef;
    static cv::Mat new_bg;
    static int position_n;
    bool wait_n = false;

    static void click(int event, int x, int y, int flags, void* userdata) {
        MouseHandler* instance = static_cast<MouseHandler*>(userdata);
        if (instance)
            instance->handleClick(event, x, y);
    }

    void handleClick(int event, int x, int y) {
      if (event == cv::EVENT_LBUTTONDOWN) {
            int clicked_x = x * 2;
            int clicked_y = y * 2;
            std::cout << "    x : "<< clicked_x << std::endl;
            std::cout << "    y : "<< clicked_y << std::endl;
            x_all.push_back(clicked_x);
            y_all.push_back(clicked_y);
            cv::circle(new_bg, cv::Point(x,y), 3, cv::Scalar(0,255,0), 2);
            cv::imshow("Click position x y", new_bg);
            std::cout << "numberOfPosition : " << numberOfPosition << std::endl << "x_all :: " << x_all.size() << std::endl;
            int positionCouunt = (bg_countRef+1)*numberClicked;
            std::cout << "bg_countRef : " << bg_countRef << std::endl << "numberClicked :: " << numberClicked << std::endl;
            if(x_all.size() == numberClicked || x_all.size() == positionCouunt){
              position_n = 1;
              return;
            }
        }
    }
  };

  cv::Mat rotate_image(cv::Mat image, double degree){
    int height = image.rows;
    int width = image.cols;
    cv::Point2f center(width/2.0, height/2.0);
    cv::Mat rotation_metrix = cv::getRotationMatrix2D(center, degree, 1.0);
    cv::Mat rotaed_image;
    cv::warpAffine(image, rotaed_image, rotation_metrix, cv::Size(width,height));
    return rotaed_image;
  }

  cv::Mat Scaling_image(cv::Mat image, int scaling_input){
    int height = image.rows * scaling_input /100;
    int width = image.cols * scaling_input /100;
    cv::Size new_size(width, height);
    cv::Mat image_scaling;
    cv::resize(image, image_scaling, new_size);
    return image_scaling;
  }

  cv::Mat WeightX_image(cv::Mat image, int WeightX_input){
    int height = image.rows;
    int width = image.cols * WeightX_input /100;
    cv::Size new_size(width, height);
    cv::Mat image_weightX;
    cv::resize(image, image_weightX, new_size);
    return image_weightX;
  }

  cv::Mat WeightY_image(cv::Mat image, int WeightY_input){
    int height = image.rows * WeightY_input /100;
    int width = image.cols;
    cv::Size new_size(width, height);
    cv::Mat image_weightY;
    cv::resize(image, image_weightY, new_size);
    return image_weightY;
  }

  cv::Mat Flip_image(cv::Mat image){
    cv::Mat image_flip;
    if(Flip_input == true){
      cv::flip(image, image_flip, 1);
      strFlip = "_Flip";
    }else{
      image_flip = image;
      strFlip = "_NonFlip";
    }
    return image_flip;
  }

  cv::Mat Gaussian_image(cv::Mat image){
    cv::Mat image_gaussian;
    if(Gaussian_input == true){
      cv::Size kernelsize(Kernel_input,Kernel_input);
      cv::GaussianBlur(image, image_gaussian, kernelsize, 0);
      strGaussian = "_GaussianKernel" + QString::number(Kernel_input);
    }else{
      image_gaussian = image;
      strGaussian = "_NonGaussian";
    }
    return image_gaussian;
  }

  cv::Mat ShearX_image(cv::Mat image, double shear_angle){
    double shear_angle_rad = shear_angle * CV_PI / 180;
    cv::Mat matrix = (cv::Mat_<float>(2,3) << 1, shear_angle_rad, 0, 0, 1, 0);
    int h = image.rows;
    int w = image.cols + static_cast<int>(h * shear_angle_rad);
    cv::Mat sheared_image;
    cv::warpAffine(image, sheared_image, matrix, cv::Size(w, h));
    return sheared_image;
  }

  cv::Mat ShearY_image(cv::Mat image, double shear_angle){
    double shear_angle_rad = shear_angle * CV_PI / 180;
    cv::Mat matrix = (cv::Mat_<float>(2,3) << 1, 0, 0, shear_angle_rad, 1, 0);
    int w = image.cols;
    int h = image.rows + static_cast<int>(w * shear_angle_rad);
    cv::Mat sheared_image;
    cv::warpAffine(image, sheared_image, matrix, cv::Size(w, h));
    return sheared_image;
  }

  std::tuple<cv::Mat, QString> process_Scaling_Image(cv::Mat image, QString name_img){
    cv::Mat imageScaling = Scaling_image(image, ScalingInput);
    cv::Mat overlay_imageScaling;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageScaling, xx, yy, image, name_bg) = overlay_sp(imageScaling);
    QString name_n2 = name_img.left(name_img.length() - 4) + strScale + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageScaling);
    writeGT(imageScaling, name_n2.toStdString());
    return std::make_tuple(imageScaling, name_n2);
  }

  std::tuple<cv::Mat, QString> process_WeightX_Image(cv::Mat image, QString name_img){
    cv::Mat imageWeightX = WeightX_image(image, WeightXInput);
    cv::Mat overlay_imageWeightX;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageWeightX, xx, yy, image, name_bg) = overlay_sp(imageWeightX);
    QString name_n2 = name_img.left(name_img.length() - 4) + strWeightX + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageWeightX);
    writeGT(imageWeightX, name_n2.toStdString());
    return std::make_tuple(imageWeightX, name_n2);
  }

  std::tuple<cv::Mat, QString> process_WeightY_Image(cv::Mat image, QString name_img){
    cv::Mat imageWeightY = WeightY_image(image, WeightYInput);
    cv::Mat overlay_imageWeightY;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageWeightY, xx, yy, image, name_bg) = overlay_sp(imageWeightY);
    QString name_n2 = name_img.left(name_img.length() - 4) + strWeightY + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageWeightY);
    writeGT(imageWeightY, name_n2.toStdString());
    return std::make_tuple(imageWeightY, name_n2);
  }

  std::tuple<cv::Mat, QString> process_Flip_Image(cv::Mat image, QString name_img){
    cv::Mat imageFlip = Flip_image(image);
    cv::Mat overlay_imageFlip;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageFlip, xx, yy, image, name_bg) = overlay_sp(imageFlip);
    QString name_n2 = name_img.left(name_img.length() - 4) + strFlip + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageFlip);
    writeGT(imageFlip, name_n2.toStdString());
    return std::make_tuple(imageFlip, name_n2);
  }

  std::tuple<cv::Mat, QString> process_Gaussian_Image(cv::Mat image, QString name_img){
    cv::Mat imageGaussian = Gaussian_image(image);
    cv::Mat overlay_imageGaussian;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageGaussian, xx, yy, image, name_bg) = overlay_sp(imageGaussian);
    QString name_n2 = name_img.left(name_img.length() - 4) + strGaussian + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageGaussian);
    writeGT(imageGaussian, name_n2.toStdString());
    return std::make_tuple(imageGaussian, name_n2);
  }

  std::tuple<cv::Mat, QString> process_ShearX_Image(cv::Mat image, QString name_img, double shear_angle){
    cv::Mat imageShearX = ShearX_image(image, shear_angle);
    cv::Mat overlay_imageShearX;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageShearX, xx, yy, image, name_bg) = overlay_sp(imageShearX);
    if(shear_angle == 0.0){
      strShearX = "_NonShearX";
    }else{
      int angle = int(shear_angle);
      strShearX = "_ShearX_" + QString::fromStdString(std::to_string(angle));
    }
    QString name_n2 = name_img.left(name_img.length() - 4) + strShearX + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageShearX);
    writeGT(imageShearX, name_n2.toStdString());
    return std::make_tuple(imageShearX, name_n2);
  }

  std::tuple<cv::Mat, QString> process_ShearY_Image(cv::Mat image, QString name_img, double shear_angle){
    cv::Mat imageShearY = ShearY_image(image, shear_angle);
    cv::Mat overlay_imageShearY;
    int xx, yy;
    std::string name_bg;
    std::tie(overlay_imageShearY, xx, yy, image, name_bg) = overlay_sp(imageShearY);
    if(shear_Yangle == 0.0){
      strShearX = "_NonShearY";
    }else{
      int angle = int(shear_Yangle);
      strShearY = "_ShearY_" + QString::fromStdString(std::to_string(angle));
    }
    QString name_n2 = name_img.left(name_img.length() - 4) + strShearY + QStringLiteral(".jpg");
    QString path_nameOverlay = PathSave + name_n2;
    cv::imwrite(path_nameOverlay.toStdString(), overlay_imageShearY);
    writeGT(imageShearY, name_n2.toStdString());
    return std::make_tuple(imageShearY, name_n2);
  }

};

#endif // BBBRUNAUGMENT_H
