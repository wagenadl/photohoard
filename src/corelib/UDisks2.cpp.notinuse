// UDisks2.cpp

#include "UDisks2.h"

/* Forked from https://github.com/cmdrkotori/udisks2.
   Retrieved 1/30/2017.
   Licensed under GPL-2.0.

   DAW has not changed the code in a real way, except to match coding
   style to the rest of photohoard.
*/

namespace UDisks2 {

  // Custom type for unmarhsalling byte arrays
  static QString char2data(QList<unsigned char> const &data) {
    QString output;
    foreach(unsigned char c, data)
      if (c)
        output.append(c);
    return output;
  }

  // Q_DECLARE_METATYPE(dbus_ay)

  UDisks2::UDisks2(QObject *parent): QObject(parent) {
    auto system = QDBusConnection::systemBus();
    system.connect("org.freedesktop.UDisks2",
                   "/org/freedesktop/UDisks2",
                   "org.freedesktop.DBus.ObjectManager",
                   "InterfacesAdded",
                   this,
                   SLOT(dbusIFaceAdded(QDBusObjectPath,
                                       QMap<QString,QVariant>)));

    system.connect("org.freedesktop.UDisks2",
                   "/org/freedesktop/UDisks2",
                   "org.freedesktop.DBus.ObjectManager",
                   "InterfacesRemoved",
                   this,
                   SLOT(dbusIFaceRemoved(QDBusObjectPath,QStringList)));

    foreach (QString block, blockDevices()) 
      addBlock(block);
    foreach (QString drive, drives()) 
      addDrive(drive);
  }

  static QString lastPart(const QString &path) {
    return path.split('/').last();
  }

  UDisks2::~UDisks2() {
  }

