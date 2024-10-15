#ifndef test_plugin_HPP
#define test_plugin_HPP

#include <cira_lib_bernoulli/plugin/ciraloader_base.h>

class test_plugin : public CiRALoader
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "teerawat.CiRACORE.CiRALoader")
  Q_INTERFACES(CiRALoader)

public:
  test_plugin();

  virtual std::shared_ptr<DataModelRegistry> registerDataModels(std::shared_ptr<DataModelRegistry> ret) override;

  virtual void setTreeWidget() override;

  virtual const QString name() const override
  {
    return info()["name"].toString();
  }

  virtual const QJsonObject info() const override
  {
    QJsonObject jso;
    jso["name"] = "test_plugin";
    jso["version"] = "0.0.0";
    jso["maintainer"] = "anonymous";
    jso["email"] = "";

    return jso;
  }

};

#endif // test_plugin_HPP
