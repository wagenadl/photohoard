// LightTable.h

#ifndef LIGHTTABLE_H

#define LIGHTTABLE_H

#include <QSplitter>
#include <QPointer>

#include "PhotoDB.h"
#include "Strip.h"
#include "LayoutBar.h"
#include "ColorLabelBar.h"
#include "FilterBar.h"
#include "LiveAdjuster.h"

class LightTable: public QSplitter {
  Q_OBJECT;
public:
  LightTable(PhotoDB *db, LiveAdjuster *adjuster,
             QWidget *parent=0);
  virtual ~LightTable();
  quint64 current() const { return curr; }
  PSize displaySize() const;
public slots:
  void setLayout(LayoutBar::Action ar);
  void slidePress(quint64 vsn, Qt::MouseButton, Qt::KeyboardModifiers);
  void select(quint64 vsn, Qt::KeyboardModifiers=Qt::NoModifier);
  void updateImage(quint64 vsn, Image16 img, quint64 chgid);
  void rescan(bool rebuildFilter=true);
  void setColorLabelEtc(ColorLabelBar::Action);
  void filterAction(FilterBar::Action);
  void clearSelection();
  void selectAll();
  void bgPress(Qt::MouseButton, Qt::KeyboardModifiers);
  void scrollToCurrent();
  void rotateSelected(int); // in steps of 90 degrees
signals:
  void needImage(quint64, QSize);
  void newCurrent(quint64);
  void newSlideSize(QSize);
  void newZoom(double);
  void newSelection();
  void newCollection(QString);
  void recacheReoriented(QSet<quint64>);
private slots:
  void requestLargerImage();
  void updateAdjusted(Image16, quint64);
  void applyFilterFromDialog();
  void resizeStrip();
protected:
  void updateSlide(quint64 id);
  void ensureReasonableGridSize();
  void populateFilterFromDialog();
  void selectNearestInFilter(quint64 vsn);
  void toggleSelection(quint64 i);
  void extendOrShrinkSelection(quint64 i);
  void simpleSelection(quint64 i, bool keepIfContained);
  void makeCurrent(quint64 i);
protected:
  PhotoDB *db;
  QPointer<LiveAdjuster> adjuster; // we do not own
  class Selection *selection;
  class StripView *strips;
  class SlideView *slide;
  class FilterDialog *filterDialog;
  bool showmax;
  LayoutBar::Action lastlay, lay;
  quint64 curr;
  int tilesize;
  int lastgridsize;
};

#endif
