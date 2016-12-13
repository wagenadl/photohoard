// Settings.h

#ifndef SETTINGS_H

#define SETTINGS_H

#include <QVariant>

class Settings {
public:
  Settings();
  ~Settings();
  void set(QString key, QVariant value);
  QVariant get(QString key) const;
  QVariant get(QString key, QVariant dflt); // sets if not found
  bool contains(QString key) const;
private:
  class QSettings *data;
};

#endif
