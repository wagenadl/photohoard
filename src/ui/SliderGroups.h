// SliderGroups.h

#ifndef SLIDERGROUPS_H

#define SLIDERGROUPS_H

#include <QStringList>
#include <QMap>

class SliderGroups {
public:
  struct SliderInfo {
    QString label;
    double dflt;
    double min, max;
    double mustep, step, pgstep;
    QString tip;
  };
public:
  SliderGroups();
  QStringList groups() const;
  QString groupLabel(QString group) const;
  QStringList allSliders() const;
  QStringList sliders(QString group) const;
  QString sliderLabel(QString slider) const;
  SliderInfo sliderInfo(QString slider) const;
private:
  QStringList groupnames;
  QMap<QString, QString> grouplabels;
  QMap<QString, QStringList> groupcontents;
  QMap<QString, SliderInfo> infos;
};

#endif
