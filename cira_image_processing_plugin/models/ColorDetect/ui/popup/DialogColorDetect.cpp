#include "DialogColorDetect.h"
#include "ui_DialogColorDetect.h"

DialogColorDetect::DialogColorDetect(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogColorDetect)
{
  ui->setupUi(this);
//  bool check = ui->groupBox->isChecked();
//  if(check == false){



//    ui->label->setEnabled(false);
//    ui->label_2->setEnabled(false);
//    ui->label_3->setEnabled(false);
//    ui->label_4->setEnabled(false);
//    ui->label_5->setEnabled(false);
//    ui->label_6->setEnabled(false);
//    ui->label_7->setEnabled(false);
//    ui->label_8->setEnabled(false);
//    ui->label_9->setEnabled(false);
//    ui->label_10->setEnabled(false);
//    ui->label_11->setEnabled(false);
//    ui->label_12->setEnabled(false);
//    ui->label_13->setEnabled(false);
//    ui->label_14->setEnabled(false);
//    ui->label_15->setEnabled(false);
//    ui->label_16->setEnabled(false);
//    ui->label_17->setEnabled(false);
//    ui->label_18->setEnabled(false);
//    ui->label_19->setEnabled(false);
//    ui->label_20->setEnabled(false);
//    ui->spinBox->setEnabled(false);
//    ui->spinBox_2->setEnabled(false);
//    ui->spinBox_3->setEnabled(false);
//    ui->spinBox_4->setEnabled(false);
//    ui->spinBox_5->setEnabled(false);
//    ui->spinBox_6->setEnabled(false);
//    //
//    ui->label_21->setEnabled(true);
//    ui->label_22->setEnabled(true);
//    ui->label_23->setEnabled(true);
//    ui->label_24->setEnabled(true);
//    ui->label_25->setEnabled(true);
//    ui->label_26->setEnabled(true);
//    ui->label_27->setEnabled(true);
//    ui->label_28->setEnabled(true);
//    ui->label_29->setEnabled(true);
//    ui->label_30->setEnabled(true);
//    ui->label_31->setEnabled(true);
//    ui->horizontalSlider->setEnabled(true);
//    ui->horizontalSlider_2->setEnabled(true);
//    ui->horizontalSlider_3->setEnabled(true);
//    ui->horizontalSlider_4->setEnabled(true);
//    ui->horizontalSlider_5->setEnabled(true);
//    ui->horizontalSlider_6->setEnabled(true);
//  }
//  else{
//    ui->label->setEnabled(true);
//    ui->label_2->setEnabled(true);
//    ui->label_3->setEnabled(true);
//    ui->label_4->setEnabled(true);
//    ui->label_5->setEnabled(true);
//    ui->label_6->setEnabled(true);
//    ui->label_7->setEnabled(true);
//    ui->label_8->setEnabled(true);
//    ui->label_9->setEnabled(true);
//    ui->label_10->setEnabled(true);
//    ui->label_11->setEnabled(true);
//    ui->label_12->setEnabled(true);
//    ui->label_13->setEnabled(true);
//    ui->label_14->setEnabled(true);
//    ui->label_15->setEnabled(true);
//    ui->label_16->setEnabled(true);
//    ui->label_17->setEnabled(true);
//    ui->label_18->setEnabled(true);
//    ui->label_19->setEnabled(true);
//    ui->label_20->setEnabled(true);
//    ui->spinBox->setEnabled(true);
//    ui->spinBox_2->setEnabled(true);
//    ui->spinBox_3->setEnabled(true);
//    ui->spinBox_4->setEnabled(true);
//    ui->spinBox_5->setEnabled(true);
//    ui->spinBox_6->setEnabled(true);
//    //
//    ui->label_21->setEnabled(false);
//    ui->label_22->setEnabled(false);
//    ui->label_23->setEnabled(false);
//    ui->label_24->setEnabled(false);
//    ui->label_25->setEnabled(false);
//    ui->label_26->setEnabled(false);
//    ui->label_27->setEnabled(false);
//    ui->label_28->setEnabled(false);
//    ui->label_29->setEnabled(false);
//    ui->label_30->setEnabled(false);
//    ui->label_31->setEnabled(false);
//    ui->horizontalSlider->setEnabled(false);
//    ui->horizontalSlider_2->setEnabled(false);
//    ui->horizontalSlider_3->setEnabled(false);
//    ui->horizontalSlider_4->setEnabled(false);
//    ui->horizontalSlider_5->setEnabled(false);
//    ui->horizontalSlider_6->setEnabled(false);

//  }

}