  QStringList UDisks2::blockDevices() {
    QDBusInterface ud2("org.freedesktop.UDisks2",
                       "/org/freedesktop/UDisks2/block_devices",
                       "org.freedesktop.DBus.Introspectable",
                       QDBusConnection::systemBus());
    QDBusReply<QString> reply = ud2.call("Introspect");
    if (!reply.isValid())
      return QStringList();
    QXmlStreamReader xml(reply.value());
    QStringList response;
    while (!xml.atEnd()) {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::StartElement
          && xml.name().toString() == "node") {
        QString name = xml.attributes().value("name").toString();
        if (!name.isEmpty())
          response << name;
      }
    }
    return response;
  }

  Block *UDisks2::blockDevice(const QString &node) const {
    return blocks_.contains(node) ? blocks_[node] : NULL;
  }

  QStringList UDisks2::drives() {
    QDBusInterface ud2("org.freedesktop.UDisks2",
                       "/org/freedesktop/UDisks2/drives",
                       "org.freedesktop.DBus.Introspectable",
                       QDBusConnection::systemBus());
    if (!ud2.isValid())
      return QStringList();
    QDBusReply<QString> reply = ud2.call("Introspect");
    if (!reply.isValid())
      return QStringList();
    QXmlStreamReader xml(reply.value());
    QStringList response;
    while (!xml.atEnd()) {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::StartElement
          && xml.name().toString() == "node") {
        QString name = xml.attributes().value("name").toString();
        if (!name.isEmpty())
          response << name;
      }
    }
    return response;
  }

  Drive *UDisks2::drive(const QString &node) const {
    return drives_.contains(node) ? drives_[node] : NULL;
  }

  void UDisks2::addDrive(const QString &node) {
    if (drives_.contains(node)) {
      drives_.value(node)->update();
    } else {
      auto drive = new Drive(node, this);
      connect(drive, &Drive::changed,
              this, &UDisks2::driveChanged);
      drives_.insert(node, drive);
      emit driveAdded(node);
    }
  }

  void UDisks2::addBlock(const QString &node) {
    if (blocks_.contains(node)) {
      blocks_.value(node)->update();
    } else {
      auto block = new Block(node, this);
      connect(block, &Block::fileSystemAdded,
              this, &UDisks2::fileSystemAdded);
      connect(block, &Block::fileSystemRemoved,
              this, &UDisks2::fileSystemRemoved);
      connect(block, &Block::fileSystemChanged,
              this, &UDisks2::fileSystemChanged);
      connect(block, &Block::changed,
              this, &UDisks2::blockDeviceChanged);
      blocks_.insert(node, block);
      emit blockDeviceAdded(node);
    }
  }

  void UDisks2::removeDrive(const QString &node) {
    if (drives_.contains(node))
      delete drives_.take(node);
  }

  void UDisks2::removeBlock(const QString &node) {
    if (blocks_.contains(node))
      delete blocks_.take(node);
  }

  void UDisks2::dbusIFaceAdded(const QDBusObjectPath &path,
                               const QMap<QString, QVariant> &ifaces) {
    // path: o [path]
    // nodes: a{sa{sv} [dict of strings dict of string variants]
    QString node = lastPart(path.path());
    if (path.path().startsWith("/org/freedesktop/UDisks2/block_devices")) {
      if (ifaces.contains("org.freedesktop.UDisks2.Block"))
        addBlock(node);
      if (ifaces.contains("org.freedesktop.UDisks2.Filesystem")
          && blocks_.contains(node))
        blocks_[node]->addFileSystem();
    } else if (path.path().startsWith("/org/freedesktop/UDisks2/drives")) {
      if (ifaces.contains("org.freedesktop.UDisks2.Drive"))
        addDrive(node);
    }
  }

  void UDisks2::dbusIFaceRemoved(const QDBusObjectPath &path,
                                 const QStringList &ifaces) {
    // path: o [path]
    // ifaces: as [list of strings]
    QString node = lastPart(path.path());
    if (path.path().startsWith("/org/freedesktop/UDisks2/block_devices")) {
      if (ifaces.contains("org.freedesktop.UDisks2.Block")) {
        removeBlock(node);
        emit blockDeviceRemoved(node);
      }
      if (ifaces.contains("org.freedesktop.UDisks2.Filesystem")
          && blocks_.contains(node)) {
        blocks_[node]->removeFileSystem();
      }
    } else if (path.path().startsWith("/org/freedesktop/UDisks2/drives")) {
      removeDrive(node);
      emit driveRemoved(node);
    }
  }

  Block::Block(const QString &node, QObject *parent):
    QObject(parent), name_(node), fs(NULL) {
    QDBusConnection system = QDBusConnection::systemBus();
    dbus = new QDBusInterface("org.freedesktop.UDisks2",
                              "/org/freedesktop/UDisks2/block_devices/" + node,
                              "org.freedesktop.UDisks2.Block",
                              system, parent);
    system.connect(dbus->service(), dbus->path(),
                   "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   this,
                   SLOT(propertiesChanged(QString,
                                          QVariantMap,
                                          QStringList)));
    update();
    if (!type_.isEmpty())
      addFileSystem();
  }

  QString Block::toString() const {
    return QString("name: %1\ndev: %2\nid: %3\ndrive: %4\nsize: %5\n"
                   "readonly: %6\nusage: %7\ntype: %8")
      .arg(name_).arg(dev_).arg(id_).arg(drive_).arg(size_).arg(readonly_)
      .arg(usage_).arg(type_);
  }

  void Block::update() {
    dev_ = dbus->property("Device").toString();
    id_ = dbus->property("Id").toString();
    drive_ = lastPart(dbus->property("Drive").value<QDBusObjectPath>().path());
    size_ = dbus->property("Size").toULongLong();
    readonly_ = dbus->property("ReadOnly").toBool();
    usage_ = dbus->property("IdUsage").toString();
    type_ = dbus->property("IdType").toString();
  }

  void Block::updateFileSystem() {
    if (fs)
      fs->update();
  }

  void Block::addFileSystem() {
    if (fs && !fs->isValid()) {
      delete fs;
      fs = NULL;
    }
    if (!fs)
      fs = new FileSystem(name_);
    emit fileSystemAdded(name_);
  }

  void Block::removeFileSystem() {
    emit fileSystemRemoved(name_);
    if (fs) delete fs;
    fs = NULL;
  }

  FileSystem *Block::fileSystem() const {
    return fs;
  }

  void Block::propertiesChanged(const QString &interface,
                                const QVariantMap &/*changedProp*/,
                                const QStringList &/*invalidatedProp*/) {
    if (interface == "org.freedesktop.UDisks2.Block") {
      update();
      emit changed(name_);
    }
    if (interface == "org.freedesktop.UDisks2.Filesystem") {
      updateFileSystem();
      emit fileSystemChanged(name_);
    }
  }


  Drive::Drive(const QString &node, QObject *parent) :
    QObject(parent), name_(node) {
    QDBusConnection system = QDBusConnection::systemBus();
    dbus = new QDBusInterface("org.freedesktop.UDisks2",
                              "/org/freedesktop/UDisks2/drives/" + node,
                              "org.freedesktop.UDisks2.Drive",
                              system, parent);
    system.connect(dbus->service(), dbus->path(),
                   "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   this,
                   SLOT(propertiesChanged(QString,
                                          QVariantMap,
                                          QStringList)));
    update();
  }

  QString Drive::toString() const {
    return QString("name: %1\nsize: %2\nvendor: %3\nmodel: %4\nserial: %5\n"
                   "id: %6\nmedia: %7\noptical: %8\n"
                   "removable: %9\navailable: %10")
      .arg(name_).arg(size_).arg(vendor_).arg(model_).arg(serial_).arg(id_)
      .arg(media_).arg(optical_).arg(removable_).arg(available_);
  }

  void Drive::update() {
    size_ = dbus->property("Size").toULongLong();
    vendor_ = dbus->property("Vendor").toString();
    model_ = dbus->property("Model").toString();
    serial_ = dbus->property("Serial").toString();
    id_ = dbus->property("Id").toString();
    media_ = dbus->property("Media").toString();
    optical_ = dbus->property("Optical").toBool();
    removable_ = dbus->property("MediaRemovable").toBool();
    available_ = dbus->property("MediaAvailable").toBool();
  }

  void Drive::propertiesChanged(const QString &/*interface*/,
                                const QVariantMap &/*changedProp*/,
                                const QStringList &/*invalidatedProp*/) {
    update();
    emit changed(name_);
  }


  FileSystem::FileSystem(const QString &node, QObject *parent):
    QObject(parent) {
    name_ = node;
    QDBusConnection system = QDBusConnection::systemBus();
    dbus = new QDBusInterface("org.freedesktop.UDisks2",
                              "/org/freedesktop/UDisks2/block_devices/" + node,
                              "org.freedesktop.UDisks2.Filesystem",
                              system, parent);
    dbusProp = new QDBusInterface("org.freedesktop.UDisks2",
                                  "/org/freedesktop/UDisks2/block_devices/"
                                  + node,
                                  "org.freedesktop.DBus.Properties",
                                  system, parent);
    emit update();
  }

  QStringList FileSystem::mountPoints() const {
    return mountPoints_;
  }

  QString FileSystem::mount() {
    QDBusMessage reply = dbus->call("Mount", QVariantMap());
    return reply.arguments().first().toString();
  }

  void FileSystem::unmount() {
    dbus->call("Unmount", QVariantMap());
  }

  void FileSystem::update() {
    mountPoints_.clear();
    QDBusMessage reply = dbusProp->call("Get",
                                        "org.freedesktop.UDisks2.Filesystem",
                                        "MountPoints");
    QVariant v = reply.arguments().first();
    QDBusArgument arg
      = v.value<QDBusVariant>().variant().value<QDBusArgument>();
    arg.beginArray();
    while (!arg.atEnd()) {
      QList<unsigned char> data;
      arg >> data;
      QString str = char2data(data);
      mountPoints_.append(str);
    }
  }

  bool FileSystem::isValid() const {
    return dbus->isValid();
  }

};
