// FileBar.h

#ifndef FILEBAR_H

#define FILEBAR_H

#include "ActionBar.h"

class FileBar: public ActionBar {
public:
  FileBar(class SessionDB *db, class AutoCache *ac,
          class Exporter *exporter, class Scanner *scanner,
          class MainWindow *parent);
  class SliderClipboard *sliderClipboard() const { return sliderclip; }
private:
  class ExportDialog *exportdialog;
  class SliderClipboard *sliderclip;
};

#endif
