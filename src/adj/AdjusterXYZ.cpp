// AdjusterXYZ.cpp

#include "AdjusterXYZ.h"

QStringList AdjusterXYZ::fields() const {
  static QStringList flds
    = QString("expose black blackyb blackrg wb wbg").split(" ");
  return flds;
}

AdjusterTile AdjusterXYZ::apply(AdjusterTile const &parent,
				Sliders const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_XYZ;
  tile.image.convertTo(Image16::Format::LMS16);
  
  // expose is stored in stops
  double slope = pow(2, final.expose);
  double slopeX = slope*pow(2, final.wbg/15);
  double slopeZ = slope*pow(2, final.wb/5);
  // black level is stored in percent of white level
  double black_dy = final.black/100.0;
  // black level corrections are stored in percent of main level
  double black_dz = black_dy * (1+(final.blackyb/100.0));
  double black_dx = black_dy * (1+(final.blackrg/100.0));
  quint16 xlut[65536], ylut[65536], zlut[65536];
  for (int k=0; k<65536; k++) {
    double v = k/32768.0;
    double x = (v - black_dx)*slopeX;
    xlut[k] = (x<0) ? 0 : (x>=65535/32768.) ? 65535 : quint16(x*32768.0 + 0.5);
    double y = (v - black_dy)*slope;
    ylut[k] = (y<0) ? 0 : (y>=65535/32768.) ? 65535 : quint16(y*32768.0 + 0.5);
    double z = (v - black_dz)*slopeZ;
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

  tile.settings.expose = final.expose;
  tile.settings.black = final.black;
  tile.settings.blackyb = final.blackyb;
  tile.settings.blackrg = final.blackrg;
  tile.settings.wb = final.wb;
  tile.settings.wbg = final.wbg;

  return tile;
}
