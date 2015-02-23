// Adjuster.cpp

#include "Adjuster.h"
#include <math.h>

Adjuster::AdjustedTile::AdjustedTile() {
  scale = 1;
}

Adjuster::AdjustedTile::AdjustedTile(Image16 const &img) {
  image = img;
  roi = QRect(QPoint(0, 0), img.size());
  scale = 1;
}

Adjuster::AdjustedTile::AdjustedTile(Image16 const &img, QSize osize) {
  image = img;
  double xsc = img.width() / double(osize.width());
  double ysc = img.height() / double(osize.height());
  roi = QRect(QPoint(0, 0), osize);
  scale = sqrt(xsc*ysc);
}

//////////////////////////////////////////////////////////////////////

Adjuster::Adjuster(QObject *parent): QObject(parent) {
  caching = true;
  keeporiginal = true;
}

Adjuster::~Adjuster() {
}

void Adjuster::setOriginal(Image16 const &image) {
  stages.clear();
  stages << AdjustedTile(image);
}

void Adjuster::setReduced(Image16 const &image, QSize originalSize) {
  stages.clear();
  stages << AdjustedTile(image, originalSize);
}

double Adjuster::maxAvailableScale() const {
  return stages.isEmpty() ? 1 : stages[0].scale;
}

QSize Adjuster::maxAvailableSize() const {
  return stages.isEmpty() ? QSize(0,0) : stages[0].image.size();
}

Image16 Adjuster::retrieveFull(Sliders const &settings) {
  if (stages.isEmpty())
    return Image16();
  if (stages[0].scale != 1)
    return Image16();
  qDebug() << "retrieveFull";
  if (stages.last().settings == settings)
    return stages.last().image;

  qDebug() << "RF: working";
  dropIncompatibleStages(settings);
  if (stages.isEmpty())
    return Image16();
  applySinglePixelSettings(settings);
  if (stages.isEmpty())
    return Image16();
  qDebug() << "RF: done";
  return stages.last().image;  
}

Image16 Adjuster::retrieveReduced(Sliders const &settings,
                                  QSize maxSize) {
  // To be properly implemented later
  return retrieveFull(settings).scaled(maxSize);
}

QSize Adjuster::finalSize(Sliders const &settings) const {
  // Is this good enough? Should rotate be allowed to expand the image?
  if (stages.isEmpty())
    return QSize(0, 0);
  QSize s0 = stages[0].image.size();
  return s0 - QSize(settings.cropl + settings.cropr,
                    settings.cropt + settings.cropb);
}

Image16 Adjuster::retrieveROI(Sliders const &, QRect) {
  // NYI
  return Image16();
}

Image16 Adjuster::retrieveReducedROI(Sliders const &,
                                     QRect, QSize) {
  // NYI
  return Image16();
}

double Adjuster::estimateScale(Sliders const &, QSize) {
  // NYI
  return 1; 
}

double Adjuster::estimateScaleForROI(Sliders const &,
                                     QRect, QSize) {
  // NYI
  return 1;
}

void Adjuster::enableCaching(bool ec) {
  caching = ec;
}

void Adjuster::disableCaching() {
  caching = false;
}

void Adjuster::preserveOriginal(bool po) {
  keeporiginal = po;
}

