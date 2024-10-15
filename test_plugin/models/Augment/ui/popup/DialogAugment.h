#ifndef DialogAugment_H
#define DialogAugment_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>

#include <iostream>

//#include <cira_std_lib/cira_std_lib.hpp>

namespace Ui {
class DialogAugment;
}

class DialogAugment : public QDialog
{
  Q_OBJECT

public:
  explicit DialogAugment(QWidget *parent = 0);
  ~DialogAugment();

  void changeEvent(QEvent *ev) {
    if (ev->type() == QEvent::ActivationChange)
    {
        if(this->isActiveWindow())
        {

        }
        else
        {
          if(!isPin) {
            this->close();
          }
        }
    }
  }

  bool isPin = true;

  QString style_unpin = "background-color: transparent; border-image: url(:/test_plugin/icon/unpin.png); background: none; border: none; background-repeat: none;";
  QString style_pin = "background-color: transparent; border-image: url(:/test_plugin/icon/pin.png); background: none; border: none; background-repeat: none;";

  QJsonObject saveState();
  void restoreState(QJsonObject param_js_data);

  bool wait = false;

private slots:
  void on_horizontalSlider_Rotated_valueChanged(int value);
  void on_horizontalSlider_Scaling_valueChanged(int value);
  void on_horizontalSlider_WeightX_valueChanged(int value);
  void on_horizontalSlider_WeightY_valueChanged(int value);
  void on_horizontalSlider_ShearX_valueChanged(int value);
  void on_horizontalSlider_ShearY_valueChanged(int value);
  void on_pushButton_add_noise_clicked();
  void on_pushButton_add_BG_clicked();
  void on_pushButton_saveas_clicked();
  void on_horizontalSlider_limitShearX_valueChanged(int value);
  void on_horizontalSlider_limitShearY_valueChanged(int value);
  void on_checkBox_spinbox_clicked(bool checked);
  void claculateTotalImage();
  void on_comboBox_positionConfiguration_currentTextChanged(QString text);
  void on_pushButton_gtgenR2_clicked();
  void on_pushButton_Run_clicked();
//  void getFlow(std::shared_ptr<FlowData> flow);
  
private:
  Ui::DialogAugment *ui;
  QTimer * calculationTimer;
};

#endif // DialogAugment_H
