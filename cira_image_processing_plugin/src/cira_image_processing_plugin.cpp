#include <cira_image_processing_plugin/cira_image_processing_plugin.hpp>
#include <cira_lib_bernoulli/CiraBernoulliWidget.h>

#include <models/BinaryImage/BinaryImageModel.hpp>
#include <models/GrayScale/GrayScaleModel.hpp>
#include <models/BGRtoHSV/BGRtoHSVModel.hpp>
#include <models/ColorDetect/ColorDetectModel.hpp>
#include <models/AdaptiveThs/AdaptiveThsModel.hpp>
#include <models/Blur/BlurModel.hpp>
#include <models/Morphology/MorphologyModel.hpp>
#include <models/Canny_Edge/Canny_EdgeModel.hpp>
#include <models/Find_Edge/Find_EdgeModel.hpp>
#include <models/Resize/ResizeModel.hpp>
#include <models/Rotation/RotationModel.hpp>
#include <models/Flip/FlipModel.hpp>
#include <models/Matching/MatchingModel.hpp>




cira_image_processing_plugin::cira_image_processing_plugin() {

}

std::shared_ptr<DataModelRegistry> cira_image_processing_plugin::registerDataModels(std::shared_ptr<DataModelRegistry> ret) {


  ret->registerModel<BinaryImageModel>();
  ret->registerModel<GrayScaleModel>();
  ret->registerModel<BGRtoHSVModel>();
  ret->registerModel<ColorDetectModel>();
  ret->registerModel<AdaptiveThsModel>();
  ret->registerModel<BlurModel>();
  ret->registerModel<MorphologyModel>();
  ret->registerModel<Canny_EdgeModel>();
  ret->registerModel<Find_EdgeModel>();
  ret->registerModel<ResizeModel>();
  ret->registerModel<RotationModel>();
  ret->registerModel<FlipModel>();
  ret->registerModel<MatchingModel>();





  setTreeWidget();
  return ret;

}

void cira_image_processing_plugin::setTreeWidget() {

  QStringList strListNodeDataModels;
  QString category;
  QString category2;
  QString category3;
  QString category4;
  QString category5;
  QString category6;
  QString category7;


  category = "Thresholding";
  strListNodeDataModels << category + ",GrayScale"      + ",null";
  strListNodeDataModels << category + ",BinaryImage"      + ",null";
  strListNodeDataModels << category + ",AdaptiveThs"      + ",null";

  category2 = "Color_Space";
  strListNodeDataModels << category2 + ",BGRtoHSV"      + ",null";
  strListNodeDataModels << category2 + ",ColorDetect"      + ",null";

  category3 = "Image_Filter";
  strListNodeDataModels << category3 + ",Blur"      + ",null";

  category4 = "Image_Morphological";
  strListNodeDataModels << category4 + ",Morphology"      + ",null";

  category5 = "Edge_Detection";
  strListNodeDataModels << category5 + ",Find_Edge"      + ",null";
  strListNodeDataModels << category5 + ",Canny_Edge"      + ",null";

  category6 = "Geometric_Transformations";
  strListNodeDataModels << category6 + ",Resize"      + ",null";
  strListNodeDataModels << category6 + ",Rotation"      + ",null";
  strListNodeDataModels << category6 + ",Flip"      + ",null";


  category7 = "Template_Matching";
  strListNodeDataModels << category7 + ",Matching"      + ",null";





  CiraBernoulliWidget::pluginTreeWidget->addNodeDataModels(strListNodeDataModels);

}
