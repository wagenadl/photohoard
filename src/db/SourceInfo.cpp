// SourceInfo.cpp

#include "SourceInfo.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <QFileInfo>
#include <QStringList>
#include "PDebug.h"
#include <QDir>
#include <QStorageInfo>

SourceInfo::SourceInfo(QList<QUrl> const &urls) {
  for (QUrl const &url: urls) 
    if (url.isLocalFile())
      sources_ << url;
  commonroot_ = commonRoot(urls);
  rootbits_ = commonroot_.split("/");
}

bool SourceInfo::isAllLocal(QList<QUrl> const &urls) {
  for (QUrl const &url: urls) 
    if (!url.isLocalFile())
      return false;
  return true;
}  

QString SourceInfo::fsRoot(QString fn) {
  QFileInfo fi(fn);
  QDir dir(fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath());
  QByteArray dev = QStorageInfo(dir).device();
  while (true) {
    QDir parent = dir;
    if (!parent.cdUp())
      return dir.absolutePath();
    if (QStorageInfo(parent).device()!=dev)
      return dir.absolutePath();
    dir = parent;
  }
}

QString SourceInfo::fsRoot() const {
  return fsRoot(commonRoot());
}

QString SourceInfo::commonRoot() const {
  return commonroot_;
}

QString SourceInfo::commonRoot(QList<QUrl> const &urls) {
  QStringList paths;
  for (QUrl const &url: urls) 
    paths << url.toLocalFile();
  return commonRoot(paths);
}

QString SourceInfo::commonRoot(QStringList const &paths) {
  if (paths.isEmpty())
    return QString();
  if (paths.size()==1) {
    QFileInfo fi(paths[0]);
    return fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
  }
  
  QStringList parts = paths[0].split("/");
  for (QString const &path: paths) {
    QStringList p0 = path.split("/");
    while (p0.size()>parts.size())
      p0.removeLast();
    while (parts.size()>p0.size())
      parts.removeLast();
    while (p0!=parts) {
      p0.removeLast();
      parts.removeLast();
    }
  }
  return parts.join("/");
}

bool SourceInfo::isExternalMedia() const {
  return rootbits_.size()>=3 && rootbits_[0]=="media";
  /* On Windows, the "getdrivetypea" function should probably be used:
     https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getdrivetypea
     On Mac, I don't yet know.
  */
}

bool SourceInfo::isTemporaryLike() const {
  return rootbits_.contains("tmp")
    || rootbits_.contains("Downloads")
    || rootbits_.contains("temp")
    || rootbits_.contains("Temp");
}

bool SourceInfo::isOtherUser() const {
  int me = QFileInfo(QDir::homePath()).ownerId();
  for (QUrl const &url: sources_)
    if (QFileInfo(url.toLocalFile()).ownerId() != me)
      return true;
  return false;
}

QString SourceInfo::simplifiedRoot() const {
  return simplified(commonroot_);
}

QString SourceInfo::reconstructed(QString fn) {
  if (fn=="Home directory")
    return QDir::homePath();
  else if (fn.startsWith("/"))
    return fn;
  else
    return QDir::homePath() + "/" + fn;
}

QString SourceInfo::simplified(QString fn) {
  QString home = QDir::homePath();
  if (fn==home)
    return "Home directory";
  else if (fn.startsWith(home))
    return fn.mid(home.size() + 1);
  else
    return fn;
}

bool SourceInfo::isWritableLocation() const {
  return QFileInfo(commonroot_).isWritable();
}

bool SourceInfo::isSingleFolder() const {
  if (sources_.size() == 1)
    return QFileInfo(sources_[0].toLocalFile()).isDir();
  else
    return false;
}

bool SourceInfo::isOnlyFolders() const {
  for (QUrl const &url: sources_)
    if (!QFileInfo(url.toLocalFile()).isDir())
      return false;
  return true;
}
  
bool SourceInfo::isSingleFile() const {
  if (sources_.size() == 1)
    return QFileInfo(sources_[0].toLocalFile()).isFile();
  else
    return false;
}

bool SourceInfo::isOnlyFiles() const {
  for (QUrl const &url: sources_)
    if (!QFileInfo(url.toLocalFile()).isFile())
      return false;
  return true;
}

bool SourceInfo::isEmpty() const {
  return sources_.isEmpty();
}
