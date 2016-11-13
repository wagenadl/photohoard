// Settings.cpp

#include "Settings.h"

#include <QSettings>

Settings::Settings() {
  data = new QSettings("danielwagenaar.net", "photohoard");
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

