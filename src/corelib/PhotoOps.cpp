// PhotoOps.cpp

#include "PhotoOps.h"
#include "PDebug.h"
#include <opencv2/photo.hpp>
#include <cmath>
#include "ColorSpaces.h"
#include <QPainter>

namespace PhotoOps {
  Image16 seamlessClone(Image16 const &target,
                        Image16 const &source, QImage const &mask,
                        QPoint p, int method) {
    //    pDebug() << "seamlessclone" << p << target.size();
    QImage in = target.toQImage();
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    in = in.convertToFormat(QImage::Format_RGB888);
#else
    in.convertTo(QImage::Format_RGB888);
#endif
    cv::Mat const tgt(in.height(), in.width(), CV_8UC3,
                      (void*)in.bits(), in.bytesPerLine());
    QImage ins = source.toQImage();
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    ins = in.convertToFormat(QImage::Format_RGB888);
#else
    ins.convertTo(QImage::Format_RGB888);
#endif
    cv::Mat const src(ins.height(), ins.width(), CV_8UC3,
                      (void*)ins.bits(), ins.bytesPerLine());
    QImage msk1(mask.convertToFormat(QImage::Format_Grayscale8));
    cv::Mat const msk(msk1.height(), msk1.width(),
                      CV_8UC1,
                      (void*)(msk1.bits()), msk1.bytesPerLine());
    QImage res(in.size(), QImage::Format_RGB888);
    cv::Mat out(res.height(), res.width(), CV_8UC3,
                res.bits(), res.bytesPerLine());
    cv::Point pt(p.x(), p.y());
    cv::seamlessClone(src, tgt, msk, pt, out, method);
    return Image16(res).convertedTo(target.format());
  }

  Image16 inpaint(Image16 const &target,
                  QImage const &mask,
                  double radius) {
    QImage in = target.toQImage();
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    in = in.convertToFormat(QImage::Format_RGB888);
#else
    in.convertTo(QImage::Format_RGB888);
#endif
    cv::Mat const tgt(in.height(), in.width(), CV_8UC3,
                      (void*)in.bits(), in.bytesPerLine());
    QImage msk1(mask.convertToFormat(QImage::Format_Grayscale8));
    cv::Mat const msk(msk1.height(), msk1.width(),
                      CV_8UC1,
                      (void*)(msk1.bits()), msk1.bytesPerLine());
    QImage res(in.size(), QImage::Format_RGB888);
    cv::Mat out(res.height(), res.width(), CV_8UC3,
                res.bits(), res.bytesPerLine());
    //    qDebug() << "inpaint" << CV_8UC1 << CV_8UC3 << tgt.type() << msk.type() << out.type();
    cv::inpaint(tgt, msk, out, radius, cv::INPAINT_TELEA);
    return Image16(res).convertedTo(target.format());
  }

  Image16 decolorizeOrBoost(Image16 const &target, bool boost) {
    QImage tgt1(target.toQImage());
    cv::Mat const tgt(tgt1.height(), tgt1.width(),
                      CV_8UC4,
                      (void*)target.bytes(), tgt1.bytesPerLine());
    QImage res(tgt1.size(), QImage::Format_Grayscale8);
    cv::Mat out(res.height(), res.width(),
                CV_8UC1,
                res.bits(), res.bytesPerLine());
    QImage resrgb(tgt1.size(), QImage::Format_ARGB32);
    cv::Mat outrgb(res.height(), res.width(),
                   CV_8UC4,
                   resrgb.bits(), resrgb.bytesPerLine());
    cv::decolor(tgt, out, outrgb);
    if (boost)
      return Image16(resrgb);
    else
      return Image16(res);
  }

