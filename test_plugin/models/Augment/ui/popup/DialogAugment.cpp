#include "DialogAugment.h"
#include "ui_DialogAugment.h"
#include <QFileDialog>
#include <QMessageBox>

#include <cira_lib_bernoulli/general/GlobalData.hpp>
#include <cira_lib_bernoulli/datatype/FlowData.hpp>
#include <cira_lib_bernoulli/asm_opencv/asmOpenCV.h>

#include <QTimer>
#include "Augment/ui/bbbrunaugment.h"
#include "Augment/ui/bbbrunaugment.cpp"

DialogAugment::DialogAugment(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DialogAugment)
{
  ui->setupUi(this);

  calculationTimer = new QTimer(this);
  connect(calculationTimer, &QTimer::timeout, this, &DialogAugment::claculateTotalImage);
  calculationTimer->setInterval(200);
  calculationTimer->start();
  
  //cira_std::connectFlowSlot("<flowName>", this, SLOT(getFlow(std::shared_ptr<FlowData>)));
  
}

DialogAugment::~DialogAugment()
{
  delete ui;
}

/*void DialogAugment::getFlow(std::shared_ptr<FlowData> flow) {
  FlowDataType data = flow->getFlowData();
  QJsonObject payload = data._payload_js_data["payload"].toObject();
  cv::Mat mat_im = data._img;
  
}*/

QJsonObject DialogAugment::saveState() {
  QJsonObject param_js_data;

  param_js_data["RotationSlider"] = ui->horizontalSlider_Rotated->value();
  param_js_data["ScalingSlider"] = ui->horizontalSlider_Scaling->value();
  param_js_data["WeightXSlider"] = ui->horizontalSlider_WeightX->value();
  param_js_data["WeightYSlider"] = ui->horizontalSlider_WeightY->value();
  param_js_data["ShearXSlider"] = ui->horizontalSlider_ShearX->value();
  param_js_data["ShearYSlider"] = ui->horizontalSlider_ShearY->value();
  param_js_data["Flip"] = ui->checkBox_Flip->isChecked();
  param_js_data["Gaussian"] = ui->groupBox_Gaussian->isChecked();
  param_js_data["Kernel"] = ui->spinBox_Kernal->value();
  param_js_data["Multioutput"] = ui->checkBox_multiOutput->isChecked();
  param_js_data["Label"] = ui->plainTextEdit_labelname->toPlainText();
  param_js_data["nameGT"] = ui->plainTextEdit_GTname->toPlainText();
  param_js_data["limitShearXSlider"] = ui->horizontalSlider_limitShearX->value();
  param_js_data["limitShearYSlider"] = ui->horizontalSlider_limitShearY->value();
  param_js_data["valueSpinBox"] = ui->checkBox_spinbox->isChecked();
  param_js_data["RotationSpibox"] = ui->spinBox_rotation->value();
  param_js_data["ScalingSpinbox"] = ui->spinBox_Scaling->value();
  param_js_data["WeightXSpinbox"] = ui->spinBox_WeightX->value();
  param_js_data["WeightYSpinbox"] = ui->spinBox_WeightY->value();
  param_js_data["ShearXSpinbox"] = ui->spinBox_ShearX->value();
  param_js_data["ShearYSpinbox"] = ui->spinBox_ShearY->value();
  param_js_data["limitShearX"] = ui->spinBox_limitShearX->value();
  param_js_data["limitShearY"] = ui->spinBox_limitShearY->value();
  param_js_data["NoiseFolder"] = ui->textBrowser_add_noise->toPlainText();
  param_js_data["BakgroungFolder"] = ui->textBrowser_add_BG->toPlainText();
  param_js_data["PathSave"] = ui->textBrowser_saveas->toPlainText();

  param_js_data["typeNoise"] = ui->comboBox_typeNoise->currentText();
  param_js_data["positionconfiguration"] = ui->comboBox_positionConfiguration->currentText();
  param_js_data["NumberOfPosition"] = ui->spinBox_numberOfPosition->value();
  param_js_data["gtR1file"] = ui->textBrowser_gtgenR2->toPlainText();
  param_js_data["CheckRound2"] = ui->groupBox_genR2->isChecked();

  return param_js_data;
}

