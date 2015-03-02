// AllControls.cpp

#include "AllControls.h"
#include "ControlGroup.h"
#include "GentleJog.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QBoxLayout>
#include <QSignalMapper>
#include "Sliders.h"

AllControls::AllControls(QWidget *parent): QFrame(parent) {
  QSignalMapper *mapper = new QSignalMapper(this);
  connect(mapper, SIGNAL(mapped(QString)), SLOT(valueChange(QString)));
  
  QFile src("/home/wagenaar/progs/photohoard/trunk/res/sliders.txt");
  if (!src.open(QFile::ReadOnly)) {
    qDebug() << "Cannot open control defs";
    return;
  }

  QVBoxLayout *vl = new QVBoxLayout;

  ControlGroup *currentgroup = 0;
  GentleJog *currentjog = 0;

  QTextStream ts(&src);
  while (!ts.atEnd()) {
    QString line = ts.readLine().simplified();
    line.replace(QRegExp("#.*"),"");
    if (line=="")
      continue;
    if (line.contains("::")) {
      int idx = line.indexOf("::");
      QString name = line.left(idx).simplified();
      QString label = line.mid(idx+2).simplified();
      currentgroup = new ControlGroup(label);
      vl->addWidget(currentgroup);
      groups[name] = currentgroup;
    } else if (line.contains(":")) {
      Q_ASSERT(currentgroup);
      int idx = line.indexOf(":");
      QString name = line.left(idx).simplified();
      Q_ASSERT(Sliders::defaults().contains(name));
      QString label = line.mid(idx+1).simplified();
      currentjog = new GentleJog(label);
      double v = Sliders::defaultFor(name);
      currentjog->setDefault(v);
      currentjog->setValue(v);
      connect(currentjog, SIGNAL(valueChanged(double)),
              mapper, SLOT(map()));
      mapper->setMapping(currentjog, name);
      currentgroup->addWidget(currentjog);
      jogs[name] = currentjog;
    } else if (line.contains("/")) {
      Q_ASSERT(currentjog);
      // set min max and steps
      QStringList lst = line.split(" ");
      if (lst.size()==6 && lst[2]=="/") {
        currentjog->setMinimum(lst[0].toDouble());
        currentjog->setMaximum(lst[1].toDouble());
        currentjog->setMicroStep(lst[3].toDouble());
        currentjog->setSingleStep(lst[4].toDouble());
        currentjog->setPageStep(lst[5].toDouble());
        currentjog->setMaxDelta(2*lst[5].toDouble());
        currentjog->setValue(currentjog->defaultValue());
        if (currentjog->microStep()>=1)
          currentjog->setDecimals(0);
        else
          currentjog->setDecimals(-floor(log10(currentjog->microStep())));
      } else {
        qDebug() << "Syntax error:" << line;
      }
    } else if (line.contains(" ")) {
      Q_ASSERT(currentjog);
      // set balloon
      currentjog->setToolTip(line);
    } else {
      qDebug() << "Syntax error:" << line;
    }       
  }
  setLayout(vl);

  for (auto cg: groups)
    cg->expand();

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

AllControls::~AllControls() {
}

ControlGroup *AllControls::group(QString name) const {
  return groups.contains(name) ? groups[name] : 0;
}

GentleJog *AllControls::jog(QString name) const {
  return jogs.contains(name) ? jogs[name] : 0;
}

double AllControls::get(QString name) const {
  GentleJog *j = jog(name);
  if (j)
    return j->value();
  else
    return nan("");
}

void AllControls::setQuietly(QMap<QString, double> const &vv) {
  for (auto it=vv.begin(); it!=vv.end(); it++)
    setQuietly(it.key(), it.value());
}

bool AllControls::setQuietly(QString name, double value) {
  GentleJog *j = jog(name);
  if (j) {
    j->setValueQuietly(value);
    return j->value()==value;
  } else {
    return false;
  }
}
  

bool AllControls::set(QString name, double value) {
  GentleJog *j = jog(name);
  if (j) {
    j->setValue(value);
    return j->value()==value;
  } else {
    return false;
  }
}

void AllControls::valueChange(QString name) {
  double value = get(name);
  qDebug() << "ALLCONTROLS: " << name << value;
  emit valueChanged(name, value);
}
