#ifndef DialogColorDetect_H
#define DialogColorDetect_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>

#include <iostream>

namespace Ui {
class DialogColorDetect;
}

class DialogColorDetect : public QDialog
{
  Q_OBJECT

public:
  explicit DialogColorDetect(QWidget *parent = 0);
  ~DialogColorDetect();

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

  QString style_unpin = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/unpin.png); background: none; border: none; background-repeat: none;";
  QString style_pin = "background-color: transparent; border-image: url(:/cira_image_processing_plugin/icon/pin.png); background: none; border: none; background-repeat: none;";

  QJsonObject saveState();
  void restoreState(QJsonObject param_js_data);

private slots:
  void on_groupBox_clicked(bool checked);
  void on_horizontalSlider_valueChanged(int value);
  void on_horizontalSlider_3_valueChanged(int value);
  void on_horizontalSlider_6_valueChanged(int value);
  void on_horizontalSlider_2_valueChanged(int value);
  void on_horizontalSlider_4_valueChanged(int value);

  void on_horizontalSlider_5_valueChanged(int value);

private:
  Ui::DialogColorDetect *ui;
};

#endif // DialogColerDetection_H