void DialogAugment::restoreState(QJsonObject param_js_data) {

  ui->horizontalSlider_Rotated->setValue(param_js_data["RotationSlider"].toInt());
  ui->horizontalSlider_Scaling->setValue(param_js_data["ScalingSlider"].toInt());
  ui->horizontalSlider_WeightX->setValue(param_js_data["WeightXSlider"].toInt());
  ui->horizontalSlider_WeightY->setValue(param_js_data["WeightYSlider"].toInt());
  ui->horizontalSlider_ShearX->setValue(param_js_data["ShearXSlider"].toInt());
  ui->horizontalSlider_ShearY->setValue(param_js_data["ShearYSlider"].toInt());
  ui->checkBox_Flip->setChecked(param_js_data["Flip"].toBool());
  ui->groupBox_Gaussian->setChecked(param_js_data["Gaussian"].toBool());
  ui->spinBox_Kernal->setValue(param_js_data["Kernel"].toInt());
  ui->checkBox_multiOutput->setChecked(param_js_data["Multioutput"].toBool());
  ui->plainTextEdit_labelname->setPlainText(param_js_data["Label"].toString());
  ui->plainTextEdit_GTname->setPlainText(param_js_data["nameGT"].toString());
  ui->horizontalSlider_limitShearX->setValue(param_js_data["limitShearXSlider"].toInt());
  ui->horizontalSlider_limitShearY->setValue(param_js_data["limitShearYSlider"].toInt());

  ui->checkBox_spinbox->setChecked(param_js_data["valueSpinBox"].toBool());
  on_checkBox_spinbox_clicked(param_js_data["valueSpinBox"].toBool());
  ui->spinBox_rotation->setValue(param_js_data["RotationSpibox"].toInt());
  ui->spinBox_Scaling->setValue(param_js_data["ScalingSpinbox"].toInt());
  ui->spinBox_WeightX->setValue(param_js_data["WeightXSpinbox"].toInt());
  ui->spinBox_WeightY->setValue(param_js_data["WeightYSpinbox"].toInt());
  ui->spinBox_ShearX->setValue(param_js_data["ShearXSpinbox"].toInt());
  ui->spinBox_ShearY->setValue(param_js_data["ShearYSpinbox"].toInt());
  ui->spinBox_limitShearX->setValue(param_js_data["limitShearX"].toInt());
  ui->spinBox_limitShearY->setValue(param_js_data["limitShearY"].toInt());

  ui->textBrowser_add_noise->setText(param_js_data["NoiseFolder"].toString());
  ui->textBrowser_add_BG->setText(param_js_data["BakgroungFolder"].toString());
  ui->textBrowser_saveas->setText(param_js_data["PathSave"].toString());

  ui->comboBox_typeNoise->setCurrentText(param_js_data["typeNoise"].toString());
  ui->comboBox_positionConfiguration->setCurrentText(param_js_data["positionconfiguration"].toString());
  ui->spinBox_numberOfPosition->setValue(param_js_data["NumberOfPosition"].toInt());
  ui->textBrowser_gtgenR2->setText(param_js_data["gtR1file"].toString());
  ui->groupBox_genR2->setChecked(param_js_data["CheckRound2"].toBool());

}

void DialogAugment:: on_pushButton_gtgenR2_clicked(){
  QString gtFile = QFileDialog::getOpenFileName(this,tr("Choose gt file"),
                                                           "C://",
                                                           tr("gt (*.gt)"));
  ui->textBrowser_gtgenR2->setText(gtFile);
}

void DialogAugment:: on_pushButton_add_noise_clicked(){
  QString folderPathNoise = QFileDialog::getExistingDirectory(this,tr("Choose folder noise"),
                                                           "C://",
                                                           QFileDialog::DontUseNativeDialog);
  ui->textBrowser_add_noise->setText(folderPathNoise);
}

void DialogAugment:: on_pushButton_add_BG_clicked(){
  QString file_floderBG = QFileDialog::getExistingDirectory(this,tr("Choose folder background"),
                                                          "C://",
                                                          QFileDialog::DontUseNativeDialog);
  ui->textBrowser_add_BG->setText(file_floderBG);
}

void DialogAugment:: on_pushButton_saveas_clicked(){
  QString file_floder_save = QFileDialog::getExistingDirectory(this,tr("Choose folder save"),
                                                          "C://",
                                                          QFileDialog::DontUseNativeDialog);
  ui->textBrowser_saveas->setText(file_floder_save);
}

void DialogAugment:: on_horizontalSlider_Rotated_valueChanged(int value){
  ui->label_NumRotated->setNum(value);
}

void DialogAugment:: on_horizontalSlider_Scaling_valueChanged(int value){
  ui->label_NumScaling->setNum(value);
}

void DialogAugment:: on_horizontalSlider_WeightX_valueChanged(int value){
  ui->label_NumWeightX->setNum(value);
}

void DialogAugment:: on_horizontalSlider_WeightY_valueChanged(int value){
  ui->label_NumWeightY->setNum(value);
}

void DialogAugment:: on_horizontalSlider_ShearX_valueChanged(int value){
  ui->label_NumShearX->setNum(value);
}

