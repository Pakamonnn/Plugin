#include <test_plugin/test_plugin.hpp>
#include <cira_lib_bernoulli/CiraBernoulliWidget.h>

#include <models/Affine/AffineModel.hpp>
#include <models/Histogram/HistogramModel.hpp>
#include <models/Contours/ContoursModel.hpp>
#include <models/Augment/AugmentModel.hpp>
test_plugin::test_plugin() {

}

std::shared_ptr<DataModelRegistry> test_plugin::registerDataModels(std::shared_ptr<DataModelRegistry> ret) {

    ret->registerModel<AffineModel>();
    ret->registerModel<HistogramModel>();
    ret->registerModel<ContoursModel>();
    ret->registerModel<AugmentModel>();

  /**********************
  regist model here
  example:

    ret->registerModel<Some1Model>();
    ret->registerModel<Some2Model>();

  ******************/

  setTreeWidget();
  return ret;

}

void test_plugin::setTreeWidget() {

  QStringList strListNodeDataModels;
  QString category;
  QString category1;
  QString category2;
  QString category3;


  category = "Histogram";
  strListNodeDataModels << category + ",Histogram"      + ",null";

  category1 = "Geometric_Transformations";
  strListNodeDataModels << category1 + ",Affine"      + ",null";

  category2 = "Contours";
  strListNodeDataModels << category2 + ",Contours"      + ",null";

  category3 = "Augmentation";
  strListNodeDataModels << category3 + ",Augment"      + ",null";



  CiraBernoulliWidget::pluginTreeWidget->addNodeDataModels(strListNodeDataModels);

  /**********************
  regist model name here for drag&drop
  example:

    category = "SomeCategory";
    strListNodeDataModels << category + ",Some1"      + ",null";
    strListNodeDataModels << category + ",Some2"      + ",null";

    CiraBernoulliWidget::pluginTreeWidget->addNodeDataModels(strListNodeDataModels);

  ******************/

}