DialogColorDetect::~DialogColorDetect()
{
  delete ui;
}

QJsonObject DialogColorDetect::saveState() {
  QJsonObject param_js_data;
  param_js_data["group_Box"] = ui->groupBox->isChecked();

  param_js_data["lower_H_Box"] = ui->spinBox->value();
  param_js_data["lower_S_Box"] = ui->spinBox_2->value();
  param_js_data["lower_V_Box"] = ui->spinBox_3->value();
  param_js_data["upper_H_Box"] = ui->spinBox_4->value();
  param_js_data["upper_S_Box"] = ui->spinBox_5->value();
  param_js_data["upper_V_Box"] = ui->spinBox_6->value();

  param_js_data["lower_H_Bar"] = ui->horizontalSlider->value();
  param_js_data["lower_S_Bar"] = ui->horizontalSlider_3->value();
  param_js_data["lower_V_Bar"] = ui->horizontalSlider_6->value();
  param_js_data["upper_H_Bar"] = ui->horizontalSlider_2->value();
  param_js_data["upper_S_Bar"] = ui->horizontalSlider_4->value();
  param_js_data["upper_V_Bar"] = ui->horizontalSlider_5->value();



  return param_js_data;
}

void DialogColorDetect::restoreState(QJsonObject param_js_data) {
  ui->groupBox->setChecked(param_js_data["group_Box"].toBool());

  ui->spinBox->setValue(param_js_data["lower_H_Box"].toInt());
  ui->spinBox_2->setValue(param_js_data["lower_S_Box"].toInt());
  ui->spinBox_3->setValue(param_js_data["lower_V_Box"].toInt());
  ui->spinBox_4->setValue(param_js_data["upper_H_Box"].toInt());
  ui->spinBox_5->setValue(param_js_data["upper_S_Box"].toInt());
  ui->spinBox_6->setValue(param_js_data["upper_V_Box"].toInt());

  ui->horizontalSlider->setValue(param_js_data["lower_H_Bar"].toInt());
  ui->horizontalSlider_3->setValue(param_js_data["lower_S_Bar"].toInt());
  ui->horizontalSlider_6->setValue(param_js_data["lower_V_Bar"].toInt());
  ui->horizontalSlider_2->setValue(param_js_data["upper_H_Bar"].toInt());
  ui->horizontalSlider_4->setValue(param_js_data["upper_S_Bar"].toInt());
  ui->horizontalSlider_5->setValue(param_js_data["upper_V_Bar"].toInt());


}

