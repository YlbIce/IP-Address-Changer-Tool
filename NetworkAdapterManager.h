#ifndef NETWORKADAPTERMANAGER_H
#define NETWORKADAPTERMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

struct NetworkAdapter {
    QString name;
    QString description;
    QString guid;
};

class NetworkAdapterManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkAdapterManager(QObject *parent = nullptr);

    QVector<NetworkAdapter> getAdapters() const;
    bool setIpAddress(const QString &adapterName, const QString &ipAddress,
                      const QString &subnetMask, const QString &gateway,
                      const QString &dns1, const QString &dns2);
    bool setDhcp(const QString &adapterName);
    QString getCurrentIpAddress(const QString &adapterName) const;
    static bool isAdmin();

    static bool runElevated(const QString &command);

signals:
    void operationFinished(bool success, const QString &message);

private:
    QString executeCommand(const QString &command) const;
    NetworkAdapter parseAdapterInfo(const QString &info) const;
};

#endif // NETWORKADAPTERMANAGER_H
