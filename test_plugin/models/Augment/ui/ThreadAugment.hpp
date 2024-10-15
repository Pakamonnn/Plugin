#ifndef ThreadAugment_HPP
#define ThreadAugment_HPP

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

class ThreadAugment : public QThread

{
  Q_OBJECT
public:

  QString name = "Augment";
  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;
  QJsonObject gt_data;
  QJsonArray gt_all_data;
  QJsonObject obj_label_defect;
  QJsonDocument jsonDoc;

  bool isUseImage = false; //dont use false
  cv::Mat mat_im;
  cv::Mat image_out;
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
  //std::vector<int> randomX_array, randomY_array;
  int bg_index;
  QString typeNoise, strSAVEPositionconfiguration;
  bool *wait;

  bool isHaveError = false;

  ThreadAugment() { }

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
    bool *wait;
    static void click(int event, int x, int y, int flags, void* userdata) {
        MouseHandler* instance = static_cast<MouseHandler*>(userdata);
        if (instance)
            instance->handleClick(event, x, y);
    }
    static cv::Mat new_bg;
    static int position_n;
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

  void run() {

    isHaveError = false;
    output_js_data = QJsonObject();
    QJsonObject jso;
//dont use image from bf
    /*if(isUseImage) {
      if(mat_im.empty()) {
        jso["error"] = "No image";
        output_js_data[name] = jso;
        isHaveError = true;
        return;
      }
    }*/

    //********* run here **************

    typeNoise = param_js_data["typeNoise"].toString();
    positionconfiguration = param_js_data["positionconfiguration"].toString();
    MouseHandler::numberOfPosition = param_js_data["NumberOfPosition"].toInt();
    int maxX, maxY, minX, minY;
    std::vector<int> X_inBG, Y_inBG;
    std::vector<std::vector<int>> x_alls, y_alls;

    bool valueSpinBox = param_js_data["valueSpinBox"].toBool();
    int step_degree_input, Scaling_input, WeightX_input, WeightY_input, ShearX_input, ShearY_input, limitShearX, limitShearY;
    if(valueSpinBox == false){ //HorizontalSlider
      step_degree_input = param_js_data["RotationSlider"].toInt();
      Scaling_input = param_js_data["ScalingSlider"].toInt();
      WeightX_input = param_js_data["WeightXSlider"].toInt();
      WeightY_input = param_js_data["WeightYSlider"].toInt();
      ShearX_input = param_js_data["ShearXSlider"].toInt();
      ShearY_input = param_js_data["ShearYSlider"].toInt();
      limitShearX = param_js_data["limitShearXSlider"].toInt();
      limitShearY = param_js_data["limitShearYSlider"].toInt();
    }else{ //Spinbox
      step_degree_input = param_js_data["RotationSpibox"].toInt();
      Scaling_input = param_js_data["ScalingSpinbox"].toInt();
      WeightX_input = param_js_data["WeightXSpinbox"].toInt();
      WeightY_input = param_js_data["WeightYSpinbox"].toInt();
      ShearX_input = param_js_data["ShearXSpinbox"].toInt();
      ShearY_input = param_js_data["ShearYSpinbox"].toInt();
      limitShearX = param_js_data["limitShearX"].toInt();
      limitShearY = param_js_data["limitShearY"].toInt();
    }
    Kernel_input = param_js_data["Kernel"].toInt();
    Flip_input = param_js_data["Flip"].toBool();
    Gaussian_input = param_js_data["Gaussian"].toBool();
    bool multiOutput = param_js_data["Multioutput"].toBool();
    QString label_1 = param_js_data["Label"].toString();
    QString nameGT = param_js_data["nameGT"].toString();
    QString NoiseFolder = param_js_data["NoiseFolder"].toString();
    QString bgFolder = param_js_data["BakgroungFolder"].toString();
    QString pathSave = param_js_data["PathSave"].toString();
    PathSave = pathSave + "/";
    name_label = label_1;
    bool checkRound2 = param_js_data["CheckRound2"].toBool();
    QString fileGTname, FinalnameGT;
    if(checkRound2 == true){
      fileGTname = PathSave + "2ndgt" + QStringLiteral(".gt");
      FinalnameGT = PathSave + nameGT + QStringLiteral(".gt");
    }else{
      fileGTname = PathSave + nameGT + QStringLiteral(".gt");

    }

    MouseHandler mouseHandler;
    QString gt1File = param_js_data["gtR1file"].toString();

    if(step_degree_input == 0){
      step_degree = 360;
    }else{
      step_degree = step_degree_input;
    }

    if(Scaling_input < 0){
      ScalingInput = 100 + Scaling_input;
      strScale = "_ReduceScale_" + QString::number(-Scaling_input);
    }else if (Scaling_input > 0){
      ScalingInput = 100 + Scaling_input;
      strScale = "_IncreaseScale_" + QString::number(Scaling_input);
    }else {
      ScalingInput = 100;
      strScale = "_NonScale";
    }

    if(WeightX_input < 0){
      WeightXInput = 100 + WeightX_input;
      strWeightX = "_ReduceX_" + QString::number(-WeightX_input);
    }else if(WeightX_input > 0){
      WeightXInput = 100 + WeightX_input;
      strWeightX = "_IncreaseX_" + QString::number(WeightX_input);
    }else{
      WeightXInput = 100;
      strWeightX = "_NonX";
    }

    if(WeightY_input < 0){
      WeightYInput = 100 + WeightY_input;
      strWeightY = "_ReduceY_" + QString::number(-WeightY_input);
    }else if(WeightY_input > 0){
      WeightYInput = 100 + WeightY_input;
      strWeightY = "_IncreaseY_" + QString::number(WeightY_input);
    }else{
      WeightYInput = 100;
      strWeightY = "_NonY";
    }

    if(ShearX_input == 0){
      shearX_angle_start = 0;
      shearX_step_angle = 181;
    }else{
      shearX_step_angle = ShearX_input;
      if(multiOutput == false){ //one output
        shearX_angle_start = 0;
      }else{
        shearX_angle_start = shearX_step_angle;
      }
    }

    if(ShearY_input == 0){
      shearY_angle_start = 0;
      shearY_step_angle = 181;
    }else{
      shearY_step_angle = ShearY_input;
      if(multiOutput == false){
        shearY_angle_start = 0;
      }else{
        shearY_angle_start = shearY_step_angle;
      }
    }
//....Read Backgorung in floder and save in array....
    QDir bg(bgFolder);
    QFileInfoList fileListBG = bg.entryInfoList();
    for(int fbg = 0; fbg <fileListBG.size(); fbg++){
      QString fileNameBG = fileListBG.at(fbg).fileName();
      if(fileNameBG.endsWith(".png", Qt::CaseInsensitive) || fileNameBG.endsWith(".jpg", Qt::CaseInsensitive)
         || fileNameBG.endsWith(".bmp", Qt::CaseInsensitive)){
        QString path_imgbg = fileListBG.at(fbg).absoluteFilePath();
        QString fileNamebg = fileNameBG.left(fileNameBG.lastIndexOf('.'));
        name_bg.push_back(fileNamebg.toStdString());
        path_bg.push_back(path_imgbg.toStdString());        
      }
    }

    cv::namedWindow("Click position x y",1);
    cv::setMouseCallback("Click position x y", MouseHandler::click, &mouseHandler);
    QRandomGenerator randomGenerator;

//....Read path background in array and click positon
    for(bg_count = 0 ; bg_count < path_bg.size(); bg_count++){
      std::cout << "path_bg.size :: " << path_bg.size() << std::endl;
      MouseHandler::bg_countRef = bg_count;
      cv::Mat bg = cv::imread(path_bg[bg_count]);

      if(positionconfiguration == "Random position all area"){
        strSAVEPositionconfiguration = "RandomALL_";
        int lowerX = 0;
        int lowerY = 0;
        int highestX = bg.cols;
        int highestY = bg.rows;
        for(int num = 0; num < MouseHandler::numberOfPosition; num++){
          int randomX = randomGenerator.bounded(lowerX, highestX);
          int randomY = randomGenerator.bounded(lowerY, highestY);
          X_inBG.push_back(randomX);
          Y_inBG.push_back(randomY);
        }
        clicked_X.push_back(X_inBG);
        X_inBG.clear();
        clicked_Y.push_back(Y_inBG);
        Y_inBG.clear();

      }else if(positionconfiguration == "Random position in ROI"){
        strSAVEPositionconfiguration = "RandomROI_";

        MouseHandler::numberClicked = 4;
        int height_bg = bg.rows /2;
        int width_bg = bg.cols /2;
        cv::Size new_size(width_bg, height_bg);
        cv::resize(bg, MouseHandler::new_bg, new_size);
        cv::imshow("Click position x y", MouseHandler::new_bg);
        std::cout << " clicking clicking " << std::endl;
        *wait = true; //pointer value to connect
        ros::Rate rate(25);
        while(*wait){
          int key = cv::waitKey(10);
          if(key == 13 || MouseHandler::position_n == 1){ //"Enter" to next BG image
            MouseHandler::position_n = 0;
            break;

          }
          rate.sleep();
        }
        x_alls.push_back(MouseHandler::x_all);
        MouseHandler::x_all.clear();
        y_alls.push_back(MouseHandler::y_all);
        MouseHandler::y_all.clear();
        std::cout << "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII" << std::endl;

      }else{ //Multi + One position
        strSAVEPositionconfiguration = "Multi_";
        if(positionconfiguration == "One position"){
          strSAVEPositionconfiguration = "One_";
          MouseHandler::numberOfPosition = 1;
          MouseHandler::numberClicked = MouseHandler::numberOfPosition;
        }
        MouseHandler::numberClicked = MouseHandler::numberOfPosition;
        int height_bg = bg.rows /2;
        int width_bg = bg.cols /2;
        cv::Size new_size(width_bg, height_bg);
        cv::resize(bg, MouseHandler::new_bg, new_size);
        cv::imshow("Click position x y", MouseHandler::new_bg);
        std::cout << " clicking clicking " << std::endl;
        *wait = true; //pointer value to connect
        ros::Rate rate(25);
        while(*wait){
          int key = cv::waitKey(10);
          //if(key>0)
          if(key == 13 || MouseHandler::position_n == 1){ //"Enter" to next BG image
            MouseHandler::position_n = 0;
            break;

          }
          rate.sleep();
        }
        clicked_X.push_back(MouseHandler::x_all);
        MouseHandler::x_all.clear();
        clicked_Y.push_back(MouseHandler::y_all);
        MouseHandler::y_all.clear();
      }
    }
    cv::destroyWindow("Click position x y");

//....Random position in select area....
    if(positionconfiguration == "Random position in ROI"){
      for(bg_index = 0; bg_index < x_alls.size(); bg_index++){

        auto valueX_Max = std::max_element(x_alls[bg_index].begin(), x_alls[bg_index].end());
        auto valueX_Min = std::min_element(x_alls[bg_index].begin(), x_alls[bg_index].end());
        auto valueY_Max = std::max_element(y_alls[bg_index].begin(), y_alls[bg_index].end());
        auto valueY_Min = std::min_element(y_alls[bg_index].begin(), y_alls[bg_index].end());
        maxX = *valueX_Max;
        minX = *valueX_Min;
        maxY = *valueY_Max;
        minY = *valueY_Min;

        for(int num = 0; num < MouseHandler::numberOfPosition; num++){
          int randomX = randomGenerator.bounded(minX, maxX);
          int randomY = randomGenerator.bounded(minY, maxY);
          std::cout << "randomX : " << randomX << "  randomY : " << randomY <<std::endl;
          X_inBG.push_back(randomX);
          Y_inBG.push_back(randomY);
        }
        clicked_X.push_back(X_inBG);
        X_inBG.clear();
        clicked_Y.push_back(Y_inBG);
        Y_inBG.clear();
      }
    }

    std::cout << ".........GENERATING........." << std::endl;

    for(bg_index = 0; bg_index < clicked_X.size(); bg_index++){
      std::cout << "bg_index : " << bg_index << std::endl;
      std::cout << "path_BG : " << path_bg[bg_index] << std::endl;
      for(int xy_index = 0; xy_index < clicked_X[bg_index].size(); xy_index++){
        //std::cout << "BG " << bg_index << " X " << xy_index << " : " << clicked_X[bg_index][xy_index] << std::endl;
        //std::cout << "BG " << bg_index << " Y " << xy_index << " : " << clicked_Y[bg_index][xy_index] << std::endl;
        x_position = clicked_X[bg_index][xy_index];
        y_position = clicked_Y[bg_index][xy_index];
        int xyIndex = xy_index + 1;
        QString position = strSAVEPositionconfiguration + "Position_" +  QString::fromStdString(std::to_string(xyIndex)) + "_";
        std::cout << "x_position  : " << x_position <<std::endl;
        std::cout << "y_position  : " << y_position <<std::endl;

        QDir dir(NoiseFolder);
        QFileInfoList fileListNoise = dir.entryInfoList();
        for (int f = 0; f < fileListNoise.size(); f++) {
            QString fileName = fileListNoise.at(f).fileName();
            if (fileName.endsWith(".png", Qt::CaseInsensitive) || fileName.endsWith(".jpg", Qt::CaseInsensitive)) {
                QString path_img = fileListNoise.at(f).absoluteFilePath();
                QString fileName1 = fileName.left(fileName.lastIndexOf('.'));
                //std::cout << path_img.toStdString() << std::endl;
                //std::cout << fileName1.toStdString() << std::endl;
                mat_im = cv::imread(path_img.toStdString(), cv::IMREAD_UNCHANGED);
                int width = mat_im.cols + 50;
                int height = mat_im.rows + 50;
                cv::Mat last_image(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                int paste_x = (width - mat_im.cols) / 2;
                int paste_y = (height - mat_im.rows) / 2;
                cv::Mat roi = last_image(cv::Rect(paste_x, paste_y, mat_im.cols, mat_im.rows));
                mat_im.copyTo(roi);

                std::string name_bg;
                for(int angle = step_degree; angle < 361; angle+=step_degree){
                  rotated = rotate_image(last_image, angle);
                  if(angle == 360){
                    strRotated = "_NonRotate";
                  }else{
                   strRotated = "_Rotate_" + QString::fromStdString(std::to_string(angle));
                  }
                  std::tie(overlay_rotated, xx, yy, image, name_bg) = overlay_sp(rotated);
                  std::cout << name_bg << std::endl;
                  QString name_rotated = QString::fromStdString(name_bg) + position + fileName1 + strRotated + QStringLiteral(".jpg");
                  QString path_nameOverlay = PathSave + name_rotated;

                  if(multiOutput == false){ //one Output
                    cv::Mat image_n = Scaling_image(rotated, ScalingInput);
                    image_n = WeightX_image(image_n, WeightXInput);
                    image_n = WeightY_image(image_n, WeightYInput);
                    image_n = Flip_image(image_n);
                    image_n = Gaussian_image(image_n);
                    for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                      image_n = ShearX_image(image_n, shear_Xangle);
                      if(shear_Xangle == 0.0){
                        strShearX = "_NonShearX";
                      }else{
                        int angle = int(shear_Xangle);
                        strShearX = "_ShearX_" + QString::fromStdString(std::to_string(angle));
                      }
                      for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                        image_n = ShearY_image(image_n, shear_Yangle);
                        if(shear_Yangle == 0.0){
                          strShearY = "_NonShearY";
                        }else{
                          int angle = int(shear_Yangle);
                          strShearY = "_ShearY_" + QString::fromStdString(std::to_string(angle));
                        }
                        std::tie(overlay_image_n, xx, yy, image, name_bg) = overlay_sp(image_n);
                        QString name_n = name_rotated.left(name_rotated.length() - 4) + strScale + strWeightX + strWeightY + strFlip + strGaussian
                            + strShearX + strShearY + QStringLiteral(".jpg");
                        QString path_nameOverlay_n = PathSave + name_n;
                        cv::imwrite(path_nameOverlay_n.toStdString(),overlay_image_n);
                        writeGT(image_n, name_n.toStdString());
                      }
                    }
                  }else{  //multi output
                    writeGT(rotated, name_rotated.toStdString());
                    cv::imwrite(path_nameOverlay.toStdString(),overlay_rotated);
                    cv::Mat img_Scaling, img_WeightX, img_WeightY, img_Flip, img_Gaussian, img_ShearX, img_ShearY, img_Scaling_WeightX, img_Scaling_WeightY, img_Scaling_Flip, img_Scaling_Gaussian, img_Scaling_ShearX, img_Scaling_ShearY,
                        img_WeightX_WeightY, img_Scaling_WeightX_WeightY, img_WeightX_Flip, img_Scaling_WeightX_Flip, img_WeightX_Gaussian, img_Scaling_WeightX_Gaussian, img_WeightX_ShearX, img_Scaling_WeightX_ShearX, img_WeightX_ShearY, img_Scaling_WeightX_ShearY,img_WeightY_Flip, img_Scaling_WeightY_Flip,
                        img_WeightX_WeightY_Flip, img_Scaling_WeightX_WeightY_Flip, img_WeightY_Gaussian, img_Scaling_WeightY_Gaussian, img_WeightX_WeightY_Gaussian, img_Scaling_WeightX_WeightY_Gaussian, img_WeightY_ShearX, img_Scaling_WeightY_ShearX, img_WeightX_WeightY_ShearX, img_Scaling_WeightX_WeightY_ShearX,
                        img_WeightY_ShearY, img_Scaling_WeightY_ShearY, img_WeightX_WeightY_ShearY, img_Scaling_WeightX_WeightY_ShearY, img_Flip_Gaussian, img_Scaling_Flip_Gaussian, img_WeightX_Flip_Gaussian, img_Scaling_WeightX_Flip_Gaussian, img_WeightY_Flip_Gaussian, img_Scaling_WeightY_Flip_Gaussian,
                        img_WeightX_WeightY_Flip_Gaussian, img_Scaling_WeightX_WeightY_Flip_Gaussian, img_Flip_ShearX, img_Scaling_Flip_ShearX, img_WeightX_Flip_ShearX, img_Scaling_WeightX_Flip_ShearX, img_WeightY_Flip_ShearX, img_Scaling_WeightY_Flip_ShearX, img_WeightX_WeightY_Flip_ShearX, img_Scaling_WeightX_WeightY_Flip_ShearX,
                        img_Flip_ShearY, img_Scaling_Flip_ShearY, img_WeightX_Flip_ShearY, img_Scaling_WeightX_Flip_ShearY, img_WeightY_Flip_ShearY, img_Scaling_WeightY_Flip_ShearY, img_WeightX_WeightY_Flip_ShearY, img_Scaling_WeightX_WeightY_Flip_ShearY, img_Gaussian_ShearX, img_Scaling_Gaussian_ShearX, img_WeightX_Gaussian_ShearX, img_Scaling_WeightX_Gaussian_ShearX, img_WeightY_Gaussian_ShearX,
                        img_Scaling_WeightY_Gaussian_ShearX, img_WeightX_WeightY_Gaussian_ShearX, img_Scaling_WeightX_WeightY_Gaussian_ShearX, img_Flip_Gaussian_ShearX, img_Scaling_Flip_Gaussian_ShearX, img_WeightX_Flip_Gaussian_ShearX, img_Scaling_WeightX_Flip_Gaussian_ShearX, img_WeightY_Flip_Gaussian_ShearX, img_Scaling_WeightY_Flip_Gaussian_ShearX,
                        img_WeightX_WeightY_Flip_Gaussian_ShearX, img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX, img_Gaussian_ShearY, img_Scaling_Gaussian_ShearY, img_WeightX_Gaussian_ShearY, img_Scaling_WeightX_Gaussian_ShearY, img_WeightY_Gaussian_ShearY, img_Scaling_WeightY_Gaussian_ShearY, img_WeightX_WeightY_Gaussian_ShearY, img_Scaling_WeightX_WeightY_Gaussian_ShearY, img_Flip_Gaussian_ShearY,
                        img_Scaling_Flip_Gaussian_ShearY, img_WeightX_Flip_Gaussian_ShearY, img_Scaling_WeightX_Flip_Gaussian_ShearY, img_WeightY_Flip_Gaussian_ShearY, img_Scaling_WeightY_Flip_Gaussian_ShearY, img_WeightX_WeightY_Flip_Gaussian_ShearY, img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearY, img_ShearX_ShearY, img_Scaling_ShearX_ShearY, img_WeightX_ShearX_ShearY, img_Scaling_WeightX_ShearX_ShearY,
                        img_WeightY_ShearX_ShearY, img_Scaling_WeightY_ShearX_ShearY, img_WeightX_WeightY_ShearX_ShearY, img_Scaling_WeightX_WeightY_ShearX_ShearY, img_Flip_ShearX_ShearY, img_Scaling_Flip_ShearX_ShearY, img_WeightX_Flip_ShearX_ShearY, img_Scaling_WeightX_Flip_ShearX_ShearY, img_WeightY_Flip_ShearX_ShearY, img_Scaling_WeightY_Flip_ShearX_ShearY, img_WeightX_WeightY_Flip_ShearX_ShearY,
                        img_Scaling_WeightX_WeightY_Flip_ShearX_ShearY, img_Gaussian_ShearX_ShearY, img_Scaling_Gaussian_ShearX_ShearY, img_WeightX_Gaussian_ShearX_ShearY, img_Scaling_WeightX_Gaussian_ShearX_ShearY, img_WeightY_Gaussian_ShearX_ShearY, img_Scaling_WeightY_Gaussian_ShearX_ShearY, img_WeightX_WeightY_Gaussian_ShearX_ShearY, img_Scaling_WeightX_WeightY_Gaussian_ShearX_ShearY, img_Flip_Gaussian_ShearX_ShearY,
                        img_Scaling_Flip_Gaussian_ShearX_ShearY, img_WeightX_Flip_Gaussian_ShearX_ShearY, img_Scaling_WeightX_Flip_Gaussian_ShearX_ShearY, img_WeightY_Flip_Gaussian_ShearX_ShearY, img_Scaling_WeightY_Flip_Gaussian_ShearX_ShearY, img_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY, img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY
                        ;

                    QString name_Scaling, name_WeightX, name_WeightY, name_Flip, name_Gaussian, name_ShearX, name_ShearY, name_Scaling_WeightX, name_Scaling_WeightY, name_Scaling_Flip, name_Scaling_Gaussian, name_Scaling_ShearX, name_Scaling_ShearY,
                        name_WeightX_WeightY, name_Scaling_WeightX_WeightY, name_WeightX_Flip, name_Scaling_WeightX_Flip, name_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, name_WeightX_ShearX, name_Scaling_WeightX_ShearX, name_WeightX_ShearY, name_Scaling_WeightX_ShearY, name_WeightY_Flip, name_Scaling_WeightY_Flip,
                        name_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, name_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, name_WeightY_ShearX, name_Scaling_WeightY_ShearX, name_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX,
                        name_WeightY_ShearY, name_Scaling_WeightY_ShearY, name_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY, name_Flip_Gaussian, name_Scaling_Flip_Gaussian, name_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, name_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian,
                        name_Scaling_WeightX_WeightY_Flip_Gaussian, name_Flip_ShearX, name_Scaling_Flip_ShearX, name_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX, name_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX, name_Scaling_WeightX_WeightY_Flip_ShearX, name_Flip_ShearY, name_Scaling_Flip_ShearY,
                        name_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY, name_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearY, name_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Gaussian_ShearX,
                        name_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX, name_Scaling_WeightX_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightY_Flip_Gaussian_ShearX, name_WeightX_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX, name_Gaussian_ShearY, name_Scaling_Gaussian_ShearY, name_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY, name_WeightY_Gaussian_ShearY,
                        name_Scaling_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearY, name_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearY, name_ShearX_ShearY,
                        name_Scaling_ShearX_ShearY, name_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY, name_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY, name_Scaling_WeightX_WeightY_ShearX_ShearY, name_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY, name_Scaling_WeightX_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightY_Flip_ShearX_ShearY,
                        name_WeightX_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearX_ShearY, name_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY,
                        name_Scaling_Flip_Gaussian_ShearX_ShearY, name_WeightX_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearX_ShearY, name_WeightY_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY
                        ;

                    if(Scaling_input < 0 || Scaling_input > 0 ){ // 1 DONE LEAW
                      std::tie(img_Scaling, name_Scaling) = process_Scaling_Image(rotated, name_rotated);

                      if(WeightX_input < 0 || WeightX_input > 0){ // 2 DONE
                        std::tie(img_WeightX, name_WeightX) = process_WeightX_Image(rotated, name_rotated);
                        std::tie(img_Scaling_WeightX, name_Scaling_WeightX) = process_WeightX_Image(img_Scaling, name_Scaling);

                        if(WeightY_input < 0 || WeightY_input > 0){ // 4 done
                          std::tie(img_WeightY, name_WeightY) = process_WeightY_Image(rotated, name_rotated);
                          std::tie(img_Scaling_WeightY, name_Scaling_WeightY) = process_WeightY_Image(img_Scaling, name_Scaling);
                          std::tie(img_WeightX_WeightY, name_WeightX_WeightY) = process_WeightY_Image(img_WeightX, name_WeightX);
                          std::tie(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY) = process_WeightY_Image(img_Scaling_WeightX, name_Scaling_WeightX);

                          if(Flip_input == true){ // 8 done
                            std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                            std::tie(img_Scaling_Flip, name_Scaling_Flip) = process_Flip_Image(img_Scaling, name_Scaling);
                            std::tie(img_WeightX_Flip, name_WeightX_Flip) = process_Flip_Image(img_WeightX, name_WeightX);
                            std::tie(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip) = process_Flip_Image(img_Scaling_WeightX, name_Scaling_WeightX);
                            std::tie(img_WeightY_Flip, name_WeightY_Flip) = process_Flip_Image(img_WeightY, name_WeightY);
                            std::tie(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip) = process_Flip_Image(img_Scaling_WeightY, name_Scaling_WeightY);
                            std::tie(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip) = process_Flip_Image(img_WeightX_WeightY, name_WeightX_WeightY);
                            std::tie(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip) = process_Flip_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY);

                            if(Gaussian_input == true){ // 16
                              std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                              std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                              std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                              std::tie(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX, name_Scaling_WeightX);
                              std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                              std::tie(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian) = process_Gaussian_Image(img_Scaling_WeightY, name_Scaling_WeightY);
                              std::tie(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian) = process_Gaussian_Image(img_WeightX_WeightY, name_WeightX_WeightY);
                              std::tie(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY);
                              std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                              std::tie(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_Flip, name_Scaling_Flip);
                              std::tie(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian) = process_Gaussian_Image(img_WeightX_Flip, name_WeightX_Flip);
                              std::tie(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip);
                              std::tie(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_WeightY_Flip, name_WeightY_Flip);
                              std::tie(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip);
                              std::tie(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip);
                              std::tie(img_Scaling_WeightX_WeightY_Flip_Gaussian, name_Scaling_WeightX_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip);

                              if(ShearX_input > 0){ // 32
                                for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                                  std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                                  std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                                  std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                                  std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                                  std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                                  std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Xangle);
                                  std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                                  std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                                  std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Xangle);
                                  std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                                  std::tie(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Xangle);
                                  std::tie(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_WeightY_Flip_ShearX, name_Scaling_WeightX_WeightY_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, shear_Xangle);
                                  std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                                  std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Xangle);
                                  std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Xangle);
                                  std::tie(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, shear_Xangle);
                                  std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearX, name_Scaling_WeightX_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearX, name_WeightX_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian, shear_Xangle);
                                  std::tie(img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY_Flip_Gaussian, name_Scaling_WeightX_WeightY_Flip_Gaussian, shear_Xangle);

                                  if(ShearY_input > 0){ // 64
                                    for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                      std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                      std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                      std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                      std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                      std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                      std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                      std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                      std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_Flip_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, shear_Yangle);
                                      std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                      std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                      std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, shear_Yangle);
                                      std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip_Gaussian, name_Scaling_WeightX_WeightY_Flip_Gaussian, shear_Yangle);
                                      std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                      std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_ShearX_ShearY, name_Scaling_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX, shear_Yangle);
                                      std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_Flip_ShearX_ShearY, name_Scaling_WeightX_Flip_ShearX_ShearY) =  process_ShearY_Image(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX, shear_Yangle);
                                      std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_Flip_ShearX_ShearY, name_WeightX_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip_ShearX, name_Scaling_WeightX_WeightY_Flip_ShearX, shear_Yangle);
                                      std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_Flip_Gaussian_ShearX_ShearY, name_Scaling_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_Flip_Gaussian_ShearX_ShearY, name_WeightX_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_Gaussian_ShearX, name_Scaling_WeightX_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_Gaussian_ShearX, name_WeightX_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                      std::tie(img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearX, shear_Yangle);

                                    }
                                  }

                                }

                              }else if(ShearY_input > 0){ // 32
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_WeightY_Flip_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip_Gaussian, name_Scaling_WeightX_WeightY_Flip_Gaussian, shear_Yangle);

                                }
                              }

                            }else if(ShearX_input > 0){ // 16
                              for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                                std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                                std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                                std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                                std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                                std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                                std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                                std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                                std::tie(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Xangle);
                                std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                                std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                                std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                                std::tie(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Xangle);
                                std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                                std::tie(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Xangle);
                                std::tie(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Xangle);
                                std::tie(img_Scaling_WeightX_WeightY_Flip_ShearX, name_Scaling_WeightX_WeightY_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, shear_Xangle);

                                if(ShearY_input > 0){ // 32
                                  for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                    std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                    std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                    std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                    std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                    std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                    std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                    std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                    std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_Flip_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, shear_Yangle);
                                    std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_ShearX_ShearY, name_Scaling_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Flip_ShearX_ShearY, name_Scaling_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Flip_ShearX_ShearY, name_WeightX_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip_ShearX, name_Scaling_WeightX_WeightY_Flip_ShearX, shear_Yangle);

                                  }
                                }

                              }

                            }else if(ShearY_input > 0){ // 16
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                std::tie(img_Scaling_WeightX_WeightY_Flip_ShearY, name_Scaling_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Flip, name_Scaling_WeightX_WeightY_Flip, shear_Yangle);

                              }
                            }

                          }else if(Gaussian_input == true){ // 8 done
                            std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                            std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                            std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                            std::tie(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX, name_Scaling_WeightX);
                            std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                            std::tie(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian) = process_Gaussian_Image(img_Scaling_WeightY, name_Scaling_WeightY);
                            std::tie(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian) = process_Gaussian_Image(img_WeightX_WeightY, name_WeightX_WeightY);
                            std::tie(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY);

                            if(ShearX_input > 0){ // 16
                              for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                                std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                                std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                                std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                                std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                                std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                                std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                                std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                                std::tie(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Xangle);
                                std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Xangle);
                                std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, shear_Xangle);

                                if(ShearY_input > 0){ // 32
                                  for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                    std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                    std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                    std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                    std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                    std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                    std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_ShearX_ShearY, name_Scaling_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Gaussian_ShearX, name_Scaling_WeightX_WeightY_Gaussian_ShearX, shear_Yangle);

                                  }
                                }

                              }

                            }else if(ShearY_input > 0){ // 16
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightX_WeightY_Gaussian_ShearY, name_Scaling_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_Gaussian, name_Scaling_WeightX_WeightY_Gaussian, shear_Yangle);

                              }
                            }

                          }else if(ShearX_input > 0){ // 8 done
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                              std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                              std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                              std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                              std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                              std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                              std::tie(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_WeightY_ShearX_ShearY, name_Scaling_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY_ShearX, name_Scaling_WeightX_WeightY_ShearX, shear_Yangle);

                                }
                              }
                            }

                          }else if(ShearY_input > 0){ // 8 donee
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                              std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                              std::tie(img_Scaling_WeightX_WeightY_ShearY, name_Scaling_WeightX_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightX_WeightY, name_Scaling_WeightX_WeightY, shear_Yangle);
                            }
                          }
                        }else if(Flip_input == true){ // 4 done
                          std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                          std::tie(img_Scaling_Flip, name_Scaling_Flip) = process_Flip_Image(img_Scaling, name_Scaling);
                          std::tie(img_WeightX_Flip, name_WeightX_Flip) = process_Flip_Image(img_WeightX, name_WeightX);
                          std::tie(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip) = process_Flip_Image(img_Scaling_WeightX, name_Scaling_WeightX);

                          if(Gaussian_input == true){ // 8 done
                            std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                            std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                            std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                            std::tie(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX, name_Scaling_WeightX);
                            std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                            std::tie(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_Flip, name_Scaling_Flip);
                            std::tie(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian) = process_Gaussian_Image(img_WeightX_Flip, name_WeightX_Flip);
                            std::tie(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip);

                            if(ShearX_input > 0){ // 16
                              for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                                std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                                std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                                std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                                std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                                std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                                std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                                std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                                std::tie(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Xangle);
                                std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Xangle);
                                std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearX, name_Scaling_WeightX_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, shear_Xangle);

                                if(ShearY_input > 0){ // 32
                                  for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                    std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                    std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                    std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                    std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                    std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                    std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                    std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                    std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Flip_ShearX_ShearY, name_Scaling_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Flip_Gaussian_ShearX_ShearY, name_Scaling_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Flip_Gaussian_ShearX_ShearY, name_WeightX_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_Gaussian_ShearX, name_Scaling_WeightX_Flip_Gaussian_ShearX, shear_Yangle);

                                  }
                                }

                              }
                            }else if(ShearY_input > 0){ // 16
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightX_Flip_Gaussian_ShearY, name_Scaling_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_Gaussian, name_Scaling_WeightX_Flip_Gaussian, shear_Yangle);

                              }
                            }

                          }else if(ShearX_input > 0){ // 8 done
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                              std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                              std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                              std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                              std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                              std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                              std::tie(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Flip_ShearX_ShearY, name_Scaling_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip_ShearX, name_Scaling_WeightX_Flip_ShearX, shear_Yangle);

                                }
                              }

                            }
                          }else if(ShearY_input > 0){ // 8 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                              std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                              std::tie(img_Scaling_WeightX_Flip_ShearY, name_Scaling_WeightX_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Flip, name_Scaling_WeightX_Flip, shear_Yangle);

                            }
                          }

                        }else if(Gaussian_input == true){ // 4 done
                          std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                          std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                          std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                          std::tie(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian) = process_Gaussian_Image(img_Scaling_WeightX, name_Scaling_WeightX);

                          if(ShearX_input > 0){ // 8
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                              std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                              std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);
                              std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                              std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                              std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                              std::tie(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Xangle);

                              if(ShearY_input > 0){ // 8
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightX_Gaussian_ShearX_ShearY, name_Scaling_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian_ShearX, name_Scaling_WeightX_Gaussian_ShearX, shear_Yangle);

                                }
                              }

                            }
                          }else if(ShearY_input > 0){ // 8
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                              std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                              std::tie(img_Scaling_WeightX_Gaussian_ShearY, name_Scaling_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightX_Gaussian, name_Scaling_WeightX_Gaussian, shear_Yangle);

                            }
                          }

                        }else if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                            std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                            std::tie(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX) = process_ShearX_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                std::tie(img_Scaling_WeightX_ShearX_ShearY, name_Scaling_WeightX_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightX_ShearX, name_Scaling_WeightX_ShearX, shear_Yangle);

                              }
                            }

                          }
                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                            std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                            std::tie(img_Scaling_WeightX_ShearY, name_Scaling_WeightX_ShearY) = process_ShearY_Image(img_Scaling_WeightX, name_Scaling_WeightX, shear_Yangle);

                          }
                        }
  // 2............................................2....................
                      }else if(WeightY_input < 0 || WeightY_input > 0){ // 2 DONE
                        std::tie(img_WeightY, name_WeightY) = process_WeightY_Image(rotated, name_rotated);
                        std::tie(img_Scaling_WeightY, name_Scaling_WeightY) = process_WeightY_Image(img_Scaling, name_Scaling);

                        if(Flip_input == true){ // 4 done
                          std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                          std::tie(img_Scaling_Flip, name_Scaling_Flip) = process_Flip_Image(img_Scaling, name_Scaling);
                          std::tie(img_WeightY_Flip, name_WeightY_Flip) = process_Flip_Image(img_WeightY, name_WeightY);
                          std::tie(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip) = process_Flip_Image(img_Scaling_WeightY, name_Scaling_WeightY);

                          if(Gaussian_input == true){ // 8
                            std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                            std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                            std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                            std::tie(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian) = process_Gaussian_Image(img_Scaling_WeightY, name_Scaling_WeightY);
                            std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                            std::tie(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_Flip, name_Scaling_Flip);
                            std::tie(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_WeightY_Flip, name_WeightY_Flip);
                            std::tie(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip);

                            if(ShearX_input > 0){ // 16
                              for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                                std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                                std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                                std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                                std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                                std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                                std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                                std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                                std::tie(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Xangle);
                                std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                                std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Xangle);
                                std::tie(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Xangle);
                                std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, shear_Xangle);

                                if(ShearY_input > 0){ // 32
                                  for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                    std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                    std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                    std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                    std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                    std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                    std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                    std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                    std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_Flip_Gaussian_ShearX_ShearY, name_Scaling_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_Gaussian_ShearX, name_Scaling_WeightY_Flip_Gaussian_ShearX, shear_Yangle);

                                  }
                                }

                              }

                            }else if(ShearY_input > 0){ // 16
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_WeightY_Flip_Gaussian_ShearY, name_Scaling_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_Gaussian, name_Scaling_WeightY_Flip_Gaussian, shear_Yangle);

                              }
                            }

                          }else if(ShearX_input > 0){ // 8
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                              std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                              std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                              std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                              std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                              std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                              std::tie(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Flip_ShearX_ShearY, name_Scaling_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip_ShearX, name_Scaling_WeightY_Flip_ShearX, shear_Yangle);

                                }
                              }

                            }

                          }else if(ShearY_input > 0){ // 8
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                              std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                              std::tie(img_Scaling_WeightY_Flip_ShearY, name_Scaling_WeightY_Flip_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Flip, name_Scaling_WeightY_Flip, shear_Yangle);

                            }
                          }

                        }else if(Gaussian_input == true){ // 4 done
                          std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                          std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                          std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                          std::tie(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian) = process_Gaussian_Image(img_Scaling_WeightY, name_Scaling_WeightY);

                          if(ShearX_input > 0){ // 8
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                              std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                              std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);
                              std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                              std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                              std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                              std::tie(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_WeightY_Gaussian_ShearX_ShearY, name_Scaling_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian_ShearX, name_Scaling_WeightY_Gaussian_ShearX, shear_Yangle);

                                }
                              }


                            }

                          }else if(ShearY_input > 0){ // 8
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                              std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                              std::tie(img_Scaling_WeightY_Gaussian_ShearY, name_Scaling_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_WeightY_Gaussian, name_Scaling_WeightY_Gaussian, shear_Yangle);

                            }
                          }

                        }else if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                            std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                            std::tie(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX) = process_ShearX_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                std::tie(img_Scaling_WeightY_ShearX_ShearY, name_Scaling_WeightY_ShearX_ShearY) = process_ShearY_Image(img_Scaling_WeightY_ShearX, name_Scaling_WeightY_ShearX, shear_Yangle);

                              }
                            }

                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                            std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                            std::tie(img_Scaling_WeightY_ShearY, name_Scaling_WeightY_ShearY) = process_ShearY_Image(img_Scaling_WeightY, name_Scaling_WeightY, shear_Yangle);

                          }
                        }

                      }else if(Flip_input == true){ // 2 DONE
                        std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                        std::tie(img_Scaling_Flip, name_Scaling_Flip) = process_Flip_Image(img_Scaling, name_Scaling);

                        if(Gaussian_input == true){ // 4 done
                          std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                          std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);
                          std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                          std::tie(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian) = process_Gaussian_Image(img_Scaling_Flip, name_Scaling_Flip);

                          if(ShearX_input > 0){ // 8
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                              std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                              std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);
                              std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                              std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);
                              std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                              std::tie(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                  std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Scaling_Flip_Gaussian_ShearX_ShearY, name_Scaling_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian_ShearX, name_Scaling_Flip_Gaussian_ShearX, shear_Yangle);

                                }
                              }

                            }

                          }else if(ShearY_input > 0){ // 8
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                              std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                              std::tie(img_Scaling_Flip_Gaussian_ShearY, name_Scaling_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Flip_Gaussian, name_Scaling_Flip_Gaussian, shear_Yangle);

                            }
                          }

                        }else if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                            std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                            std::tie(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX) = process_ShearX_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                std::tie(img_Scaling_Flip_ShearX_ShearY, name_Scaling_Flip_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Flip_ShearX, name_Scaling_Flip_ShearX, shear_Yangle);

                              }
                            }

                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                            std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                            std::tie(img_Scaling_Flip_ShearY, name_Scaling_Flip_ShearY) = process_ShearY_Image(img_Scaling_Flip, name_Scaling_Flip, shear_Yangle);

                          }
                        }

                      }else if(Gaussian_input == true){ // 2 DONE
                        std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                        std::tie(img_Scaling_Gaussian, name_Scaling_Gaussian) = process_Gaussian_Image(img_Scaling, name_Scaling);

                        if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);
                            std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                            std::tie(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX) = process_ShearX_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);
                                std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                std::tie(img_Scaling_Gaussian_ShearX_ShearY, name_Scaling_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Scaling_Gaussian_ShearX, name_Scaling_Gaussian_ShearX, shear_Yangle);

                              }
                            }

                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                            std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                            std::tie(img_Scaling_Gaussian_ShearY, name_Scaling_Gaussian_ShearY) = process_ShearY_Image(img_Scaling_Gaussian, name_Scaling_Gaussian, shear_Yangle);

                          }
                        }

                      }else if(ShearX_input > 0){ // 2 DONE
                        for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                          std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                          std::tie(img_Scaling_ShearX, name_Scaling_ShearX) = process_ShearX_Image(img_Scaling, name_Scaling, shear_Xangle);

                          if(ShearY_input > 0){ // 4 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);
                              std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                              std::tie(img_Scaling_ShearX_ShearY, name_Scaling_ShearX_ShearY) = process_ShearY_Image(img_Scaling_ShearX, name_Scaling_ShearX, shear_Yangle);

                            }
                          }

                        }

                      }else if(ShearY_input > 0){ // 2 DONE
                        for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                          std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                          std::tie(img_Scaling_ShearY, name_Scaling_ShearY) = process_ShearY_Image(img_Scaling, name_Scaling, shear_Yangle);

                        }
                      }

                    }else if(WeightX_input < 0 || WeightX_input > 0){ // 1 DONE LEAW
                      std::tie(img_WeightX, name_WeightX) = process_WeightX_Image(rotated, name_rotated);

                      if(WeightY_input < 0 || WeightY_input > 0){ // 2 DONE
                        std::tie(img_WeightY, name_WeightY) = process_WeightY_Image(rotated, name_rotated);
                        std::tie(img_WeightX_WeightY, name_WeightX_WeightY) = process_WeightY_Image(img_WeightX, name_WeightX);

                        if(Flip_input == true){ // 4 DONE
                          std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                          std::tie(img_WeightX_Flip, name_WeightX_Flip) = process_Flip_Image(img_WeightX, name_WeightX);
                          std::tie(img_WeightY_Flip, name_WeightY_Flip) = process_Flip_Image(img_WeightY, name_WeightY);
                          std::tie(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip) = process_Flip_Image(img_WeightX_WeightY, name_WeightX_WeightY);

                          if(Gaussian_input == true){ // 8 done
                            std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                            std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                            std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                            std::tie(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian) = process_Gaussian_Image(img_WeightX_WeightY, name_WeightX_WeightY);
                            std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                            std::tie(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian) = process_Gaussian_Image(img_WeightX_Flip, name_WeightX_Flip);
                            std::tie(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_WeightY_Flip, name_WeightY_Flip);
                            std::tie(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip);

                            if(ShearX_input > 0){ // 16
                              for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                                std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                                std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                                std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                                std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                                std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                                std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                                std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                                std::tie(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Xangle);
                                std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                                std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Xangle);
                                std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Xangle);
                                std::tie(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Xangle);
                                std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearX, name_WeightX_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian, shear_Xangle);

                                if(ShearY_input > 0){ // 32
                                  for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                    std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                    std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                    std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                    std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                    std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                    std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                    std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                    std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                    std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian, shear_Yangle);
                                    std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                    std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Flip_ShearX_ShearY, name_WeightX_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX, shear_Yangle);
                                    std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_Flip_Gaussian_ShearX_ShearY, name_WeightX_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                    std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_Gaussian_ShearX, name_WeightX_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                  }
                                }
                              }

                            }else if(ShearY_input > 0){ // 16
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_WeightY_Flip_Gaussian_ShearY, name_WeightX_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_Gaussian, name_WeightX_WeightY_Flip_Gaussian, shear_Yangle);
                              }
                            }

                          }else if(ShearX_input > 0){ // 8 done
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                              std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                              std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                              std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                              std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                              std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                              std::tie(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Flip_ShearX_ShearY, name_WeightX_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip_ShearX, name_WeightX_WeightY_Flip_ShearX, shear_Yangle);
                                }
                              }

                            }

                          }else if(ShearY_input > 0){ // 8 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                              std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                              std::tie(img_WeightX_WeightY_Flip_ShearY, name_WeightX_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Flip, name_WeightX_WeightY_Flip, shear_Yangle);
                            }
                          }

                        }else if(Gaussian_input == true){ // 4 DONE
                          std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                          std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                          std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                          std::tie(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian) = process_Gaussian_Image(img_WeightX_WeightY, name_WeightX_WeightY);

                          if(ShearX_input > 0){ // 8 done
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                              std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                              std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);
                              std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                              std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                              std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                              std::tie(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_WeightY_Gaussian_ShearX_ShearY, name_WeightX_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian_ShearX, name_WeightX_WeightY_Gaussian_ShearX, shear_Yangle);
                                }
                              }
                            }

                          }else if(ShearY_input > 0){ // 8 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                              std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                              std::tie(img_WeightX_WeightY_Gaussian_ShearY, name_WeightX_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_WeightY_Gaussian, name_WeightX_WeightY_Gaussian, shear_Yangle);
                            }
                          }

                        }else if(ShearX_input > 0){ // 4 DONE
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                            std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                            std::tie(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX) = process_ShearX_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                std::tie(img_WeightX_WeightY_ShearX_ShearY, name_WeightX_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightX_WeightY_ShearX, name_WeightX_WeightY_ShearX, shear_Yangle);
                              }
                            }
                          }

                        }else if(ShearY_input > 0){ // 4 DONE
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                            std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                            std::tie(img_WeightX_WeightY_ShearY, name_WeightX_WeightY_ShearY) = process_ShearY_Image(img_WeightX_WeightY, name_WeightX_WeightY, shear_Yangle);
                          }
                        }

                      }else if(Flip_input == true){ // 2 DONE
                        std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                        std::tie(img_WeightX_Flip, name_WeightX_Flip) = process_Flip_Image(img_WeightX, name_WeightX);

                        if(Gaussian_input == true){ // 4 done
                          std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                          std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);
                          std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                          std::tie(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian) = process_Gaussian_Image(img_WeightX_Flip, name_WeightX_Flip);

                          if(ShearX_input > 0){ // 8 done
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                              std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                              std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);
                              std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                              std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);
                              std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                              std::tie(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                  std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightX_Flip_Gaussian_ShearX_ShearY, name_WeightX_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian_ShearX, name_WeightX_Flip_Gaussian_ShearX, shear_Yangle);
                                }
                              }
                            }

                          }else if(ShearY_input > 0){ // 8 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                              std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                              std::tie(img_WeightX_Flip_Gaussian_ShearY, name_WeightX_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Flip_Gaussian, name_WeightX_Flip_Gaussian, shear_Yangle);
                            }
                          }

                        }else if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                            std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                            std::tie(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX) = process_ShearX_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                std::tie(img_WeightX_Flip_ShearX_ShearY, name_WeightX_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Flip_ShearX, name_WeightX_Flip_ShearX, shear_Yangle);
                              }
                            }
                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                            std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                            std::tie(img_WeightX_Flip_ShearY, name_WeightX_Flip_ShearY) = process_ShearY_Image(img_WeightX_Flip, name_WeightX_Flip, shear_Yangle);
                          }
                        }

                      }else if(Gaussian_input == true){ // 2 DONE
                        std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                        std::tie(img_WeightX_Gaussian, name_WeightX_Gaussian) = process_Gaussian_Image(img_WeightX, name_WeightX);

                        if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);
                            std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                            std::tie(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX) = process_ShearX_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                                std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                std::tie(img_WeightX_Gaussian_ShearX_ShearY, name_WeightX_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightX_Gaussian_ShearX, name_WeightX_Gaussian_ShearX, shear_Yangle);
                              }
                            }
                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                            std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                            std::tie(img_WeightX_Gaussian_ShearY, name_WeightX_Gaussian_ShearY) = process_ShearY_Image(img_WeightX_Gaussian, name_WeightX_Gaussian, shear_Yangle);
                          }
                        }

                      }else if(ShearX_input > 0){ // 2 DONE
                        for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                          std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                          std::tie(img_WeightX_ShearX, name_WeightX_ShearX) = process_ShearX_Image(img_WeightX, name_WeightX, shear_Xangle);

                          if(ShearY_input > 0){ // 4
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                              std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                              std::tie(img_WeightX_ShearX_ShearY, name_WeightX_ShearX_ShearY) = process_ShearY_Image(img_WeightX_ShearX, name_WeightX_ShearX, shear_Yangle);
                            }
                          }
                        }

                      }else if(ShearY_input > 0){ // 2 DONE
                        for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                          std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                          std::tie(img_WeightX_ShearY, name_WeightX_ShearY) = process_ShearY_Image(img_WeightX, name_WeightX, shear_Yangle);
                        }
                      }

                    }else if(WeightY_input < 0 || WeightY_input > 0){ // 1 DONE LAEW
                      std::tie(img_WeightY, name_WeightY) = process_WeightY_Image(rotated, name_rotated);

                      if(Flip_input == true){ // 2 done
                        std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);
                        std::tie(img_WeightY_Flip, name_WeightY_Flip) = process_Flip_Image(img_WeightY, name_WeightY);

                        if(Gaussian_input == true){ // 4 done
                          std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                          std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);
                          std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);
                          std::tie(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian) = process_Gaussian_Image(img_WeightY_Flip, name_WeightY_Flip);

                          if(ShearX_input > 0){ // 8 done
                            for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                              std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                              std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                              std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                              std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);
                              std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                              std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);
                              std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);
                              std::tie(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Xangle);

                              if(ShearY_input > 0){ // 16
                                for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                  std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                  std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                  std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                  std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                                  std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                  std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                                  std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                                  std::tie(img_WeightY_Flip_Gaussian_ShearX_ShearY, name_WeightY_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian_ShearX, name_WeightY_Flip_Gaussian_ShearX, shear_Yangle);
                                }
                              }
                            }

                          }else if(ShearY_input > 0){ // 8  done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                              std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                              std::tie(img_WeightY_Flip_Gaussian_ShearY, name_WeightY_Flip_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Flip_Gaussian, name_WeightY_Flip_Gaussian, shear_Yangle);
                            }
                          }

                        }else if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                            std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                            std::tie(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX) = process_ShearX_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                std::tie(img_WeightY_Flip_ShearX_ShearY, name_WeightY_Flip_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Flip_ShearX, name_WeightY_Flip_ShearX, shear_Yangle);
                              }
                            }
                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                            std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                            std::tie(img_WeightY_Flip_ShearY, name_WeightY_Flip_ShearY) = process_ShearY_Image(img_WeightY_Flip, name_WeightY_Flip, shear_Yangle);
                          }
                        }

                      }else if(Gaussian_input == true){ // 2 done
                        std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                        std::tie(img_WeightY_Gaussian, name_WeightY_Gaussian) = process_Gaussian_Image(img_WeightY, name_WeightY);

                        if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);
                            std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                            std::tie(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX) = process_ShearX_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Xangle);

                            if(ShearY_input > 0){ // 8 done
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                                std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                std::tie(img_WeightY_Gaussian_ShearX_ShearY, name_WeightY_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_WeightY_Gaussian_ShearX, name_WeightY_Gaussian_ShearX, shear_Yangle);
                              }
                            }
                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                            std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                            std::tie(img_WeightY_Gaussian_ShearY, name_WeightY_Gaussian_ShearY) = process_ShearY_Image(img_WeightY_Gaussian, name_WeightY_Gaussian, shear_Yangle);
                          }
                        }

                      }else if(ShearX_input > 0){ // 2 done
                        for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                          std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                          std::tie(img_WeightY_ShearX, name_WeightY_ShearX) = process_ShearX_Image(img_WeightY, name_WeightY, shear_Xangle);

                          if(ShearY_input > 0){ // 4 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                              std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                              std::tie(img_WeightY_ShearX_ShearY, name_WeightY_ShearX_ShearY) = process_ShearY_Image(img_WeightY_ShearX, name_WeightY_ShearX, shear_Yangle);
                            }
                          }
                        }

                      }else if(ShearY_input > 0){ // 2 done
                        for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                          std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                          std::tie(img_WeightY_ShearY, name_WeightY_ShearY) = process_ShearY_Image(img_WeightY, name_WeightY, shear_Yangle);
                        }
                      }

                    }else if(Flip_input == true){ // 1 DONE LAEW
                      std::tie(img_Flip, name_Flip) = process_Flip_Image(rotated, name_rotated);

                      if(Gaussian_input == true){ // 2 done
                        std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);
                        std::tie(img_Flip_Gaussian, name_Flip_Gaussian) = process_Gaussian_Image(img_Flip, name_Flip);

                        if(ShearX_input > 0){ // 4 done
                          for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                            std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                            std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);
                            std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);
                            std::tie(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX) = process_ShearX_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Xangle);

                            if(ShearY_input > 0){ // 8
                              for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                                std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                                std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                                std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                                std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                                std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                                std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                                std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                                std::tie(img_Flip_Gaussian_ShearX_ShearY, name_Flip_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Flip_Gaussian_ShearX, name_Flip_Gaussian_ShearX, shear_Yangle);
                              }
                            }
                          }

                        }else if(ShearY_input > 0){ // 4 done
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                            std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                            std::tie(img_Flip_Gaussian_ShearY, name_Flip_Gaussian_ShearY) = process_ShearY_Image(img_Flip_Gaussian, name_Flip_Gaussian, shear_Yangle);
                          }
                        }


                      }else if(ShearX_input > 0){ // 2 done
                        for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                          std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                          std::tie(img_Flip_ShearX, name_Flip_ShearX) = process_ShearX_Image(img_Flip, name_Flip, shear_Xangle);

                          if(ShearY_input > 0){ // 4 done
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                              std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                              std::tie(img_Flip_ShearX_ShearY, name_Flip_ShearX_ShearY) = process_ShearY_Image(img_Flip_ShearX, name_Flip_ShearX, shear_Yangle);
                            }
                          }

                        }

                      }else if(ShearY_input > 0){ // 2 done
                        for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                          std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                          std::tie(img_Flip_ShearY, name_Flip_ShearY) = process_ShearY_Image(img_Flip, name_Flip, shear_Yangle);
                        }
                      }

                    }else if(Gaussian_input == true){ // 1 DONE LAEW
                      std::tie(img_Gaussian, name_Gaussian) = process_Gaussian_Image(rotated, name_rotated);

                      if(ShearX_input > 0){ // 2 done
                        for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                          std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);
                          std::tie(img_Gaussian_ShearX, name_Gaussian_ShearX) = process_ShearX_Image(img_Gaussian, name_Gaussian, shear_Xangle);

                          if(ShearY_input > 0){ // 4
                            for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                              std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                              std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                              std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                              std::tie(img_Gaussian_ShearX_ShearY, name_Gaussian_ShearX_ShearY) = process_ShearY_Image(img_Gaussian_ShearX, name_Gaussian_ShearX, shear_Yangle);
                            }
                          }
                        }

                      }else if(ShearY_input > 0){ // 2 done
                        for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                          std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                          std::tie(img_Gaussian_ShearY, name_Gaussian_ShearY) = process_ShearY_Image(img_Gaussian, name_Gaussian, shear_Yangle);
                        }
                      }

                    }else if(ShearX_input > 0){ // 1 DONE LAEW
                      for(shear_Xangle = shearX_angle_start; shear_Xangle <= limitShearX; shear_Xangle+=shearX_step_angle){
                        std::tie(img_ShearX, name_ShearX) = process_ShearX_Image(rotated, name_rotated, shear_Xangle);

                        if(ShearY_input > 0){ // 2
                          for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                            std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                            std::tie(img_ShearX_ShearY, name_ShearX_ShearY) = process_ShearY_Image(img_ShearX, name_ShearX, shear_Yangle);
                          }
                        }
                      }

                    }else if(ShearY_input > 0){ // 1  DONE LAEW
                      for(shear_Yangle = shearY_angle_start; shear_Yangle <= limitShearY; shear_Yangle+= shearY_step_angle){
                        std::tie(img_ShearY, name_ShearY) = process_ShearY_Image(rotated, name_rotated, shear_Yangle);
                      }
                    }
                  }
              }
            }
        }
      }

    }


    std::cout << "Generate DONE" << std::endl;
    std::cout << "Write GT file" << std::endl;
    QJsonDocument jsonDoc(gt_all_data);
    QFile file(fileGTname);
    file.open(QIODevice::WriteOnly);
    std::cout << ".........Writing Writing........." << std::endl;
    file.write(jsonDoc.toJson());
    file.close();

    std::cout << "Writing DONE" << std::endl;
    name_bg.clear();
    path_bg.clear();
    clicked_X.clear();
    clicked_Y.clear();
    gt_all_data = QJsonArray();


    QJsonObject gt_data_final;
    QJsonArray gt_all_data_final;
    QFile fileCurrent(fileGTname);
    if(checkRound2 == true){
      QFile fileBefore(gt1File);
      if(fileBefore.open(QIODevice::ReadOnly)){
        QTextStream inBefore(&fileBefore);
        QString fileBeforeContent = inBefore.readAll();
        fileBefore.close();
        QJsonDocument jsonDoc_fileBefore = QJsonDocument::fromJson(fileBeforeContent.toUtf8());
        if(!jsonDoc_fileBefore.isNull()){
          QJsonArray data_fileBefore = jsonDoc_fileBefore.array();
          if(fileCurrent.open(QIODevice::ReadOnly)){
            QTextStream inCurrent(&fileCurrent);
            QString fileCurrentContent = inCurrent.readAll();
            fileCurrent.close();
            QJsonDocument jsonDoc_fileCurrent = QJsonDocument::fromJson(fileCurrentContent.toUtf8());
            if(!jsonDoc_fileCurrent.isNull()){
              QJsonArray data_fileCurrent = jsonDoc_fileCurrent.array();
              for(auto obj1 : data_fileBefore){
                QString filenameBefore = obj1.toObject()["filename"].toString();
                QString filenameBefore_noJPG = filenameBefore.left(filenameBefore.lastIndexOf('.'));
                  for(auto obj2 : data_fileCurrent){
                    QString filenameCurrent = obj2.toObject()["filename"].toString();
                    QString filenameCurrent_noJPG = filenameCurrent.left(filenameCurrent.lastIndexOf('.'));
                    QJsonArray obj_array3;

                    if(filenameCurrent_noJPG.contains(filenameBefore_noJPG)){
                      QJsonArray obj_array1 = obj1.toObject()["obj_array"].toArray();
                      for(int IndexObj1 = 0; IndexObj1 < obj_array1.size(); IndexObj1++){
                        obj_array3.append(obj_array1[IndexObj1]);
                      }
                      QJsonArray obj_array2 = obj2.toObject()["obj_array"].toArray();
                      for(int IndexObj2 = 0; IndexObj2 < obj_array2.size(); IndexObj2++){
                        obj_array3.append(obj_array2[IndexObj2]);
                      }
                      gt_data_final["filename"] = filenameCurrent;
                      gt_data_final["obj_array"] = obj_array3;
                      gt_all_data_final.append(gt_data_final);
                    }
                  }
                }
              QJsonDocument jsonDoc_Finalfile(gt_all_data_final);
              QFile Finalfile(FinalnameGT);
              Finalfile.open(QIODevice::WriteOnly);
              Finalfile.write(jsonDoc_Finalfile.toJson());
              Finalfile.close();
            }
          }
       }
      }
    fileCurrent.remove();
    }



    jso["Roation"] = step_degree;
    jso["Scaling"] = Scaling_input;
    jso["WeightX"] = WeightX_input;
    jso["WeightY"] = WeightY_input;
    jso["ShearX"] = ShearX_input;
    jso["ShearY"] = ShearY_input;
    jso["Kernel"] = Kernel_input;
    jso["Flip"] = Flip_input;
    jso["Gaussian"] = Gaussian_input;
    jso["multiOutput"] = multiOutput;
    jso["Label"] = name_label;
    jso["Value by SpinBox"] = valueSpinBox;
    jso["Limit ShearX"] = limitShearX;
    jso["Limit ShearY"] = limitShearY;
    jso["TypeNoise"] = typeNoise;

    payload_js_data[name] = jso;

    QMessageBox msgBox_afterGenerate;
    msgBox_afterGenerate.setWindowTitle("Generate");
    msgBox_afterGenerate.setText("finish generating images");
    msgBox_afterGenerate.setIcon(QMessageBox::Information);
    msgBox_afterGenerate.exec();

  }

};

#endif // ThreadAugment_HPP
