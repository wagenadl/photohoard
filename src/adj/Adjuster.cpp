// Adjuster.cpp

#include "Adjuster.h"
#include <math.h>
#include "AdjusterXYZ.h"
#include "AdjusterIPT.h"
#include "AdjusterGeometry.h"

Adjuster::Adjuster(QObject *parent): QObject(parent) {
  caching = true;
  keeporiginal = true;
}

Adjuster::~Adjuster() {
}

void Adjuster::clear() {
  stages.clear();
}

bool Adjuster::isEmpty() const {
  return stages.isEmpty();
}

void Adjuster::setOriginal(Image16 const &image) {
  clear();
  stages << AdjusterTile(image);
}

void Adjuster::setReduced(Image16 const &image, PSize originalSize) {
  clear();
  stages << AdjusterTile(image, originalSize);
}

double Adjuster::maxAvailableScale() const {
  if (stages.isEmpty())
    return 0;
  else if (stages[0].osize.width()==0)
    return 0;
  else
    return stages[0].image.width() / stages[0].osize.width();
}

PSize Adjuster::maxAvailableSize() const {
  return stages.isEmpty() ? PSize(0,0) : stages[0].image.size();
}

Image16 Adjuster::retrieveFull(Sliders const &settings) {
  resetCanceled();
  if (stages.isEmpty())
    return Image16();
  if (stages[0].stage != Stage_Original)
    return Image16();
  qDebug() << "retrieveFull";
  if (stages.last().settings == settings)
    return stages.last().image;

  /* Order of stages here must match enum Stage */
  if (!applyFirstXYZ(settings))
    return Image16();
  if (!applyGeometry(settings))
      return Image16();
  if (!applyIPT(settings))
    return Image16();

  return stages.last().image;  
}

Image16 Adjuster::retrieveReduced(Sliders const &settings,
                                  PSize maxSize) {
  resetCanceled();
  if (stages.isEmpty())
    return Image16();
  int k = 0;
  while (k+1<stages.size()) {
    if (stages[k+1].stage>Stage_Reduced)
      break;
    if (stages[k+1].image.width()<maxSize.width()
        && stages[k+1].image.height()<maxSize.height())
      break;
    if (!stages[k+1].roi.isEmpty())
      break;
    k++;
  }
  if (!stages[k].roi.isEmpty())
    return Image16();
  if (stages[k].image.isNull())
    return Image16();

  while (stages.size()>k+1) // drop no-good stages
    stages.removeLast();
  
  // Now we have a stage that has no reduced roi and that has a suitable scale
  double fac = stages[k].image.size().scaleFactorToFitIn(maxSize);
  qDebug() << "Adjuster img: " << stages[k].image.size()
           << " rq:" << maxSize << fac;
  if (fac<0.8) {
    // It's worth scaling
    // Should we drop excessive scale stacks? Probably.
    stages << AdjusterTile(stages[k].image.scaledToFitIn(maxSize));
    k++;
    stages[k].stage = Stage_Reduced;
  }

  if (stages.last().settings==settings)
    return stages.last().image;

  if (!applyFirstXYZ(settings))
    return Image16();
  if (!applyGeometry(settings))
      return Image16();
  if (!applyIPT(settings))
    return Image16();

  return stages.last().image;  
}

PSize Adjuster::finalSize(Sliders const &settings) const {
  // Is this good enough? Should rotate be allowed to expand the image?
  if (stages.isEmpty())
    return PSize(0, 0);
  PSize s0 = stages[0].osize;
  return s0 - PSize(settings.cropl + settings.cropr,
                    settings.cropt + settings.cropb);
}

Image16 Adjuster::retrieveROI(Sliders const &, QRect) {
  // NYI
  return Image16();
}

Image16 Adjuster::retrieveReducedROI(Sliders const &,
                                     QRect, PSize) {
  // NYI
  return Image16();
}

double Adjuster::estimateScale(Sliders const &, PSize) {
  // NYI
  return 1; 
}

double Adjuster::estimateScaleForROI(Sliders const &,
                                     QRect, PSize) {
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

bool Adjuster::applyGeometry(Sliders const &final) {
  /* Here we apply rotate, and eventually perspective and crop. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  qDebug() << "applyGeometry";
  int iparent = findParentStage(Stage_Geometry);
  if (iparent<0)
    return false;

  AdjusterGeometry adj;
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  qDebug() << "Not already good";
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);

  return true;
}

bool Adjuster::applyFirstXYZ(Sliders const &final) {
  /* Here we apply expose, blackXX, and soon whiteXX. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  qDebug() << "applyXYZ";
  int iparent = findParentStage(Stage_XYZ);
  if (iparent<0)
    return false;

  AdjusterXYZ adj; // we could store this somewhere to enable reuse of LUTs
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  qDebug() << "Not already good";
  if (isCanceled())
    return false;
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
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);
  return true;
}

void Adjuster::cancel() {
  canceled = true;
}

bool Adjuster::isCanceled() {
  if (canceled) {
    canceled = false;
    return true;
  } else {
    return false;
  }
}

void Adjuster::resetCanceled() {
  canceled = false;
}
