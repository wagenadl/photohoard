// Runcontrol.cpp

#include "RunControl.h"
#include <QFileInfo>
#include <QString>
#include <QFile>
#include <QApplication>
#include "PDebug.h"

bool RunControl::isRunning(quint64 pid) {
  QString fn = QString("/proc/%1").arg(pid);
  bool active = QFile(fn).exists();
  if (!active)
    return false;
  QString exe = QFileInfo(fn + "/exe").symLinkTarget();
  return exe.contains("photohoard");
}

quint64 RunControl::pid() {
  return QApplication::applicationPid();
}

  
  
