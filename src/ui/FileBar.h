// FileBar.h

#ifndef FILEBAR_H

#define FILEBAR_H

#include <QToolBar>
#include "Action.h"

class FileBar: public QToolBar {
public:
  FileBar(class PhotoDB *db,
          class Exporter *exporter, class Scanner *scanner,
          QWidget *parent);
private:
  class ExportDialog *exportdialog;
  Actions actions;
};

#endif
