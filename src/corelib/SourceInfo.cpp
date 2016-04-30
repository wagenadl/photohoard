// SourceInfo.cpp

#include "SourceInfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <QFileInfo>
#include <QStringList>

SourceInfo::SourceInfo(QList<QUrl> const &urls): sources_(urls) {
  commonroot_ = commonRoot(urls);
}

QString SourceInfo::fsRoot(QString fn) {
  dev_t dev0 = 0;
  QFileInfo fi(fn);
  while (true) {
    QString dir = fi.path();
    if (dir==fn)
      return fn;
    QByteArray ba = dir.toLatin1();
    struct stat s;
    if (::stat(ba.data(), &s) < 0) {
      perror("stat failed");
      return fn;
    }
    if (dev0==0)
      dev0 = s.st_dev;
    else if (s.st_dev != dev0)
      return fn;
    fn = dir;
  }
  return fn; // not reached
};

QString SourceInfo::fsRoot() const {
  return fsRoot(commonRoot());
}

QString SourceInfo::commonRoot() const {
  return commonroot_;
}

QString SourceInfo::commonRoot(QList<QUrl> const &urls) {
  QStringList paths;
  for (QUrl const &url: urls) {
    if (url.isLocalFile()) {
      paths << url.path();
    }
  }
  return commonRoot(paths);
}

QString SourceInfo::commonRoot(QStringList const &paths) {
  QStringList parts;
  bool any = false;
  for (QString const &path: paths) {
    QStringList p0 = path.split("/");
    if (!p0.isEmpty() && p0.last().contains("."))
      p0.takeLast();
    if (any) {
      while (p0.size()>parts.size())
        p0.takeLast();
      while (parts.size()>p0.size())
        parts.takeLast();
      while (!parts.isEmpty()) {
        if (p0==parts)
          break;
        p0.takeLast();
        parts.takeLast();
      }
    } else {
      parts = p0;
      any = true;
    }
  }
  return parts.join("/");
}

bool SourceInfo::isExternalMedia() const {
  return commonroot_.startsWith("/media/")
    && commonroot_.count("/")>=3;
}
