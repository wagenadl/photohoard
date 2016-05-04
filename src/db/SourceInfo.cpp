// SourceInfo.cpp

#include "SourceInfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <QFileInfo>
#include <QStringList>
#include "PDebug.h"

SourceInfo::SourceInfo(QList<QUrl> const &urls) {
  for (QUrl const &url: urls) 
    if (url.isLocalFile())
      sources_ << url;
  commonroot_ = commonRoot(urls);
}

bool SourceInfo::isAllLocal(QList<QUrl> const &urls) {
  for (QUrl const &url: urls) 
    if (!url.isLocalFile())
      return false;
  return true;
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
  for (QUrl const &url: urls) 
    paths << url.path();
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

bool SourceInfo::isTemporaryLike() const {
  return commonroot_.startsWith("/tmp")
    || commonroot_.contains("/Downloads");
}

QString SourceInfo::homeDirectory() {
  static QString home(QString(qgetenv("HOME")));
  return home;
}

static uid_t myUid() {
  static bool have = false;
  static uid_t uid = 0;
  if (!have) {
    struct stat s;
    QByteArray ba = SourceInfo::homeDirectory().toLatin1();
    if (::stat(ba.data(), &s) < 0) {
      perror("stat failed");
      CRASH("stat failed");
    }
    uid = s.st_uid;
    have = true;
  }
  return uid;
}

bool SourceInfo::isOtherUser() const {
  for (QUrl const &url: sources_) {
    QByteArray ba = url.path().toLatin1();
    struct stat s;
    if (::stat(ba.data(), &s) < 0) {
      perror("stat failed");
      COMPLAIN("stat failed");
      return false;
    } else {
      if (s.st_uid != myUid())
	return true;
    }
  }
  return false;
}

QString SourceInfo::simplifiedRoot() const {
  return simplified(commonroot_);
}

QString SourceInfo::reconstructed(QString fn) {
  if (fn.startsWith("/"))
    return fn;
  else
    return homeDirectory() + "/" + fn;
}

QString SourceInfo::simplified(QString fn) {
  QString home = homeDirectory();
  if (fn==home)
    return "Home directory";
  else if (fn.startsWith(home))
    return fn.mid(home.size() + 1);
  else
    return fn;
}

bool SourceInfo::isWritableLocation() const {
  QFileInfo fi(commonroot_);
  return fi.isWritable();
}

bool SourceInfo::isSingleFolder() const {
  if (sources_.size() == 1)
    return QFileInfo(sources_.first().path()).isDir();
  else
    return false;
}

bool SourceInfo::isOnlyFolders() const {
  for (QUrl const &url: sources_)
    if (!QFileInfo(url.path()).isDir())
      return false;
  return true;
}
  
bool SourceInfo::isSingleFile() const {
  if (sources_.size() == 1)
    return QFileInfo(sources_.first().path()).isFile();
  else
    return false;
}

bool SourceInfo::isOnlyFiles() const {
  for (QUrl const &url: sources_)
    if (!QFileInfo(url.path()).isFile())
      return false;
  return true;
}

bool SourceInfo::isEmpty() const {
  return sources_.isEmpty();
}
