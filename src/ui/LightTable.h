// LightTable.h

#ifndef LIGHTTABLE_H

#define LIGHTTABLE_H

#include <QSplitter>
#include <QPointer>

#include "SessionDB.h"
#include "Strip.h"
#include "LayoutBar.h"
#include "ColorLabelBar.h"
#include "FilterBar.h"
#include "LiveAdjuster.h"
#include "Filter.h"
#include "Action.h"

class LightTable: public QSplitter {
  Q_OBJECT;
public:
  LightTable(SessionDB *db, class AutoCache *cache,
             LiveAdjuster *adjuster, class Exporter *expo,
             QWidget *parent=0);
  virtual ~LightTable();
  PSize displaySize() const;
  Actions const &actions() const;
  class Filter const &filter() const;
public slots:
  void setLayout(LayoutBar::Layout ar);
  void slidePress(quint64 vsn, Qt::MouseButton, Qt::KeyboardModifiers);
  void select(quint64 vsn, Qt::KeyboardModifiers=Qt::NoModifier);
  void updateImage(quint64 vsn, Image16 img, quint64 chgid, QSize fullsize);
  void rescan(bool rebuildFilter=true);
  void clearSelection();
  void selectAll();
  void bgPress(Qt::MouseButton, Qt::KeyboardModifiers);
  void scrollToCurrent();
  void rotateSelected(int); // in steps of 90 degrees
  void openFilterDialog();
  void increaseTileSize(double factor);
  void updateSelectedTiles();
  void reloadVersion(quint64 vsn);
  void ensureDragExportComplete();
  void visualizeLayer(quint64 vsn, int layer);
  void applyFilter(Filter);
  void applyFilterFromDialog();
signals:
  void needImage(quint64, QSize);
  void wantImage(quint64, QSize);
  void newCurrent(quint64);
  void newSlideSize(QSize);
  void newZoom(double);
  void newSelection();
  void newCollection(QString);
  void recacheReoriented(QSet<quint64>);
private slots:
  void updateAdjusted(Image16, quint64, QSize fullsize);
  void resizeStrip();
  void startDrag(quint64);
  void saveSplitterPos();
private:
  void makeActions();
protected:
  virtual void keyPressEvent(QKeyEvent *) override;
  void updateSlide(quint64 id);
  void ensureReasonableGridSize();
  void populateFilterFromDialog();
  void selectNearestInFilter(quint64 vsn);
  void toggleSelection(quint64 i);
  void extendOrShrinkSelection(quint64 i);
  void simpleSelection(quint64 i, bool keepIfContained);
  void makeCurrent(quint64 i);
  void ensureSlideShown();
  void updateMainSlide();
protected:
  SessionDB *db;
  AutoCache *cache;
  class Exporter *exporter;
  QPointer<LiveAdjuster> adjuster; // we do not own
  class Selection *selection;
  class StripView *strips;
  class SlideView *slide;
  class FilterDialog *filterDialog;
  bool showmax;
  LayoutBar::Layout lastlay, lay;
  int tilesize;
  int lastgridsize;
  Actions acts;
  class DragOut *dragout;
};

#endif
