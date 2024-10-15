#include "DialogMatching.h"
#include "ui_DialogMatching.h"
#include <QFileDialog>
#include <QMessageBox>

DialogMatching::DialogMatching(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogMatching)
{
  ui->setupUi(this);

  if(array_img.isEmpty()){
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
  }

}

DialogMatching::~DialogMatching()
{
  delete ui;
}

QJsonObject DialogMatching::saveState() {
  QJsonObject param_js_data;
  param_js_data["Threshold"] = ui->doubleSpinBox->value();
  param_js_data["img"] = ui->label_2->text();
  return param_js_data;
}

void DialogMatching::restoreState(QJsonObject param_js_data) {
  ui->doubleSpinBox->setValue(param_js_data["Threshold"].toDouble());
  ui->label_2->setText(param_js_data["img"].toString());
  //if(array_img.isEmpty()){
  if(path_img.isEmpty()){
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
  }
  ui->pushButton->setEnabled(false);
  ui->pushButton_2->setEnabled(true);

}

void DialogMatching::on_pushButton_clicked()
{

  bool pin_tmp = isPin;
  isPin = true;

  ui->pushButton->setEnabled(false);

  QString dir = QFileDialog::getExistingDirectory(this, tr("Open Config Directory"),
                                                  "C://",
                                                  QFileDialog::DontUseNativeDialog /*| QFileDialog::ShowDirsOnly*/);

  if(dir == "") {
    ui->pushButton->setEnabled(true);
    isPin = pin_tmp;
    return;
  }
  ui->label->setText(dir);
  //QString path_img;

  for(int f = 0; f < QDir(dir).entryInfoList().size(); f++) {
      if(QDir(dir).entryInfoList().at(f).fileName().contains(".png")||QDir(dir).entryInfoList().at(f).fileName().contains(".jpg")) {
        path_img = QDir(dir).entryInfoList().at(f).absoluteFilePath();
        //array_img.append(path_img);
      }
  }
  //ui->listWidget->addItems(array_img);
  ui->label_2->setText(path_img);

  //if(array_img.isEmpty()) {
  if(path_img.isEmpty()) {
    QMessageBox::critical(this, "Load Template Error", "Can not load template files from this folder", QMessageBox::Ok);
    ui->pushButton->setEnabled(true);
    isPin = pin_tmp;
    return;
  }

  ui->pushButton->setEnabled(false);
  ui->pushButton_2->setEnabled(true);

  isPin = pin_tmp;

}

void DialogMatching::on_pushButton_2_clicked()
{
  //if (array_img.isEmpty()){}
  if (path_img.isEmpty()){}
  else{
    //array_img.clear();
    path_img.clear();
    }
    //ui->listWidget->takeItem(0);
    ui->label_2->setText("");
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
  }