void DialogAugment:: on_horizontalSlider_ShearY_valueChanged(int value){
  ui->label_NumShearY->setNum(value);
}

void DialogAugment:: on_horizontalSlider_limitShearX_valueChanged(int value){
  ui->label_NumlimitShearX->setNum(value);
}

void DialogAugment:: on_horizontalSlider_limitShearY_valueChanged(int value){
  ui->label_NumlimitShearY->setNum(value);
}

void DialogAugment:: on_checkBox_spinbox_clicked(bool checked){
  if(checked == false){ //ไม่ติ๊ก
    ui->horizontalSlider_Rotated->setEnabled(true);
    ui->label_NumRotated->setEnabled(true);
    ui->horizontalSlider_Scaling->setEnabled(true);
    ui->label_NumScaling->setEnabled(true);
    ui->horizontalSlider_WeightX->setEnabled(true);
    ui->label_NumWeightX->setEnabled(true);
    ui->horizontalSlider_WeightY->setEnabled(true);
    ui->label_NumWeightY->setEnabled(true);
    ui->horizontalSlider_ShearX->setEnabled(true);
    ui->label_NumShearX->setEnabled(true);
    ui->horizontalSlider_ShearY->setEnabled(true);
    ui->label_NumShearY->setEnabled(true);
    ui->horizontalSlider_limitShearX->setEnabled(true);
    ui->label_NumlimitShearX->setEnabled(true);
    ui->horizontalSlider_limitShearY->setEnabled(true);
    ui->label_NumlimitShearY->setEnabled(true);

    ui->spinBox_rotation->setEnabled(false); //หาย
    ui->spinBox_Scaling->setEnabled(false);
    ui->spinBox_WeightX->setEnabled(false);
    ui->spinBox_WeightY->setEnabled(false);
    ui->spinBox_ShearX->setEnabled(false);
    ui->spinBox_ShearY->setEnabled(false);
    ui->spinBox_limitShearX->setEnabled(false);
    ui->spinBox_limitShearY->setEnabled(false);
  }else{
    ui->horizontalSlider_Rotated->setEnabled(false);
    ui->label_NumRotated->setEnabled(false);
    ui->horizontalSlider_Scaling->setEnabled(false);
    ui->label_NumScaling->setEnabled(false);
    ui->horizontalSlider_WeightX->setEnabled(false);
    ui->label_NumWeightX->setEnabled(false);
    ui->horizontalSlider_WeightY->setEnabled(false);
    ui->label_NumWeightY->setEnabled(false);
    ui->horizontalSlider_ShearX->setEnabled(false);
    ui->label_NumShearX->setEnabled(false);
    ui->horizontalSlider_ShearY->setEnabled(false);
    ui->label_NumShearY->setEnabled(false);
    ui->horizontalSlider_limitShearX->setEnabled(false);
    ui->label_NumlimitShearX->setEnabled(false);
    ui->horizontalSlider_limitShearY->setEnabled(false);
    ui->label_NumlimitShearY->setEnabled(false);

    ui->spinBox_rotation->setEnabled(true);
    ui->spinBox_Scaling->setEnabled(true);
    ui->spinBox_WeightX->setEnabled(true);
    ui->spinBox_WeightY->setEnabled(true);
    ui->spinBox_ShearX->setEnabled(true);
    ui->spinBox_ShearY->setEnabled(true);
    ui->spinBox_limitShearX->setEnabled(true);
    ui->spinBox_limitShearY->setEnabled(true);
  }
}

