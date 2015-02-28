// AdjusterStage.cpp

#include "AdjusterStage.h"

bool AdjusterStage::isDefault(Sliders const &a, QStringList fields) {
  for (auto s: fields) 
    if (!a.isDefault(s))
      return false;
  return true;
}

bool AdjusterStage::isEquivalent(Sliders const &a, Sliders const &b,
				 QStringList fields) {
  for (auto s: fields) 
    if (a.get(s) != b.get(s))
      return false;
  return true;
}

