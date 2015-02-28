// Adjuster.cpp

#include "Adjuster.h"
#include <math.h>
#include "AdjusterXYZ.h"
#include "AdjusterIPT.h"

Adjuster::Adjuster(QObject *parent): QObject(parent) {
  caching = true;
  keeporiginal = true;
}

Adjuster::~Adjuster() {
}

void Adjuster::setOriginal(Image16 const &image) {
  stages.clear();
  stages << AdjusterTile(image);
}

void Adjuster::setReduced(Image16 const &image, QSize originalSize) {
  stages.clear();
  stages << AdjusterTile(image, originalSize);
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
  if (stages.isEmpty())
    return Image16();
  if (!applySinglePixelSettings(settings))
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


bool Adjuster::ensureAlreadyGood(AdjusterStage const &adj, int iparent,
				 Sliders const &final) {
  if (stages.size()>iparent+1) {
    Sliders const &current = stages[iparent+1].settings;
    if (adj.isEquivalent(current, final)) 
      return true; // previous version of stage is fine
    else
      // everything that follows will be invalid
      while (stages.size()>iparent+1)
	stages.removeLast();
  }

  if (adj.isDefault(final))
    return true; // we don't need this stage
  else
    return false; // work to be done
}

int Adjuster::findParentStage(Stage s) const {
  for (int k=0; k<stages.size(); k++)
    if (stages[k].stage>=s)
      return k-1;
  return stages.size()-1;
}

bool Adjuster::applyFirstXYZ(Sliders const &final) {
  /* Here we apply expose, blackXX, and soon whiteXX. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  qDebug() << "applyXYZ";
  int iparent = findParentStage(Stage_XYZ);
  qDebug() << iparent << stages.size();
  if (iparent<0)
    return false;

  AdjusterXYZ adj; // we could store this somewhere to enable reuse of LUTs
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  qDebug() << "Not already good";
  qDebug() << iparent << stages.size();
  stages << adj.apply(stages[iparent], final);

  return true;
}


bool Adjuster::applyIPT(Sliders const &final) {
  qDebug() << "applyIPT";
  int iparent = findParentStage(Stage_IPT);
  if (iparent<0)
    return false;
  AdjusterIPT adj;
   if (ensureAlreadyGood(adj, iparent, final))
    return true;
  stages << adj.apply(stages[iparent], final);
  return true;
}
 

bool Adjuster::applySinglePixelSettings(Sliders const &settings) {
  /* Applies all those settings that operate on individual pixels.
     These are relatively easy, becase they work regardless of scale or ROI.
     At this point, we must already know that the topmost stage is
     compatible with our goals.
   */
  return applyFirstXYZ(settings) && applyIPT(settings);
}

