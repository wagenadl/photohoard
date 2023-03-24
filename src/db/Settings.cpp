// Settings.cpp

#include "Settings.h"
#include <QFileInfo>
#include <QSettings>

Settings::Settings() {
  data = new QSettings("photohoard", "photohoard");
}

Settings::~Settings() {
  delete data;
}

void Settings::set(QString key, QVariant value) {
  data->setValue(key, value);
  //  data->sync();
}
  
QVariant Settings::get(QString key) const {
  return data->value(key);
}

QVariant Settings::get(QString key, QVariant dflt) {
  if (!contains(key))
    set(key, dflt);
  return get(key);
}

bool Settings::contains(QString key) const {
  return data->contains(key);
}

QStringList Settings::recentFiles() const {
  QStringList fns = get("recentdbs").toStringList();
  QStringList res;
  for (QString const &fn: fns)
    if (QFileInfo(fn).exists())
      res << fn;
  return res;
}

void Settings::markRecentFile(QString fn) {
  QStringList fns = recentFiles();
  fns.removeAll(fn);
  fns.insert(0, fn);
  while (fns.size()>20)
    fns.removeLast();
  set("recentdbs", fns);
}
