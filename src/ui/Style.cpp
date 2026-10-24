// Style.cpp

#include "Style.h"
#include <QFile>
#include "PDebug.h"
#include <QApplication>
#include <QJsonDocument>
#include "Slide.h"
#include <QIcon>

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
  //  QFile f(darkmode ? ":dark.css" : ":light.css");
  QFile f(darkmode ? "/home/wagenaar/progs/photohoard/photohoard/res/dark.css" : ":light.css");
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
  
QString Style::option(QString name) const {
  QString x = options[name].toString();
  return x;
}

QColor Style::color(QString name) const {
  QString x = options[name].toString();
  return QColor(x);
}

QIcon Style::icon(QString name) const {
  if (darkmode) {
    QString darkfile = QString(":icons/%1-dark.svg").arg(name);
    if (QFile::exists(darkfile))
      return QIcon(darkfile);
  }
  QString lightfile = QString(":icons/%1.svg").arg(name);
  return QIcon(lightfile);
}
