// ScreenResolution.cpp

#include "ScreenResolution.h"

#include <QX11Info>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <stdlib.h>
#include <math.h>
#include <QMutexLocker>

QSize ScreenResolution::pixelCount() {
  ensure();
  return pc();
}

QSizeF ScreenResolution::millimeterSize() {
  ensure();
  return ms();
}

double ScreenResolution::dpi() {
  ensure();
  return dpi_();
}

QFont ScreenResolution::defaultFont() {
  static QFont f("Lato");
  static bool first = true;
  if (first) 
    f.setPixelSize(0.1 * dpi() * pow(millimeterSize().width()/250, 0.3));
  return f;
}

//////////////////////////////////////////////////////////////////////
// Internal functions
QSize &ScreenResolution::pc() {
  static QSize p;
  return p;
}

QSizeF &ScreenResolution::ms() {
  static QSizeF m;
  return m;
}

double &ScreenResolution::dpi_() {
  static double d;
  return d;
}

bool &ScreenResolution::ready() {
  static bool ok = false;
  return ok;
}

void ScreenResolution::ensure() {
  // This mutex guarantees that once we are calculating, we won't start again.
  static QMutex mutex;
  QMutexLocker l(&mutex);
  
  if (ready())
    return;

  Display *d = QX11Info::display();
  char const *name = DisplayString(d);
  xcb_connection_t *xcbcon = xcb_connect(name, 0);

  //Get the first X screen
  xcb_screen_t* firstscreen
    = xcb_setup_roots_iterator(xcb_get_setup(xcbcon)).data;
  //Generate ID for the X window
  xcb_window_t win = xcb_generate_id(xcbcon);
  
  //Create dummy X window
  xcb_create_window(xcbcon, 0, win, firstscreen->root,
		    0, 0, 1, 1, 0, 0, 0, 0, 0);
  
  //Flush pending requests to the X server
  xcb_flush(xcbcon);
  
  //Send a request for screen resources to the X server
  xcb_randr_get_screen_resources_cookie_t screenResCookie
    = xcb_randr_get_screen_resources(xcbcon, win);
  
  //Receive reply from X server
  xcb_randr_get_screen_resources_reply_t* screenResReply 
    = xcb_randr_get_screen_resources_reply(xcbcon, screenResCookie, 0);
  
  if (!screenResReply) {
    fallback();
    return;
  }
  
  xcb_randr_get_output_primary_cookie_t primc
    = xcb_randr_get_output_primary(xcbcon, win);
  xcb_randr_get_output_primary_reply_t* prim
    = xcb_randr_get_output_primary_reply(xcbcon, primc, 0);
  
  xcb_randr_get_output_info_cookie_t outputResCookie
    = xcb_randr_get_output_info(xcbcon, prim->output, 0);    
  xcb_randr_get_output_info_reply_t* outputResReply
    = xcb_randr_get_output_info_reply(xcbcon,
				      outputResCookie, 0);

  ms() = QSizeF(outputResReply->mm_width, outputResReply->mm_height);
  
  xcb_randr_get_crtc_info_cookie_t crtcResCookie
    = xcb_randr_get_crtc_info(xcbcon, 
			      outputResReply->crtc, 0);
  xcb_randr_get_crtc_info_reply_t* crtcResReply
    = xcb_randr_get_crtc_info_reply(xcbcon,
				    crtcResCookie, 0);

  pc() = QSize(crtcResReply->width, crtcResReply->height);
  
  free(prim);
  free(crtcResReply);
  free(outputResReply);

  double xdpi = pc().width()/(ms().width()/25.4);
  double ydpi = pc().height()/(ms().height()/25.4);
  dpi_() = sqrt(xdpi*ydpi);
  ready() = true;
}

void ScreenResolution::fallback() {
  QDesktopWidget *dt = QApplication::desktop();
  pc() = dt->screenGeometry().size();
  dpi_() = 96;
  ms() = QSizeF(pc()*25.4/dpi_());
  ready() = true;
}