  void blur(QImage &target, double sigma) {
    pDebug() << "blur - could be faster with multithreading";
    ASSERT(target.format() == QImage::Format_Grayscale8);
    uchar *bits = target.bits();
    int X = target.width();
    int Y = target.height();
    int B = target.bytesPerLine();
    double alpha = 1/sigma;
    //    pDebug() << "  blur" << X << Y << B << alpha;
    for (int y=0; y<Y; y++) {
      uchar *row = bits + B*y;
      double b = *row;
      for (int x=0; x<X; x++) {
        double a = *row;
        b += alpha*(a - b);
        *row++ = b + .5;
      }
      for (int x=X-1; x>=0; x--) {
        double a = *--row;
        b += alpha*(a - b);
        *row = b + .5;
      }
    }
    for (int x=0; x<X; x++) {
      uchar *col = bits + x;
      double b = col[0];
      for (int y=0; y<Y; y++) {
        double a = *col;
        b += alpha*(a-b);
        *col = b + .5;
        col += B;
      }
      for (int y=Y-1; y>=0; y--) {
        col -= B;
        double a = *col;
        b += alpha*(a-b);
        *col = b + .5;
      }
    }
      
//    cv::Mat tgt(target.height(), target.width(),
//                isGray?CV_8UC1:CV_8UC4,
//                (void*)bits, target.bytesPerLine());
//    cv::GaussianBlur(tgt, tgt, cv::Size(0,0), sigma, sigma);
//    pDebug() << "blurred";
  }


  void ringAverage(Image16 const &src, QPointF center, double radius,
                   float out[3]) {
    quint16 const *words = src.words();
    float xc = center.x();
    float yc = center.y();
    int L = src.wordsPerLine();
    int W = src.width();
    int H = src.height();
    int x0 = xc - radius;
    if (x0<0)
      x0 = 0;
    int x1 = xc + radius + .9999;
    if (x1 > W)
      x1 = W;
    int y0 = yc - radius;
    if (y0 < 0)
      y0 = 0;
    int y1 = yc + radius + .9999;
    if (y1 > H)
      y1 = H;
    int N = 0;
    float r2 = radius*radius;
    float r20 = std::floor(.7*r2); // does this guarantee we get something?
    float accum[3]{0, 0, 0};
    for (int y=y0; y<y1; y++) {
      float dy = y-yc;
      float dy2 = dy*dy;
      ColorSpaces::XYZ const *line = (ColorSpaces::XYZ const *)(words + L*y);
      for (int x=x0; x<x1; x++) {
        float dx = x-xc;
        float dx2 = dx*dx;
        float dr2 = dx2 + dy2;
        if (dr2>=r20 && dr2<r2) {
          ColorSpaces::XYZ const &pix = line[x];
          accum[0] += pix.X;
          accum[1] += pix.Y;
          accum[2] += pix.Z;
          N += 1;
        }
      }
    }
    if (N==0) {
      qDebug() << "clone N=0!";
      N = 1;
    }
    out[0] = accum[0] / N;
    out[1] = accum[1] / N;
    out[2] = accum[2] / N;
  }

