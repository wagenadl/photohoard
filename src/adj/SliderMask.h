// SliderMask.h

#ifndef SLIDERMASK_H

#define SLIDERMASK_H

#include <QSet>

class SliderMask {
public:
  SliderMask();
  void enable(QString key, bool on=true);
  void disable(QString, bool off=true);
  bool isEnabled(QString key) const;
  QSet<QString> mask() const;
private:
  QSet<QString> keys;
private:
  static QSet<QString> const &universe();
};

#endif