void Adjuster::applyBlackPointAndExpose(Sliders const &final) {
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  Sliders const &current = stages.last().settings;
  /* I know already that the "current" black point is either the default
     or the final, and same for exposure, but I have *not* made sure that
     one is default if the other is or that one is final if the other is.
  */
  if (current.expose==final.expose
      && current.black==final.black
      && current.blackyb==final.blackyb
      && current.blackrg==final.blackrg)
    return; // easy

  stages << stages.last();
  AdjustedTile &tile(stages.last());
  tile.image.convertTo(Image16::Format::XYZp16);
  // expose is stored in stops
  double slope = pow(2, final.expose - current.expose);
  // black level is stored in percent of white level
  double black_fy = final.black/100.0;
  // black level corrections are stored in percent of main level
  double black_fz = black_fy * (1+(final.blackyb/100.0));
  double black_fx = black_fy * (1+(final.blackrg/100.0));
  double black_oy = current.black/100.0;
  double black_oz = black_oy * (1+(current.blackyb/100.0));
  double black_ox = black_oy * (1+(current.blackrg/100.0));
  double black_dx = black_fx - black_ox;
  double black_dy = black_fy - black_oy;
  double black_dz = black_fz - black_oz;
  qDebug() << "building lut" << slope << black_dx << black_dy << black_dz;  
  quint16 xlut[65536], ylut[65536], zlut[65536];
  for (int k=0; k<65536; k++) {
    double x = k/32768.0;
    x = (x - black_dx)*slope;
    xlut[k] = (x<0) ? 0 : (x>=65535/32768.) ? 65535 : quint16(x*32768.0 + 0.5);
    double y = k/32768.0;
    y = (y - black_dy)*slope;
    ylut[k] = (y<0) ? 0 : (y>=65535/32768.) ? 65535 : quint16(y*32768.0 + 0.5);
    double z = k/32768.0;
    z = (z - black_dz)*slope;
    zlut[k] = (z<0) ? 0 : (z>=65535/32768.) ? 65535 : quint16(z*32768.0 + 0.5);
  }
  quint16 *words = tile.image.words();
  int X = tile.image.width();
  int Y = tile.image.height();
  int DL = tile.image.wordsPerLine() - 3*X;
  for (int y=0; y<Y; y++) {
    for (int x=0; x<X; x++) {
      *words = xlut[*words]; words++;
      *words = ylut[*words]; words++;
      *words = zlut[*words]; words++;
    }
    words += DL;
  }
  qDebug() << X << Y << DL;
  tile.settings.expose = final.expose;
  tile.settings.black = final.black;
  tile.settings.blackyb = final.blackyb;
  tile.settings.blackrg = final.blackrg;
}

void Adjuster::applySinglePixelSettings(Sliders const &settings) {
  /* Applies all those settings that operate on individual pixels.
     These are relatively easy, becase they work regardless of scale or ROI.
     At this point, we must already know that the topmost stage is
     compatible with our goals.
   */
  applyBlackPointAndExpose(settings);
}

static bool canBeSinglePixelBase(Sliders const &src, Sliders const &dst) {
  /* This checks some special problems having to do with the fact that
     certain settings are applied in groups.
   * Presumption: src can be ancestor of dst per Sliders def.
   * src cannot be a base for the single pixel transformations toward dst if
     - src has expose or hlrescue different from dst AND the other is different
       from default.
     - src has black, blackyb, blackrg different from dst AND one of the others
       is different from default.
     - etc?
   * This can be stated more succinctly as:
     - (expose,hlresuce) has to be applied to src in order to get to dst and
       (expose,hlresuce) is not (default,default).
     - etc.
   * Or positively:
     - (expose,hlrescue) must either both be the same in src and dst or
       (expose,hlrescue) must both be default in src
   */
  return ((src.expose==dst.expose
           && src.hlrescue==dst.hlrescue)
          || (src.expose==src.exposeDefault
              && src.hlrescue==src.hlrescueDefault))
    && ((src.black==dst.black
         && src.blackyb==dst.blackyb
         && src.blackrg==dst.blackrg)
        || (src.black==src.blackDefault
            && src.blackyb==src.blackybDefault
            && src.blackrg==src.blackrgDefault))
    ;
          
}
  

void Adjuster::dropIncompatibleStages(Sliders const &s) {
  while (!stages.isEmpty()) {
    Sliders const &m = stages.last().settings;
    bool ok = m.couldBeAncestorOf(s);
    if (ok && canBeSinglePixelBase(m, s))
      break;
    stages.removeLast();
  }
}
