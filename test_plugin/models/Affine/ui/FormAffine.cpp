#include "FormAffine.h"
#include "ui_FormAffine.h"

FormAffine::FormAffine(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FormAffine)
{
  ui->setupUi(this);

  timestamp_base = GlobalData::timer.nsecsElapsed();
  nodeStatus_ready = true;

  workerThread = new ThreadAffine();
  dialog = new DialogAffine();

  mv_node_run = new QMovie(":/test_plugin/icon/run_led.gif");
  update_ui();
  connect(GlobalData::GlobalDataObject, SIGNAL(stopAllScene()), this, SLOT(stop_node_process()));
}

FormAffine::~FormAffine()
{
  delete ui;
}

void FormAffine::on_pushButton_nodeEnable_clicked()
{
  if(nodeStatus_enable) {
    ui->pushButton_nodeEnable->setStyleSheet(style_nodeDisable);
    nodeStatus_enable = false;
  } else {
    ui->pushButton_nodeEnable->setStyleSheet(style_nodeEnable);
    timestamp_base = GlobalData::timer.nsecsElapsed();
    nodeStatus_enable = true;
  }
}

void FormAffine::on_pushButton_prop_clicked()
{
#ifdef WIN32
   dialog->setWindowModality(Qt::NonModal); dialog->setParent(GlobalData::parent);
   if(ui->label->pixmap()) dialog->setWindowIcon(QIcon(   *ui->label->pixmap()   ));
   //dialog->setWindowFlags( Qt::Window |  Qt::WindowCloseButtonHint );
   dialog->setWindowFlags( Qt::Window );
#else
  dialog->setWindowModality(Qt::WindowModal);
  dialog->setWindowFlags( Qt::WindowCloseButtonHint );
#endif

  QScreen *scr = qApp->screenAt(QCursor::pos());
  QPoint p = QCursor::pos();
  int w = scr->geometry().x() + scr->geometry().width(); int h = scr->geometry().y() + scr->geometry().height();
  int x = p.x(); int y = p.y();
  if(x + dialog->width() > w) {
    x -= x + dialog->width() - w;
  }
  if(y + dialog->height() > h) {
    y -= y + dialog->height() - h;
  }

  dialog->move(x, y);
  dialog->setVisible(true);
}
