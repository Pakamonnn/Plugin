#ifndef BGRtoHSVModel_HPP
#define BGRtoHSVModel_HPP

#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QShowEvent>

#include <cira_lib_bernoulli/general/GlobalData.hpp>

#include <nodes/NodeDataModel>

#include <iostream>

#include "ui/FormBGRtoHSV.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

class FlowData;

class BGRtoHSVModel : public NodeDataModel
{

  Q_OBJECT

public:
  BGRtoHSVModel();

  virtual
  ~BGRtoHSVModel() {}

  int portInStatus[1] = {PORTSTATUS::DISCONNECTED};

public:

  QString
  caption() const override
  { return QStringLiteral("BGRtoHSV"); }

  bool
  captionVisible() const override
  { return true; }

  bool
  portCaptionVisible(PortType, PortIndex) const override
  { return true; }

  bool
  resizable() const override { return false; }


  QString
  name() const override
  { return QStringLiteral("BGRtoHSV"); }

public:

  QJsonObject
  save() const override;

  void
  restore(QJsonObject const &p) override;

public:

  unsigned int
  nPorts(PortType portType) const override;

  NodeDataType
  dataType(PortType portType, PortIndex portIndex) const override;

  std::shared_ptr<NodeData>
  outData(PortIndex port) override;

  void
  setInData(std::shared_ptr<NodeData> data, PortIndex portIndex) override;

  QWidget *
  embeddedWidget() override { return form; }

private:

  bool isBusy = false;
  std::shared_ptr<FlowData> _flowDataOut;
  FormBGRtoHSV *form;

  QTimer *timerLoadFromButton;

private slots:
  void runProcess(std::shared_ptr<FlowData> _flowDataIn);
  void loopTimerLoadFromButton();

};

#endif // BGRtoHSVModel_HPP
