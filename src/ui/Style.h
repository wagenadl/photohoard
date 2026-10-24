// Style.h

#ifndef STYLE_H

#define STYLE_H

#include <QString>
#include <QObject>
#include <QVariantMap>

class Style: public QObject {
  Q_OBJECT;
public:
  static Style *instance();
signals:
  void styleChanged();
public:
  Style();
  void setDarkMode();
  void setLightMode();
  QString documentStyle() const;
  QString option(QString name) const;
  QColor color(QString name) const;
  bool isDark() const { return darkmode; }
  QIcon icon(QString name) const;
private:
  void updateAppStyle();
private:
  bool darkmode;
  QVariantMap options;
  QString docstyle;
};

#endif