void DialogColorDetect::on_groupBox_clicked(bool checked)
{
  if(checked == true){
    //

    ui->label->setEnabled(false);
    ui->label_2->setEnabled(false);
    ui->label_3->setEnabled(false);
    ui->label_4->setEnabled(false);
    ui->label_5->setEnabled(false);
    ui->label_6->setEnabled(false);
    ui->label_7->setEnabled(false);
    ui->label_8->setEnabled(false);
    ui->label_9->setEnabled(false);
    ui->label_10->setEnabled(false);
    ui->label_11->setEnabled(false);
    ui->label_12->setEnabled(false);
    ui->label_13->setEnabled(false);
    ui->label_14->setEnabled(false);
    ui->label_15->setEnabled(false);
    ui->label_16->setEnabled(false);
    ui->label_17->setEnabled(false);
    ui->label_18->setEnabled(false);
    ui->label_19->setEnabled(false);
    ui->label_20->setEnabled(false);
    ui->spinBox->setEnabled(false);
    ui->spinBox_2->setEnabled(false);
    ui->spinBox_3->setEnabled(false);
    ui->spinBox_4->setEnabled(false);
    ui->spinBox_5->setEnabled(false);
    ui->spinBox_6->setEnabled(false);
    //
    ui->label_21->setEnabled(true);
    ui->label_22->setEnabled(true);
    ui->label_23->setEnabled(true);
    ui->label_24->setEnabled(true);
    ui->label_25->setEnabled(true);
    ui->label_26->setEnabled(true);
    ui->label_27->setEnabled(true);
    ui->label_28->setEnabled(true);
    ui->label_29->setEnabled(true);
    ui->label_30->setEnabled(true);
    ui->label_31->setEnabled(true);
    ui->horizontalSlider->setEnabled(true);
    ui->horizontalSlider_2->setEnabled(true);
    ui->horizontalSlider_3->setEnabled(true);
    ui->horizontalSlider_4->setEnabled(true);
    ui->horizontalSlider_5->setEnabled(true);
    ui->horizontalSlider_6->setEnabled(true);
  }
  else{
    //
    ui->label->setEnabled(true);
    ui->label_2->setEnabled(true);
    ui->label_3->setEnabled(true);
    ui->label_4->setEnabled(true);
    ui->label_5->setEnabled(true);
    ui->label_6->setEnabled(true);
    ui->label_7->setEnabled(true);
    ui->label_8->setEnabled(true);
    ui->label_9->setEnabled(true);
    ui->label_10->setEnabled(true);
    ui->label_11->setEnabled(true);
    ui->label_12->setEnabled(true);
    ui->label_13->setEnabled(true);
    ui->label_14->setEnabled(true);
    ui->label_15->setEnabled(true);
    ui->label_16->setEnabled(true);
    ui->label_17->setEnabled(true);
    ui->label_18->setEnabled(true);
    ui->label_19->setEnabled(true);
    ui->label_20->setEnabled(true);
    ui->spinBox->setEnabled(true);
    ui->spinBox_2->setEnabled(true);
    ui->spinBox_3->setEnabled(true);
    ui->spinBox_4->setEnabled(true);
    ui->spinBox_5->setEnabled(true);
    ui->spinBox_6->setEnabled(true);
    //
    ui->label_21->setEnabled(false);
    ui->label_22->setEnabled(false);
    ui->label_23->setEnabled(false);
    ui->label_24->setEnabled(false);
    ui->label_25->setEnabled(false);
    ui->label_26->setEnabled(false);
    ui->label_27->setEnabled(false);
    ui->label_28->setEnabled(false);
    ui->label_29->setEnabled(false);
    ui->label_30->setEnabled(false);
    ui->label_31->setEnabled(false);
    ui->horizontalSlider->setEnabled(false);
    ui->horizontalSlider_2->setEnabled(false);
    ui->horizontalSlider_3->setEnabled(false);
    ui->horizontalSlider_4->setEnabled(false);
    ui->horizontalSlider_5->setEnabled(false);
    ui->horizontalSlider_6->setEnabled(false);
  }
}


void DialogColorDetect::on_horizontalSlider_valueChanged(int value)
{
    ui->label_26->setNum(value);
}

void DialogColorDetect::on_horizontalSlider_3_valueChanged(int value)
{
    ui->label_29->setNum(value);
}

void DialogColorDetect::on_horizontalSlider_6_valueChanged(int value)
{
    ui->label_31->setNum(value);
}

void DialogColorDetect::on_horizontalSlider_2_valueChanged(int value)
{
    ui->label_27->setNum(value);
}

void DialogColorDetect::on_horizontalSlider_4_valueChanged(int value)
{
    ui->label_28->setNum(value);
}

void DialogColorDetect::on_horizontalSlider_5_valueChanged(int value)
{
    ui->label_30->setNum(value);
}
