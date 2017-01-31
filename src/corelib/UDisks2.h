#ifndef UDISKS2_H
#define UDISKS2_H

#include <QObject>
#include <QtDBus>

/* Forked from https://github.com/cmdrkotori/udisks2.
   Retrieved 1/30/2017.
   Licensed under GPL-2.0.
 */

namespace UDisks2 {
  class Drive: public QObject {
    Q_OBJECT
  public:
    explicit Drive(const QString &node, QObject *parent=0);
  public:
    QString name() const { return name_; }
    qulonglong size() const { return size_; }
    QString vendor() const { return vendor_; }
    QString model() const { return model_; }
    QString serial() const { return serial_; }
    QString id() const { return id_; }
    QString media() const { return media_; }
    bool isOptical() const { return optical_; }
    bool isRemovable() const { return removable_; }
    bool isAvailable() const { return available_; }
    QString toString() const;
  public slots:
    void update();
  signals:
    void changed(const QString &node);
  private slots:
    void propertiesChanged(const QString &interface,
                           const QVariantMap &changed,
                           const QStringList &invalidated);
  private:
    QDBusInterface *dbus;
    QString name_;
    qulonglong size_;
    QString vendor_;
    QString model_;
    QString serial_;
    QString id_;
    QString media_;
    bool optical_;
    bool removable_;
    bool available_;
  };

  class FileSystem: public QObject {
    Q_OBJECT
  public:
    FileSystem(const QString &node, QObject *parent=0);
  public slots:
    QString mount();
    void unmount();
    void update();
  public:
    bool isValid() const;
    QString name() const { return name_; }
    QStringList mountPoints() const;
  private:
    QDBusInterface *dbus;
    QDBusInterface *dbusProp;
    QStringList mountPoints_;
    QString name_;
  };

  class Block: public QObject {
    Q_OBJECT
  public:
    explicit Block(const QString &node, QObject *parent=0);
  public:
    QString name() const { return name_; }
    QString dev() const { return dev_; }
    QString id() const { return id_; }
    QString drive() const { return drive_; }
    qulonglong size() const { return size_; }
    bool isReadOnly() const { return readonly_; }
    QString usage() const { return usage_; }
    QString type() const { return type_; }
    QString toString() const;
    FileSystem *fileSystem() const;
  public slots:
    void update();
    void updateFileSystem();
    void addFileSystem();
    void removeFileSystem();
  signals:
    void fileSystemAdded(const QString &node);
    void fileSystemRemoved(const QString &node);
    void fileSystemChanged(const QString &node);
    void changed(const QString &node);

  private slots:
    void propertiesChanged(const QString &interface,
                           const QVariantMap &changedProp,
                           const QStringList &invalidatedProp);
  private:
    QDBusInterface *dbus;
    QString name_;
    QString dev_;
    QString id_;
    QString drive_;
    qulonglong size_;
    bool readonly_;
    QString usage_;
    QString type_;
    FileSystem *fs;
  };

  class UDisks2: public QObject {
    Q_OBJECT
  public:
    explicit UDisks2(QObject *parent=0);
    ~UDisks2();

    QStringList blockDevices();
    Block *blockDevice(const QString &node) const;

    QStringList drives();
    Drive *drive(const QString &node) const;

  signals:
    void deviceInformationChanged(QString node, QVariantMap info);
    void driveAdded(const QString& node);
    void driveRemoved(const QString& node);
    void driveChanged(const QString& node);
    void blockDeviceAdded(const QString& node);
    void blockDeviceRemoved(const QString &node);
    void blockDeviceChanged(const QString &node);
    void fileSystemAdded(const QString& node);
    void fileSystemRemoved(const QString &node);
    void fileSystemChanged(const QString &node);

  private:
    void addDrive(const QString &node);
    void addBlock(const QString &node);
    void removeDrive(const QString &node);
    void removeBlock(const QString &node);

  private slots:
    void dbusIFaceAdded(const QDBusObjectPath &path,
                             const QMap<QString, QVariant> &ifaces);
    void dbusIFaceRemoved(const QDBusObjectPath &path,
                               const QStringList &ifaces);

  private:
    QMap<QString, Drive *> drives_;
    QMap<QString, Block *> blocks_;
  };
};

#endif // UDISKS2_H
