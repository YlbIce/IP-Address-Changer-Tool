#ifndef IPCONFIGMANAGER_H
#define IPCONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>

struct IpConfig {
    QString name;
    QString ipAddress;
    QString subnetMask;
    QString gateway;
    QString dns1;
    QString dns2;
    bool isDhcp;
    QString adapterGuid;  // Associate config with specific adapter
};

Q_DECLARE_METATYPE(IpConfig)

class IpConfigManager : public QObject
{
    Q_OBJECT

public:
    explicit IpConfigManager(QObject *parent = nullptr);

    QVector<IpConfig> getConfigs() const;
    QVector<IpConfig> getConfigsForAdapter(const QString &adapterGuid) const;
    void addConfig(const IpConfig &config);
    void removeConfig(int index);
    void removeConfigForAdapter(const QString &adapterGuid, int index);
    void updateConfig(int index, const IpConfig &config);
    void updateConfigForAdapter(const QString &adapterGuid, int index, const IpConfig &config);
    IpConfig getConfig(int index) const;

    void loadFromFile();
    void saveToFile();

signals:
    void configListChanged();

private:
    QString getConfigFilePath() const;
    IpConfig parseIpConfig(const QJsonObject &obj) const;
    QJsonObject serializeIpConfig(const IpConfig &config) const;

    QVector<IpConfig> m_configs;
};

#endif // IPCONFIGMANAGER_H
