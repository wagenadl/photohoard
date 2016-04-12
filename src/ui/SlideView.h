// SlideView.h

#ifndef SLIDEVIEW_H

#define SLIDEVIEW_H

#include <QLabel>
#include "Image16.h"
#include "ColorLabelBar.h"
#include "Action.h"
#include <QPointer>

class SlideView: public QFrame {
  Q_OBJECT;
public:
  SlideView(QWidget *parent=0);
  virtual ~SlideView();
  PSize desiredSize() const;
  /* DESIREDSIZE - Desired dimensions of current scaled image
     If the zoom mode is set to SCALE-TO-FIT, returns the size of our widget.
     Otherwise, returns the size that the current image should be scaled to
     for filling the widget.
   */
  double currentZoom() const;
  /* CURRENTZOOM - Actual zoom factor
     Returns the actual current zoom factor.
   */
  double fittingZoom() const;
  /* FITTINGZOOM - Zoom factor that fills widget
     Returns the zoom factor that would make the current image snugly fit.
   */
  quint64 currentVersion() const;
  /* CURRENTVERSION - Return version ID of image being displayed, or 0 */
  Actions const &actions() const;
  /* ACTIONS - List of keyboard actions
   */
  PSize currentImageSize() const;
  /* CURRENTIMAGESIZE - Natural size of current image
   */
  QTransform transformationToImage() const;
  /* TRANSFORMATIONTOIMAGE - Matrix to map widget coords to image coords
     t = TRANSFORMATIONFROMIMAGE() returns a transformation matrix that can
     be used to map widget coordinates to image coordinates.
  */
  QTransform transformationFromImage() const;
  /* TRANSFORMATIONFROMIMAGE - Matrix to map image coords to widget coords
     t = TRANSFORMATIONFROMIMAGE() returns a transformation matrix that can
     be used to map image coordinates to widget coordinates.
  */
  void addOverlay(class SlideOverlay *over);
  /* ADDOVERLAY - Experimental interface */
  void removeOverlay(class SlideOverlay *over);
  /* REMOVEOVERLAY - Experimental interface */
public slots:
  void newImage(quint64 vsn, QSize natSize);
  /* NEWIMAGE - Prepare for new version
     NEWIMAGE(vsn, natsize) prepares the slide view for displaying a new
     version (identified by id VSN) that has the given natural size.
   */
  void clear();
  /* CLEAR - Drop image
     CLEAR clears the display and forgets about the previously displayed version
   */
  void updateImage(quint64 vsn, Image16 const &img, bool force=false);
  /* UPDATEIMAGE - Offer a new image for the currently displayed version.
     UPDATEIMAGE(vsn, img, force) offers the image IMG for display as version
     VSN. If VSN doesn't match our current version (see NEWIMAGE), this
     call is ignored.
     If IMG is smaller than the image we currently have in memory, the
     update is only performed if FORCE is true. (This facility is used when
     sliders are changed, and not, e.g., when the slide strip requests a
     thumbnail.)
   */
  void setZoom(double zm);
  /* SETZOOM - Set zoom level to absolute ratio
     SETZOOM(zm) sets the zoom level to the ratio ZM.
     If ZM is less than the fitting zoom, we shift to scale-to-fit.
   */
  void changeZoomLevel(QPoint center, double delta, bool roundtodelta=false);
  /* CHANGEZOOMLEVEL - Change zoom level by a fraction
     CHANGEZOOMLEVEL(center, delta) changes the zoom level by a factor 2^DELTA
     keeping the point CENTER where it is.
     CHANGEZOOMLEVEL(center, delta, roundtodelta) specifies that the final
     zoom level must be an integer power of DELTA if ROUNDTODELTA is true.
     If the resulting zoom is less than the fitting zoom, we shift to
     scale-to-fit.
   */
  void scaleToFit();
  /* SCALETOFIT - Zoom such that the image fits snugly
   */
signals:
  void needImage(quint64 vsn, QSize desired);
  /* NEEDIMAGE - Emitted if a larger image is required
     NEEDIMAGE is emitted when a larger image would be needed to
     fill the widget at the current zoom level. This signal is ordinarily
     connected (ultimately) to the AutoCache and also to the LiveAdjuster.
   */
  void doubleClicked();
  void newSize(QSize);
  /* NEWSIZE - Emitted when DESIREDSIZE changes.
   */
  void newZoom(double);
  /* NEWZOOM - Emitted when actual zoom level changes */
protected:
  virtual void keyPressEvent(QKeyEvent *) override;
  virtual void mousePressEvent(QMouseEvent *) override;
  virtual void mouseMoveEvent(QMouseEvent *) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *) override;
  virtual void resizeEvent(QResizeEvent *) override;
  virtual void paintEvent(QPaintEvent *) override;
  virtual void wheelEvent(QWheelEvent *) override;
  virtual void enterEvent(QEvent *) override;
private:
  //  QPointF mapWidgetToImage(QPointF) const;
  //  QPointF mapImageToWidget(QPointF) const;
  void updateRelXY(QPoint);
  void makeActions();
  QList<class SlideOverlay *> overlays() const;
  void needLargerImage();
private:
  quint64 vsnid;
  PSize naturalSize;
  Image16 img;
  double zoom;
  bool fit;
  double relx; // 0 for extreme left/top visible, 1 for extreme right/bot
  double rely; // ... not defined when fit
  PSize lastSize;
  QPoint presspoint;
  double pressrelx, pressrely;
  bool dragging;
  Actions acts;
  mutable QList< QPointer<QObject> > overlays_;
};

#endif
