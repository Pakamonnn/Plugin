#ifndef ThreadContours_HPP
#define ThreadContours_HPP

#include <ros/ros.h>
#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <QThread>
#include <QJsonObject>
#include <vector>

#include <opencv2/opencv.hpp>

class ThreadContours : public QThread
{
  Q_OBJECT
public:

  QString name = "Contours";

  QJsonObject payload_js_data;
  QJsonObject output_js_data;
  QJsonObject param_js_data;

  bool isUseImage = true;
  cv::Mat mat_im, mat_out, mat_out2, image_contour,image_bounding;
  double aspect_ratio;
  QJsonArray obj_all_data;
  std::vector<double> all_aspect_ratio, all_diameter, all_area, all_extent;
  QJsonObject obj_data;
  double rect_area, extent;
  double max_aspect_ratio, max_diameter, max_area, max_extent;
  double min_aspect_ratio, min_diameter, min_area, min_extent;

  bool isHaveError = false;

  ThreadContours() { }

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
    std::cout << "IW RUN" << std::endl;
    int type_contour = param_js_data["contourtype"].toInt();
    int type_bounding = param_js_data["boundingtype"].toInt();
    bool puttext = param_js_data["puttext"].toBool();
    int size_text = param_js_data["sizetext"].toInt();
    int thickness = param_js_data["thickness"].toInt();
    int method = param_js_data["methodcontour"].toInt();
    QString method_array[] = {"RETR_TREE", "RETR_LIST", "RETR_CCOMP", "RETR_EXTERNAL"};
    QString methodContour = method_array[method];

    QString type_contour_array[] = {"Normal contour", "Convex Hull"};
    QString type_bounding_array[] = {"Normal bounding rectangle", "Minimun area bounding rectangle"};
    QString typeContour = type_contour_array[type_contour];
    QString typeBounding = type_bounding_array[type_bounding];
    cv::Mat binary, img_g;
    int count = 0;
    max_aspect_ratio = max_diameter = max_area = max_extent = -99999999999999;
    min_aspect_ratio = min_diameter = min_area = min_extent = 99999999999999;

