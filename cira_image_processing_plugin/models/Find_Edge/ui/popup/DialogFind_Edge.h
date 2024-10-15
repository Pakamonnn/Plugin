#ifndef DialogFind_Edge_H
#define DialogFind_Edge_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>

#include <iostream>

namespace Ui {
class DialogFind_Edge;
}

class DialogFind_Edge : public QDialog
{
  Q_OBJECT

public:
  explicit DialogFind_Edge(QWidget *parent = 0);
  ~DialogFind_Edge();

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
  void on_spinBox_valueChanged(int arg1);

private:
  Ui::DialogFind_Edge *ui;
};

#endif // DialogFind_Edge_H
