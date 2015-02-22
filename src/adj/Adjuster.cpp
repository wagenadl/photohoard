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

Adjuster::Adjuster() {
  caching = true;
  keeporiginal = true;
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

Image16 Adjuster::retrieveFull(Sliders const &settings) const {
  if (stages.isEmpty())
    return Image16();
  if (stages[0].scale != 1)
    return Image16();
  if (stages.last().settings == settings)
    return stages.last().image;

  dropIncompatibleStages(settings);
  applyCurveSettings(settings);
  return stages.last().image;  
}

Image16 Adjuster::retrieveReduced(Sliders const &settings,
                                  QSize maxSize) const {
}

QSize Adjuster::finalSize(Sliders const &settings) const {
}

Image16 Adjuster::retrieveROI(Sliders const &settings, QRect roi) const {
}

Image16 Adjuster::retrieveReducedROI(Sliders const &settings,
                                     QRect roi, QSize maxSize) const {
}

double Adjuster::estimateScale(Sliders const &settings, QSize imageSize) {
}

double Adjuster::estimateScaleForROI(Sliders const &settings,
                                     QRect roi, QSize imageSize) {
}

void Adjuster::enableCaching(bool ec) {
}

void Adjuster::disableCaching() {
}

void Adjuster::preserveOriginal(bool po) {
}

void Adjuster::applyCurveSettings(Sliders const &settings) {
}
