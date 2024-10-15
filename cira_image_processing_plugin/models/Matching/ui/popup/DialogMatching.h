#ifndef DialogMatching_H
#define DialogMatching_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>
#include <iostream>

namespace Ui {
class DialogMatching;
}

class DialogMatching : public QDialog
{
  Q_OBJECT

public:

  explicit DialogMatching(QWidget *parent = 0);  
  ~DialogMatching();

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
  QStringList array_img;
  QString path_img;
  void restoreState(QJsonObject param_js_data);


private slots:
  void on_pushButton_clicked();

  void on_pushButton_2_clicked();

private:
  Ui::DialogMatching *ui;

};

#endif // DialogMatching_H
