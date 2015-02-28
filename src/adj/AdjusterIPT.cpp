// AdjusterIPT.cpp

#include "AdjusterIPT.h"

QStringList AdjusterIPT::fields() const {
  static QStringList flds
    = QString("expose hlrescue shadows highlights saturation vibrance")
    .split(" ");
  return flds;
}

AdjusterTile AdjusterIPT::apply(AdjusterTile const &parent,
				Sliders const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_IPT;
  tile.image.convertTo(Image16::Format::Lab16);

  double shadows = pow(2, final.shadows);
  // current.shadows had better be 0!
  double highlights = pow(0.5, final.highlights);
  //    double hlrslope = pow(2, final.hlrescue);
  //    double hlrpar = final.hlrescue;
  quint16 ilut[65536];
  //    double y0 = 0;
  //    for (int k=0; k<65536; k++) {
  //      double dydx = tanh(pow(1-y0, hlrpar));
  //      y0 += hlrslope*dydx/65536.0;
  //    }
  //    double y = 0;
  for (int k=0; k<65536; k++) {
    double x = k/65535.0;
    // double dydx = tanh(pow(1-y, hlrpar));
    //y += hlrslope*dydx/65536.0;
    double y = x;
    // if ((k&255) == 0)
    // qDebug() << x << y/y0;
    double z = y/*/y0*/ * shadows / (1+(shadows-1)*(1-(1-y)*(1-y)));
    z = 1 - z;
    z *= highlights / (1+(highlights-1)*(1-(1-z)*(1-z)));
    z = 1 - z;
    ilut[k] = (z<0) ? 0 : (z>=1.) ? 65535 : quint16(z*65535.499 + 0.5);
  }
  
  quint16 *words = tile.image.words();
  int X = tile.image.width();
  int Y = tile.image.height();
  int DL = tile.image.wordsPerLine() - 3*X;
  for (int y=0; y<Y; y++) {
    for (int x=0; x<X; x++) {
      *words = ilut[*words];
      words+=3;
    }
    words += DL;
  }

  // Apply chroma curve here
  
  tile.settings.shadows = final.shadows;
  tile.settings.highlights = final.highlights;
  tile.settings.expose = final.expose;
  tile.settings.hlrescue = final.hlrescue;
  tile.settings.saturation = final.saturation;
  tile.settings.vibrance = final.vibrance;

  return tile;
}  
