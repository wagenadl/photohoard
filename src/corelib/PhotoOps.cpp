// PhotoOps.cpp

#include "PhotoOps.h"
#include "PDebug.h"
#include <opencv2/photo.hpp>

namespace PhotoOps {
  Image16 seamlessClone(Image16 const &target,
                        Image16 const &source, QImage const &mask,
                        QPoint p, int method) {
    pDebug() << "seamlessclone" << p << target.size();
    QImage in = target.toQImage();
    in.convertTo(QImage::Format_RGB888);
    cv::Mat const tgt(in.height(), in.width(), CV_8UC3,
                      (void*)in.bits(), in.bytesPerLine());
    QImage ins = source.toQImage();
    ins.convertTo(QImage::Format_RGB888);
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
                  double radius, int method) {
    QImage in = target.toQImage();
    in.convertTo(QImage::Format_RGB888);
    cv::Mat const tgt(in.height(), in.width(), CV_8UC3,
                      (void*)in.bits(), in.bytesPerLine());
    QImage msk1(mask.convertToFormat(QImage::Format_Grayscale8));
    cv::Mat const msk(msk1.height(), msk1.width(),
                      CV_8UC1,
                      (void*)(msk1.bits()), msk1.bytesPerLine());
    QImage res(in.size(), QImage::Format_RGB888);
    cv::Mat out(res.height(), res.width(), CV_8UC3,
                res.bits(), res.bytesPerLine());
    qDebug() << "inpaint" << CV_8UC1 << CV_8UC3 << tgt.type() << msk.type() << out.type();
    cv::inpaint(tgt, msk, out, radius,
                method ? cv::INPAINT_TELEA : cv::INPAINT_NS);
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
    pDebug() << "  blur" << X << Y << B << alpha;
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
    pDebug() << "blurred";
  }
};
