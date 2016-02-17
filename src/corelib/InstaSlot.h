// InstaSlot.h

#ifndef INSTASLOT_H

#define INSTASLOT_H

#include <QObject>
#include <functional>

class InstaSlot: public QObject {
  Q_OBJECT;
public:
  InstaSlot(QObject *source, char const *signal,
            QObject *dest, std::function<void()> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  InstaSlot(QObject *source, char const *signal,
            std::function<void()> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  /* INSTASLOT - Slot without a prototype
     NEW INSTASLOT(source, signal, slot), where SOURCE is a QObject, SIGNAL
     is a signal, and SLOT is a C++11-style lambda expression, creates a
     connection between SIGNAL and SLOT just like any other Qt connection,
     except that SLOT is not associated with any particular object.
     NEW INSTASLOT(source, signal, dest, slot), where DEST is also a QObject,
     does the same but does associate the SLOT with an object, so that the
     connection is broken (and the INSTASLOT deleted) when DEST is deleted,
     not just when SOURCE is deleted.
     INSTASLOT(..., TYPE) specifies a connection type, as for ordinary
     connections.
     Example:
        QPushButton *button = new QPushButton();
        new InstaSlot(button, SIGNAL(clicked()),
                      []() { printf("Hello\n"); });
        new InstaSlot(button, SIGNAL(clicked()),
                      this, [this]() { this->doSometing(); });
   */
public slots:
  void slot();
private:
   std::function<void()> foo;
};

class InstaBool: public QObject {
  Q_OBJECT;
public:
  InstaBool(QObject *source, char const *signal,
            QObject *dest, std::function<void(bool)> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  InstaBool(QObject *source, char const *signal,
            std::function<void(bool)> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  /* INSTABOOL - Instaslot with a BOOL argument
     Example:
        QCheckBox *button = new QCheckBox();
        new InstaBool(button, SIGNAL(toggled(bool)),
                      [](bool b) { printf("Checked: %i\n", b); });
  */     
public slots:
  void slot(bool);
private:
   std::function<void(bool)> foo;
};

class InstaInt: public QObject {
  Q_OBJECT;
public:
  InstaInt(QObject *source, char const *signal,
            QObject *dest, std::function<void(int)> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  InstaInt(QObject *source, char const *signal,
            std::function<void(int)> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  /* INSTAINT - Instaslot with an INT argument
  */     
public slots:
  void slot(int);
private:
   std::function<void(int)> foo;
};
    
class InstaString: public QObject {
  Q_OBJECT;
public:
  InstaString(QObject *source, char const *signal,
            QObject *dest, std::function<void(QString)> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  InstaString(QObject *source, char const *signal,
            std::function<void(QString)> slot,
            Qt::ConnectionType type = Qt::AutoConnection);
  /* INSTASTRING - Instaslot with a QSTRING argument
  */     
public slots:
  void slot(QString);
private:
   std::function<void(QString)> foo;
};
    

#endif
