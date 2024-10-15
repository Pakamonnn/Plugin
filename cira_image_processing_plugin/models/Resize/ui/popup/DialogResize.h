#ifndef DialogResize_H
#define DialogResize_H

#include <QDialog>
#include <QJsonObject>
#include <QEvent>

#include <iostream>

namespace Ui {
class DialogResize;
}

class DialogResize : public QDialog
{
  Q_OBJECT

public:
  explicit DialogResize(QWidget *parent = 0);
  ~DialogResize();

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


private:
  Ui::DialogResize *ui;
};

#endif // DialogResize_H
