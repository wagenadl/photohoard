// Style.cpp

#include "Style.h"
#include <QFile>
#include "PDebug.h"
#include <QApplication>
#include <QJsonDocument>
#include "Slide.h"

Style *Style::instance() {
  static Style style;
  return &style;
}

Style::Style() {
}

void Style::setDarkMode() {
  darkmode = true;
  updateAppStyle();
}

void Style::setLightMode() {
  darkmode = false;
  updateAppStyle();
}

void Style::updateAppStyle() {
  QFile f(darkmode ? ":dark.css" : ":light.css");
  ASSERT(f.open(QFile::ReadOnly));
  QApplication *app
    = dynamic_cast<QApplication *>(QApplication::instance());
  if (app)
    app->setStyleSheet(QString::fromUtf8(f.readAll()));

  QFile j(darkmode ? ":dark.json" : ":light.json");
  ASSERT(j.open(QFile::ReadOnly));
  QJsonDocument jdoc(QJsonDocument::fromJson(j.readAll()));
  options = jdoc.toVariant().toMap();

  QFile d(darkmode ? ":darkdoc.css" : ":lightdoc.css");
  ASSERT(d.open(QFile::ReadOnly));
  docstyle = QString::fromUtf8(d.readAll());
  
  //
  emit styleChanged();
  Slide::updatePalette();
}

QString Style::documentStyle() const {
  return docstyle;
}
  
QColor Style::color(QString name) const {
  QString x = options[name].toString();
  qDebug() << "color" << name << x << QColor(x);
  return QColor(x);
}
