
static inline double foo_a(double y, double a) {
  return -(a-1)*y;
}

static inline double foo_b(double y, double a) {
  return 2*(a-1)*y - a;
}

static inline double foo_c(double y, double /*a*/) {
  return y;
}

static inline double foo_d(double y, double a) {
  return pow(foo_b(y,a), 2) - 4*foo_a(y,a)*foo_c(y,a);
}

static inline double foo_f(double y, double a) {
  return a==1 ? y : (-foo_b(y, a) - sqrt(foo_d(y, a)))/(2*foo_a(y, a));
}

static inline double foo_g(double x, double a) {
  return a*x/(1+(a-1)*(1-pow(1-x, 2)));
}

static inline double foo_shadow(double x, double a) {
  return foo_g(foo_f(x, a), 1/a);
}

static inline double foo_highlight(double x, double a) {
  return 1 - foo_shadow(1-x, 1/a);
}

static inline double foo_midtone(double x, double a) {
  return a*x/(1+(a-1)*(1-(1-x)));
}

AdjusterTile AdjusterIPT::apply(AdjusterTile const &parent,
				Sliders const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_IPT;
  tile.image.convertTo(Image16::Format::IPT16);

  double shadows = pow(1.41, final.shadows);
  double highlights = pow(1.41, final.highlights);
  double midtones = pow(2, final.midtones);
  quint16 ilut[65536];
  for (int k=0; k<65536; k++) {
    double x = k/65535.0;
    x = foo_midtone(x, midtones);
    x = foo_shadow(x, shadows);
    x = foo_highlight(x, highlights);
    ilut[k] = (x<0) ? 0 : (x>=1.) ? 65535 : quint16(x*65535.499 + 0.5);
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
  tile.settings.midtones = final.midtones;
  tile.settings.saturation = final.saturation;
  tile.settings.vibrance = final.vibrance;

  return tile;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

AdjusterTile AdjusterIPT::apply(AdjusterTile const &parent,
				Sliders const &final) {
  AdjusterTile tile = parent;
  
  tile.stage = Stage_IPT;
  tile.image.convertTo(Image16::Format::XYZp16);

  double shadows = pow(2, final.shadows);
  double highlights = pow(0.5, final.highlights);
  double midtones = pow(2, final.midtones);
  quint16 ilut[65536];
  for (int k=0; k<65536; k++) {
    double x = k/32768.0;
    x *= (midtones>1)
      ? (midtones/(1+(midtones-1)*pow(x, midtones/(midtones-1))))
      : midtones;
    if (x>1)
      x = 1;
    x *= shadows / (1+(shadows-1)*(1-(1-x)*(1-x)));
    x = 1 - x;
    x *= highlights / (1+(highlights-1)*(1-(1-x)*(1-x)));
    x = 1 - x;
    ilut[k] = (x<0) ? 0 : (x>=1.) ? 32768 : quint16(x*32767.499 + 0.5);
  }
  
  quint16 *words = tile.image.words();
  int X = tile.image.width();
  int Y = tile.image.height();
  int DL = tile.image.wordsPerLine() - 3*X;
  for (int y=0; y<Y; y++) {
    for (int x=0; x<X; x++) {
      int x_ = words[0];
      int y_ = words[1];
      int z_ = words[2];
      int k_ = y_;
      if (x_>k_)
        k_ = x_;
      if (z_>k_)
        k_ = z_;
      if (k_==0)
        k_ = 1;
      *words++ = x_*ilut[k_]/k_;
      *words++ = y_*ilut[k_]/k_;
      *words++ = z_*ilut[k_]/k_;
    }
    words += DL;
  }

  // Apply chroma curve here
  
  tile.settings.shadows = final.shadows;
  tile.settings.highlights = final.highlights;
  tile.settings.midtones = final.midtones;
  tile.settings.saturation = final.saturation;
  tile.settings.vibrance = final.vibrance;

  return tile;
}  
