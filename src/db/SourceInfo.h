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
  QString simplifiedRoot() const;
  QString fsRoot() const;
  bool isExternalMedia() const;
  bool isTemporaryLike() const; // e.g., /tmp and ~/Downloads
  bool isOtherUser() const; // true if any of the urls belongs to someone else
  bool isWritableLocation() const;
  bool isSingleFolder() const;
  bool isOnlyFolders() const;
  bool isSingleFile() const;
  bool isOnlyFiles() const;
  bool isEmpty() const;
public:
  static QString simplified(QString fn); // removes "/home/xxx/"
  static QString reconstructed(QString fn); // adds "/home/xxx/" if needed
  static QString fsRoot(QString fn); // finds top dir in same fs
  static QString commonRoot(QList<QUrl> const &);
  static QString commonRoot(QStringList const &);
  static bool isAllLocal(QList<QUrl> const &);
private:
  QList<QUrl> sources_;
  QString commonroot_;
  QStringList rootbits_;
};

#endif