  void clone(Image16 &target, QPointF dst, QPointF src, double radius) {
    int W = target.width();
    int H = target.height();
    int xc = dst.x() + .5;
    int yc = dst.y() + .5;
    int x0 = xc - radius;
    if (x0<0)
      x0 = 0;
    int x1 = xc + radius + .9999;
    if (x1 > W)
      x1 = W;
    int y0 = yc - radius;
    if (y0 < 0)
      y0 = 0;
    int y1 = yc + radius + .9999;
    if (y1 > H)
      y1 = H;
    int sxc = src.x() + .5;
    int syc = src.y() + .5;
    int sx0 = sxc + (x0 - xc);
    if (sx0<0) {
      x0 += 0 - sx0;
      x0 = 0;
    }
    int sx1 = sxc + (x1 - xc);
    if (sx1 > W) {
      x1 -= sx1 - W;
      x1 = W;
    }
    int sy0 = syc + (y0 - yc);
    if (sy0<0) {
      y0 += 0 - sy0;
      y0 = 0;
    }
    int sy1 = syc + (y1 - yc);
    if (sy1 > H) {
      y1 -= sy1 - H;
      y1 = W;
    }
    if (y1<=y0 || x1<=x0)
      return;
    QRect srcrect(QPoint(x0, y0), QPoint(x1, y1));
    QRect dstrect(QPoint(sx0, sy0), QPoint(sx1, sy1));
    qDebug() << "Clone broken - subimage does not work";
    /*
    Image16 srcbit(target.subImage(srcrect));
    Image16 dstbit(target.subImage(dstrect));
    QImage mask(srcrect.size(), QImage::Format_Grayscale8);
    mask.fill(0);
    { QPainter p(&mask);
      p.setBrush(QColor(255, 255, 255));
      p.drawEllipse(QPoint(xc - x0, yc - y0), int(.8*radius), int(.8*radius));
    }
    PhotoOps::blur(mask, .15*radius);
    srcbit.alphablend(dstbit, mask); // is this right??
    */
  }

  
  void blendclone(Image16 &target, QPointF dst, QPointF src, double radius) {
    target.convertTo(Image16::Format::XYZp16);
    int L = target.wordsPerLine();
    int W = target.width();
    int H = target.height();
    float r2 = radius*radius;
    double xc = dst.x();
    double yc = dst.y();
    int x0 = xc - radius;
    if (x0<0)
      x0 = 0;
    int x1 = xc + radius + .9999;
    if (x1 > W)
      x1 = W;
    int y0 = yc - radius;
    if (y0 < 0)
      y0 = 0;
    int y1 = yc + radius + .9999;
    if (y1 > H)
      y1 = H;
    int sdx = src.x() - dst.x();
    int sdy = src.y() - dst.y();
    float ipt_srcedge[3]; ringAverage(target, src, radius, ipt_srcedge);
    float ipt_dstedge[3]; ringAverage(target, dst, radius, ipt_dstedge);
    float scl[3];
    for (int k=0; k<3; k++)
      scl[k] = ipt_dstedge[k] / ipt_srcedge[k];

    quint16 *words = target.words();
    constexpr int COSTBLSIZE = 256;
    float costbl[COSTBLSIZE];
    for (int i=0; i<COSTBLSIZE; i++)
      costbl[i] = std::cos(std::sqrt(i/float(COSTBLSIZE))*3.1415)*.5 + .5;
    
    for (int y=y0; y<y1; y++) {
      int sy = y + sdy;
      if (sy<0 || sy>=H)
        continue; // ugly way to deal with edge
      ColorSpaces::XYZ *line = (ColorSpaces::XYZ *)(words + L*y);
      ColorSpaces::XYZ const *sline = (ColorSpaces::XYZ const *)(words + L*sy);
      float dy = y-yc;
      float dy2 = dy*dy;
      for (int x=x0; x<x1; x++) {
        int sx = x + sdx;
        if (sx<0 || sx>=W)
          continue; // ugly way to deal with edge

        ColorSpaces::XYZ &pix = line[x];
        ColorSpaces::XYZ const &spix = sline[sx];
        float dx = x-xc;
        float dx2 = dx*dx;
        float dr2 = dx2 + dy2;
        float drx = dr2/r2;
        if (drx>=1)
          continue;
        float alpha = drx <= .5 ? 1 : costbl[int(COSTBLSIZE*(drx-.5)*2)];
        int X = (1-alpha)*pix.X + alpha*scl[0]*spix.X;
        pix.X = (X<0) ? 0 : (X>32767) ? 32767 : X;
        int Y = (1-alpha)*pix.Y + alpha*scl[1]*spix.Y;
        pix.Y = (Y<0) ? 0 : (Y>32767) ? 32767 : Y;
        int Z = (1-alpha)*pix.Z + alpha*scl[2]*spix.Z;
        pix.Z = (Z<0) ? 0 : (Z>32767) ? 32767 : Z;
      }
    }
  }
};