void DialogAugment::claculateTotalImage(){
  QString bgFolder = ui->textBrowser_add_BG->toPlainText();
  QDir bg(bgFolder);
  QFileInfoList fileListBG = bg.entryInfoList();
  int number_bg = 0;
  for(int fbg = 0; fbg <fileListBG.size(); fbg++){ //count number of bg
    QString fileNameBG = fileListBG.at(fbg).fileName();
    if(fileNameBG.endsWith(".png", Qt::CaseInsensitive) || fileNameBG.endsWith(".jpg", Qt::CaseInsensitive)
       || fileNameBG.endsWith(".bmp", Qt::CaseInsensitive)){
      number_bg++;
    }
  }
  QString NoiseFolder = ui->textBrowser_add_noise->toPlainText();
  QDir dir(NoiseFolder);
  QFileInfoList fileListNoise = dir.entryInfoList();
  int number_noise = 0;
  for (int f = 0; f < fileListNoise.size(); f++){ //count number of noise
    QString fileName = fileListNoise.at(f).fileName();
    if (fileName.endsWith(".png", Qt::CaseInsensitive) || fileName.endsWith(".jpg", Qt::CaseInsensitive)){
      number_noise++;
    }
  }
  bool multi_output = ui->checkBox_multiOutput->checkState(); //chosse 1 , non 0
  bool InputValue = ui->checkBox_spinbox->checkState();
  bool FlipValue = ui->checkBox_Flip->checkState();
  bool GaussianValue = ui->groupBox_Gaussian->isChecked();
  int RotatedValue, ScalingValue, WeightXvalue, WeightYvalue, ShearXvalue, limitShearX, ShearYvalue, limitShearY;
  int TotalImg, NumRotatedImg, NumShearXimg, NumShearYimg, NumScalingImg, NumWeightX, NumWeightY, NumFlip, NumGaussian;
//get value from user change
  if(InputValue == 0){ //horizontal
    RotatedValue = ui->horizontalSlider_Rotated->value();
    ScalingValue = ui->horizontalSlider_Scaling->value();
    WeightXvalue = ui->horizontalSlider_WeightX->value();
    WeightYvalue = ui->horizontalSlider_WeightY->value();
    ShearXvalue = ui->horizontalSlider_ShearX->value();
    limitShearX = ui->horizontalSlider_limitShearX->value();
    ShearYvalue = ui->horizontalSlider_ShearY->value();
    limitShearY = ui->horizontalSlider_limitShearY->value();
  }else if(InputValue == 1){ //spinbox
    RotatedValue = ui->spinBox_rotation->value();
    ScalingValue = ui->spinBox_Scaling->value();
    WeightXvalue = ui->spinBox_WeightX->value();
    WeightYvalue = ui->spinBox_WeightY->value();
    ShearXvalue = ui->spinBox_ShearX->value();
    limitShearX = ui->spinBox_limitShearX->value();
    ShearYvalue = ui->spinBox_ShearY->value();
    limitShearY = ui->spinBox_limitShearY->value();
  }

  int number_position = ui->spinBox_numberOfPosition->value();

  if(multi_output == 1){ //multioutput
    if(RotatedValue == 0){
      NumRotatedImg = 1;
    }else{
      NumRotatedImg = 360 / RotatedValue;
    }
    if(ScalingValue == 0){
      NumScalingImg = 1;
    }else{
      NumScalingImg = 2;
    }
    if(WeightXvalue == 0){
      NumWeightX = 1;
    }else{
      NumWeightX = 2;
    }
    if(WeightYvalue == 0){
      NumWeightY = 1;
    }else{
      NumWeightY = 2;
    }
    if(ShearXvalue == 0){
      NumShearXimg = 1;
    }else{
      NumShearXimg = (limitShearX / ShearXvalue) + 1;
    }
    if(ShearYvalue == 0){
      NumShearYimg = 1;
    }else{
      NumShearYimg = (limitShearY / ShearYvalue) + 1;
    }
    if(FlipValue == 1){
      NumFlip = 2;
    }else{
      NumFlip = 1;
    }
    if(GaussianValue == 1){
      NumGaussian = 2;
    }else{
      NumGaussian = 1;
    }
    TotalImg = number_position * NumRotatedImg * NumScalingImg * NumWeightX * NumWeightY * NumShearXimg *
               NumShearYimg * NumFlip * NumGaussian * number_bg * number_noise;

    ui->label_totalimg_num->setNum(TotalImg);

  }else if(multi_output == 0){ //Oneoutput
    if(RotatedValue == 0){
      NumRotatedImg = 1;
    }else{
      NumRotatedImg = 360 / RotatedValue;
    }
    if(ShearXvalue == 0){
      NumShearXimg = 1;
    }else{
      NumShearXimg = (limitShearX / ShearXvalue) + 1;
    }
    if(ShearYvalue == 0){
      NumShearYimg = 1;
    }else{
      NumShearYimg = (limitShearY / ShearYvalue) + 1;
    }

    TotalImg = number_position * number_bg * number_noise * NumRotatedImg * NumShearXimg * NumShearYimg;
    ui->label_totalimg_num->setNum(TotalImg);
  }
}

void DialogAugment:: on_comboBox_positionConfiguration_currentTextChanged(QString text){
  std::cout<< "text :: " << text.toStdString() << std::endl;
  if(text == "One position"){ //hide
    ui->label_numberOfPosition->setEnabled(false);
    ui->spinBox_numberOfPosition->setEnabled(false);
    ui->spinBox_numberOfPosition->setValue(1);
  }else{
    ui->label_numberOfPosition->setEnabled(true);
    ui->spinBox_numberOfPosition->setEnabled(true);
  }
}

void DialogAugment:: on_pushButton_Run_clicked(){
  QJsonObject param_js_data = saveState();
  bbbrunaugment myBrunAugment(param_js_data);
}
