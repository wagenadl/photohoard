// AllAdjuster.h

#ifndef ALLADJUSTER_H

#define ALLADJUSTER_H

#include "Adjuster.h"
#include "AllAdjustments.h"

class AllAdjuster: public Adjuster {
  /* Class: AllAdjuster

     AllAdjuster is a derivative of Adjuster that can handle
     multiple layers.
  */
  Q_OBJECT;
public:
  AllAdjuster(QObject *parent=0);
  virtual ~AllAdjuster();
  void setMaxThreads(int) override;
  // Please see Adjuster for methods not documented here
  void clear() override;
  bool isEmpty() const override;
  void setOriginal(Image16 const &image) override;
  void setReduced(Image16 const &image, PSize originalSize) override;
  Image16 retrieveFull(Adjustments const &settings) = delete;
  Image16 retrieveFull(AllAdjustments const &settings);
  Image16 retrieveReduced(Adjustments const &settings,
			  PSize maxSize) = delete;
  Image16 retrieveReduced(AllAdjustments const &settings,
			  PSize maxSize);
  Image16 retrieveROI(Adjustments const &settings, QRect roi) = delete;
  Image16 retrieveROI(AllAdjustments const &settings, QRect roi);
  Image16 retrieveReducedROI(Adjustments const &settings,
                             QRect roi, PSize maxSize) = delete;
  Image16 retrieveReducedROI(AllAdjustments const &settings,
                             QRect roi, PSize maxSize);
  void enableCaching(bool ec=true) override;
  void disableCaching() override;
  void preserveOriginal(bool po=true) override;
  void cancel() override;
protected:
  void ensureAdjusters(int nLayers); // nLayers is number of adjustment
  // layers not counting the base.
protected:
  QList<Adjuster *> layerAdjusters; // index k corresponds to the layer
  // that would be called n = k+1 in Layers and AllAdjustments.
  QList<bool> validInput; // does adjuster k have valid input?
  AllAdjustments lastrq;
};

#endif