    cv::cvtColor(mat_im, binary, cv::COLOR_BGR2GRAY);
    std::vector<std::vector<cv::Point>> contours_edges;
    std::vector<cv::Vec4i> hierarchy;
    if(methodContour == "RETR_TREE"){
      cv::findContours(binary, contours_edges, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
    }else if(methodContour == "RETR_LIST"){
      cv::findContours(binary, contours_edges, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    }else if(methodContour == "RETR_CCOMP"){
      cv::findContours(binary, contours_edges, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
    }else{
      cv::findContours(binary, contours_edges, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    }

    std::cout << "IW" << std::endl;

    cv::Mat image_contour = mat_im.clone();
    cv::Mat image_bounding = mat_im.clone();

    for (size_t cnt = 0; cnt < contours_edges.size(); cnt++) {
      count++;
      if(typeContour == "Normal contour"){
        cv::drawContours(image_contour, contours_edges, static_cast<int>(cnt), cv::Scalar(255, 255, 0), 3);
        double arcLength = cv::arcLength(contours_edges[cnt], true);
        double area = cv::contourArea(contours_edges[cnt]);
        double equi_diameter = std::sqrt(4* area / M_PI);
        obj_data["Lenght"] = arcLength;
        obj_data["Equivalent_Diameter"] = equi_diameter;
        all_area.push_back(area);
        all_diameter.push_back(equi_diameter);

        if(typeBounding == "Normal bounding rectangle"){
        cv::Rect rect = cv::boundingRect(contours_edges[cnt]);
        cv::rectangle(image_bounding, rect.tl(), rect.br(), cv::Scalar(0, 255, 0), 3);
        aspect_ratio = static_cast<double>(rect.width) / static_cast<double>(rect.height);
        rect_area = static_cast<double>(rect.width) * static_cast<double>(rect.height);
        extent = static_cast<double>(area) / static_cast<double>(rect_area);
        obj_data["Aspect_ratio"] = aspect_ratio;
        obj_data["Width"] = static_cast<double>(rect.width);
        obj_data["Height"] = static_cast<double>(rect.height);
        obj_data["Extent"] = static_cast<double>(extent);
        all_aspect_ratio.push_back(aspect_ratio);
        all_extent.push_back(extent);

        for(std::vector<double>::size_type i = 0; i < all_aspect_ratio.size(); i++){
          if(all_aspect_ratio[i] > max_aspect_ratio){
            max_aspect_ratio = all_aspect_ratio[i];
          }else if(all_aspect_ratio[i] < min_aspect_ratio){
            min_aspect_ratio = all_aspect_ratio[i];
          }
          if(all_extent[i] > max_extent){
            max_extent = all_extent[i];
          }else if(all_extent[i] < min_extent){
            min_extent = all_extent[i];
          }
        }
        jso["Max_Aspect_ratio"] = max_aspect_ratio;
        jso["Min_Aspect_ratio"] = min_aspect_ratio;
        jso["Max_Extent"] = max_extent;
        jso["Min_Extent"] = min_extent;


        }else{ // Minimun area bounding rectangle
          cv::RotatedRect rect = cv::minAreaRect(contours_edges[cnt]);
          cv::Point2f box[4];
          rect.points(box);
          std::vector<cv::Point> boxInPoint;
          for(int i = 0; i < 4; i++){
            boxInPoint.push_back(box[i]);
          }
          std::vector<std::vector<cv::Point>> contourTodraw = {boxInPoint};
          cv::drawContours(image_bounding, contourTodraw, -1, cv::Scalar(0, 255, 0), 3);
          aspect_ratio = static_cast<double>(rect.size.width) / static_cast<double>(rect.size.height);
          rect_area = static_cast<double>(rect.size.width) * static_cast<double>(rect.size.height);
          extent = static_cast<double>(area) / static_cast<double>(rect_area);
          obj_data["Aspect_ratio"] = aspect_ratio;
          obj_data["Width"] = static_cast<double>(rect.size.width);
          obj_data["Height"] = static_cast<double>(rect.size.height);
          all_aspect_ratio.push_back(aspect_ratio);\
          all_extent.push_back(extent);

          for(std::vector<double>::size_type i = 0; i < all_aspect_ratio.size(); i++){
            if(all_aspect_ratio[i] > max_aspect_ratio){
              max_aspect_ratio = all_aspect_ratio[i];
            }else if(all_aspect_ratio[i] < min_aspect_ratio){
              min_aspect_ratio = all_aspect_ratio[i];
            }
            if(all_extent[i] > max_extent){
              max_extent = all_extent[i];
            }else if(all_extent[i] < min_extent){
              min_extent = all_extent[i];
            }
          }
          jso["Max_Aspect_ratio"] = max_aspect_ratio;
          jso["Min_Aspect_ratio"] = min_aspect_ratio;
          jso["Max_Extent"] = max_extent;
          jso["Min_Extent"] = min_extent;
          }

      }else if(typeContour == "Convex Hull"){
        std::vector<cv::Point> hull;
        cv::convexHull(contours_edges[cnt], hull);
        cv::drawContours(image_contour, std::vector<std::vector<cv::Point>>{hull}, -1, cv::Scalar(255,255,0), 3);
        double arcLength = cv::arcLength(std::vector<cv::Point>{hull}, true);
        double area = cv::contourArea(std::vector<cv::Point>{hull});
        double equi_diameter = std::sqrt(4* area / M_PI);
        obj_data["Lenght"] = arcLength;
        obj_data["Equivalent_Diameter"] = equi_diameter;
        all_area.push_back(area);
        all_diameter.push_back(equi_diameter);

        if(typeBounding == "Normal bounding rectangle"){
        cv::Rect rect = cv::boundingRect(contours_edges[cnt]);
        cv::rectangle(image_bounding, rect.tl(), rect.br(), cv::Scalar(0, 255, 0), 3);
        aspect_ratio = static_cast<double>(rect.width) / static_cast<double>(rect.height);
        rect_area = static_cast<double>(rect.width) * static_cast<double>(rect.height);
        extent = static_cast<double>(area) / static_cast<double>(rect_area);
        obj_data["Aspect_ratio"] = aspect_ratio;
        obj_data["Width"] = static_cast<double>(rect.width);
        obj_data["Height"] = static_cast<double>(rect.height);
        obj_data["Extent"] = static_cast<double>(extent);
        all_aspect_ratio.push_back(aspect_ratio);
        all_extent.push_back(extent);

        for(std::vector<double>::size_type i = 0; i < all_aspect_ratio.size(); i++){
          if(all_aspect_ratio[i] > max_aspect_ratio){
            max_aspect_ratio = all_aspect_ratio[i];
          }else if(all_aspect_ratio[i] < min_aspect_ratio){
            min_aspect_ratio = all_aspect_ratio[i];
          }
          if(all_extent[i] > max_extent){
            max_extent = all_extent[i];
          }else if(all_extent[i] < min_extent){
            min_extent = all_extent[i];
          }
        }
        jso["Max_Aspect_ratio"] = max_aspect_ratio;
        jso["Min_Aspect_ratio"] = min_aspect_ratio;
        jso["Max_Extent"] = max_extent;
        jso["Min_Extent"] = min_extent;


        }else{ // Minimun area bounding rectangle
          cv::RotatedRect rect = cv::minAreaRect(contours_edges[cnt]);
          cv::Point2f box[4];
          rect.points(box);
          std::vector<cv::Point> boxInPoint;
          for(int i = 0; i < 4; i++){
            boxInPoint.push_back(box[i]);
          }
          std::vector<std::vector<cv::Point>> contourTodraw = {boxInPoint};
          cv::drawContours(image_bounding, contourTodraw, -1, cv::Scalar(0, 255, 0), 3);
          aspect_ratio = static_cast<double>(rect.size.width) / static_cast<double>(rect.size.height);
          rect_area = static_cast<double>(rect.size.width) * static_cast<double>(rect.size.height);
          extent = static_cast<double>(area) / static_cast<double>(rect_area);
          obj_data["Aspect_ratio"] = aspect_ratio;
          obj_data["Width"] = static_cast<double>(rect.size.width);
          obj_data["Height"] = static_cast<double>(rect.size.height);
          all_aspect_ratio.push_back(aspect_ratio);\
          all_extent.push_back(extent);

          for(std::vector<double>::size_type i = 0; i < all_aspect_ratio.size(); i++){
            if(all_aspect_ratio[i] > max_aspect_ratio){
              max_aspect_ratio = all_aspect_ratio[i];
            }else if(all_aspect_ratio[i] < min_aspect_ratio){
              min_aspect_ratio = all_aspect_ratio[i];
            }
            if(all_extent[i] > max_extent){
              max_extent = all_extent[i];
            }else if(all_extent[i] < min_extent){
              min_extent = all_extent[i];
            }
          }
          jso["Max_Aspect_ratio"] = max_aspect_ratio;
          jso["Min_Aspect_ratio"] = min_aspect_ratio;
          jso["Max_Extent"] = max_extent;
          jso["Min_Extent"] = min_extent;
          }
      }
      if(puttext == true){
        cv::putText(image_contour, std::to_string(count), cv::Point(contours_edges[cnt][0].x, contours_edges[cnt][0].y),
            cv::FONT_HERSHEY_SIMPLEX, size_text, cv::Scalar(0, 255, 0), thickness);
        cv::putText(image_bounding, std::to_string(count), cv::Point(contours_edges[cnt][0].x, contours_edges[cnt][0].y),
            cv::FONT_HERSHEY_SIMPLEX, size_text, cv::Scalar(0, 255, 0), thickness);
      }

      obj_data["Object"] = count;
      obj_all_data.append(obj_data);
    }
    std::cout << "END LOOP" << std::endl;

    for(std::vector<double>::size_type i = 0; i < all_diameter.size(); i++){
      if(all_diameter[i] > max_diameter){
        max_diameter = all_diameter[i];
      }else if(all_diameter[i] < min_diameter){
        min_diameter = all_diameter[i];
      }
      if(all_area[i] > max_area){
        max_area = all_area[i];
      }else if(all_area[i] < min_area){
        min_area = all_area[i];
      }
    }
    mat_out = image_contour;
    mat_out2 = image_bounding;
    std::cout << "IW DONE" << std::endl;

    jso["Contour_method"] = methodContour;
    jso["Contour_type"] = typeContour;
    jso["Contour_bounding"] = typeBounding;
    jso["Objects"] = obj_all_data;
    jso["Max_Diameter"] = max_diameter;
    jso["Min_Diameter"] = min_diameter;
    jso["Max_Area"] = max_area;
    jso["Min_Area"] = min_area;

    all_area.clear();
    all_aspect_ratio.clear();
    all_diameter.clear();

    payload_js_data[name] = jso;

  }


};

#endif // ThreadContours_HPP
