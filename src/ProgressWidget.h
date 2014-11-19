// ProgressWidget.h

#ifndef PROGRESSWIDGET_H

#define PROGRESSWIDGET_H

#include <QLabel>

class ProgressWidget: public QLabel {
  Q_OBJECT;
public:
  ProgressWidget(QString title, QWidget *parent=0);
public slots:
  void markProgress(int n, int N);
private:
  QString title;
};

#endif
