// SourceInfo.h

#ifndef SOURCEINFO_H

#define SOURCEINFO_H

#include <QString>
#include <QList>
#include <QUrl>

class SourceInfo {
public:
  SourceInfo(QList<QUrl> const &);
  QList<QUrl> const &sources() const { return sources_; }
  QString commonRoot() const;
  QString fsRoot() const;
  bool isExternalMedia() const;
  bool isWritableLocation() const;
  bool isSingleFolder() const;
  bool involvesOnlyFiles() const;
public:
  static QString fsRoot(QString fn); // finds top dir in same fs
  static QString commonRoot(QList<QUrl> const &);
  static QString commonRoot(QStringList const &);
private:
  QList<QUrl> sources_;
  QString commonroot_;
};

#endif
