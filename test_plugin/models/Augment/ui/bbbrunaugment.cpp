#include "bbbrunaugment.h"

cv::Mat bbbrunaugment::MouseHandler::new_bg;
int bbbrunaugment::MouseHandler::numberOfPosition;
int bbbrunaugment::MouseHandler::position_n;
int bbbrunaugment::MouseHandler::bg_countRef;
std::vector<int> bbbrunaugment::MouseHandler::x_all;
std::vector<int> bbbrunaugment::MouseHandler::y_all;
int bbbrunaugment::MouseHandler::numberClicked;

bbbrunaugment::bbbrunaugment(const QJsonObject& params)
{
  typeNoise = params["typeNoise"].toString();
  positionconfiguration = params["positionconfiguration"].toString();
  MouseHandler::numberOfPosition = params["NumberOfPosition"].toInt();
  int maxX, maxY, minX, minY;
  std::vector<int> X_inBG, Y_inBG;
  std::vector<std::vector<int>> x_alls, y_alls;

  bool valueSpinBox = params["valueSpinBox"].toBool();
  int step_degree_input, Scaling_input, WeightX_input, WeightY_input, ShearX_input, ShearY_input, limitShearX, limitShearY;
  if(valueSpinBox == false){ //HorizontalSlider
    step_degree_input = params["RotationSlider"].toInt();
    Scaling_input = params["ScalingSlider"].toInt();
    WeightX_input = params["WeightXSlider"].toInt();
    WeightY_input = params["WeightYSlider"].toInt();
    ShearX_input = params["ShearXSlider"].toInt();
    ShearY_input = params["ShearYSlider"].toInt();
    limitShearX = params["limitShearXSlider"].toInt();
    limitShearY = params["limitShearYSlider"].toInt();
  }else{ //Spinbox
    step_degree_input = params["RotationSpibox"].toInt();
    Scaling_input = params["ScalingSpinbox"].toInt();
    WeightX_input = params["WeightXSpinbox"].toInt();
    WeightY_input = params["WeightYSpinbox"].toInt();
    ShearX_input = params["ShearXSpinbox"].toInt();
    ShearY_input = params["ShearYSpinbox"].toInt();
    limitShearX = params["limitShearX"].toInt();
    limitShearY = params["limitShearY"].toInt();
  }
  Kernel_input = params["Kernel"].toInt();
  Flip_input = params["Flip"].toBool();
  Gaussian_input = params["Gaussian"].toBool();
  bool multiOutput = params["Multioutput"].toBool();
  QString label_1 = params["Label"].toString();
  QString nameGT = params["nameGT"].toString();
  QString NoiseFolder = params["NoiseFolder"].toString();
  QString bgFolder = params["BakgroungFolder"].toString();
  QString pathSave = params["PathSave"].toString();
  PathSave = pathSave + "/";
  name_label = label_1;
  bool checkRound2 = params["CheckRound2"].toBool();
  QString fileGTname, FinalnameGT;
  if(checkRound2 == true){
    fileGTname = PathSave + "2ndgt" + QStringLiteral(".gt");
    FinalnameGT = PathSave + nameGT + QStringLiteral(".gt");
  }else{
    fileGTname = PathSave + nameGT + QStringLiteral(".gt");

  }

  MouseHandler mouseHandler;
  QString gt1File = params["gtR1file"].toString();

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
    std::cout << "fileNameBG : " << fileNameBG.toStdString() << std::endl;
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
      wait_n = true; //pointer value to connect
      ros::Rate rate(25);
      while(wait_n){
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
      wait_n = true; //pointer value to connect
      ros::Rate rate(25);
      while(wait_n){
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

  QMessageBox msgBox_afterGenerate;
  msgBox_afterGenerate.setWindowTitle("Generate");
  msgBox_afterGenerate.setText("finish generating images");
  msgBox_afterGenerate.setIcon(QMessageBox::Information);
  msgBox_afterGenerate.exec();

}
