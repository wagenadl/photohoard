// FileBar.h

#ifndef FILEBAR_H

#define FILEBAR_H

#include "ActionBar.h"

class FileBar: public ActionBar {
public:
  FileBar(class PhotoDB *db,
          class Exporter *exporter, class Scanner *scanner,
          class MainWindow *parent);
private:
  class ExportDialog *exportdialog;
};

#endif
