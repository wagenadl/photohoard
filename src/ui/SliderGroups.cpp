// SliderGroups.cpp

#include "SliderGroups.h"
#include <QDebug>
#include <QFile>
#include <QApplication>
#include "Sliders.h"

SliderGroups::SliderGroups() {
  QFile src(":/sliders.txt");
  if (!src.open(QFile::ReadOnly)) {
    qDebug() << "Cannot open control defs";
    Q_ASSERT(0);
  }
  QTextStream ts(&src);

  QString currentgroup;
  QString currentslider;
  
  while (!ts.atEnd()) {
    QString line = ts.readLine().simplified();
    line.replace(QRegExp("#.*"),"");
    if (line=="")
      continue;
    if (line.contains("::")) {
      // Defining a group
      int idx = line.indexOf("::");
      currentgroup = line.left(idx).simplified();
      groupnames << currentgroup;
      grouplabels[currentgroup] = line.mid(idx+2).simplified();
      groupcontents[currentgroup] = QStringList();
      currentslider = "";
    } else if (line.contains(":")) {
      // Defining a slider
      Q_ASSERT(!currentgroup.isEmpty());
      int idx = line.indexOf(":");
      currentslider = line.left(idx).simplified();
      groupcontents[currentgroup] << currentslider;
      Q_ASSERT(Sliders::defaults().contains(name));
      SliderInfo info;
      info.label = line.mid(idx+1).simplified();
      info.label.replace("~", " ");
      info.dflt = Sliders::defaultFor(currentslider);
      infos[currentslider] = info;
    } else if (line.contains("/")) {
      Q_ASSERT(!currentslider.isEmpty());
      // set min max and steps
      QStringList lst = line.split(" ");
      if (lst.size()==6 && lst[2]=="/") {
	SliderInfo &info = infos[currentslider];
        info.min = lst[0].toDouble();
        info.max = lst[1].toDouble();
        info.mustep = lst[3].toDouble();
        info.step = lst[4].toDouble();
        info.pgstep = lst[5].toDouble();
      } else {
        qDebug() << "Syntax error:" << line;
	Q_ASSERT(0);
      }
    } else if (line.contains(" ")) {
      Q_ASSERT(!currentslider.isEmpty());
      // set balloon
      infos[currentslider].tip = line;
    } else {
      qDebug() << "Syntax error:" << line;
      Q_ASSERT(0);
    }       
  }
}

QStringList SliderGroups::groups() const {
  return groupnames;
}

QString SliderGroups::groupLabel(QString group) const {
  Q_ASSERT(grouplabels.contains(group));
  return grouplabels[group];
}

QStringList SliderGroups::allSliders() const {
  QStringList sli;
  foreach (QString grp, groupnames)
    sli.append(groupcontents[grp]);
  return sli;
}

QStringList SliderGroups::sliders(QString group) const {
  Q_ASSERT(groupcontents.contains(group));
  return groupcontents[group];
}

QString SliderGroups::sliderLabel(QString slider) const {
  Q_ASSERT(infos.contains(slider));
  return infos[slider].label;
}

SliderGroups::SliderInfo SliderGroups::sliderInfo(QString slider) const {
  Q_ASSERT(infos.contains(slider));
  return infos[slider];
}