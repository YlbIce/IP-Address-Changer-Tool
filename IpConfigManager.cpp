#include "IpConfigManager.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

IpConfigManager::IpConfigManager(QObject *parent)
    : QObject(parent)
{
    loadFromFile();
}

QVector<IpConfig> IpConfigManager::getConfigs() const
{
    return m_configs;
}

QVector<IpConfig> IpConfigManager::getConfigsForAdapter(const QString &adapterGuid) const
{
    QVector<IpConfig> result;
    for (const IpConfig &config : m_configs) {
        if (config.adapterGuid == adapterGuid) {
            result.append(config);
        }
    }
    return result;
}

void IpConfigManager::addConfig(const IpConfig &config)
{
    m_configs.append(config);
    saveToFile();
    emit configListChanged();
}

void IpConfigManager::removeConfig(int index)
{
    if (index >= 0 && index < m_configs.size()) {
        m_configs.removeAt(index);
        saveToFile();
        emit configListChanged();
    }
}

void IpConfigManager::removeConfigForAdapter(const QString &adapterGuid, int index)
{
    QVector<IpConfig> adapterConfigs = getConfigsForAdapter(adapterGuid);
    if (index >= 0 && index < adapterConfigs.size()) {
        IpConfig configToRemove = adapterConfigs[index];
        // Find and remove from main list
        for (int i = 0; i < m_configs.size(); ++i) {
            if (m_configs[i].name == configToRemove.name &&
                m_configs[i].adapterGuid == configToRemove.adapterGuid &&
                m_configs[i].ipAddress == configToRemove.ipAddress) {
                m_configs.removeAt(i);
                saveToFile();
                emit configListChanged();
                return;
            }
        }
    }
}

void IpConfigManager::updateConfig(int index, const IpConfig &config)
{
    if (index >= 0 && index < m_configs.size()) {
        m_configs[index] = config;
        saveToFile();
        emit configListChanged();
    }
}

void IpConfigManager::updateConfigForAdapter(const QString &adapterGuid, int index, const IpConfig &config)
{
    QVector<IpConfig> adapterConfigs = getConfigsForAdapter(adapterGuid);
    if (index >= 0 && index < adapterConfigs.size()) {
        IpConfig oldConfig = adapterConfigs[index];
        // Find and update in main list
        for (int i = 0; i < m_configs.size(); ++i) {
            if (m_configs[i].adapterGuid == oldConfig.adapterGuid &&
                m_configs[i].name == oldConfig.name &&
                m_configs[i].ipAddress == oldConfig.ipAddress) {
                m_configs[i].name = config.name;
                m_configs[i].ipAddress = config.ipAddress;
                m_configs[i].subnetMask = config.subnetMask;
                m_configs[i].gateway = config.gateway;
                m_configs[i].dns1 = config.dns1;
                m_configs[i].dns2 = config.dns2;
                m_configs[i].isDhcp = config.isDhcp;
                // Keep the original adapterGuid
                saveToFile();
                emit configListChanged();
                return;
            }
        }
    }
}

IpConfig IpConfigManager::getConfig(int index) const
{
    if (index >= 0 && index < m_configs.size()) {
        return m_configs[index];
    }
    return IpConfig();
}

void IpConfigManager::loadFromFile()
{
    QString filePath = getConfigFilePath();
    QFile file(filePath);

    if (!file.exists()) {
        qWarning() << "Config file does not exist:" << filePath;
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse config file:" << error.errorString();
        return;
    }

    m_configs.clear();
    QJsonArray array = doc.array();

    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            m_configs.append(parseIpConfig(value.toObject()));
        }
    }

    qDebug() << "Loaded" << m_configs.size() << "IP configurations";
}

void IpConfigManager::saveToFile()
{
    QString filePath = getConfigFilePath();
    QJsonArray array;

    for (const IpConfig &config : m_configs) {
        array.append(serializeIpConfig(config));
    }

    QJsonDocument doc(array);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save config file:" << filePath;
        return;
    }

    file.write(data);
    file.close();

    qDebug() << "Saved" << m_configs.size() << "IP configurations";
}

QString IpConfigManager::getConfigFilePath() const
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return appDataPath + "/ip_configs.json";
}

IpConfig IpConfigManager::parseIpConfig(const QJsonObject &obj) const
{
    IpConfig config;
    config.name = obj["name"].toString();
    config.ipAddress = obj["ipAddress"].toString();
    config.subnetMask = obj["subnetMask"].toString();
    config.gateway = obj["gateway"].toString();
    config.dns1 = obj["dns1"].toString();
    config.dns2 = obj["dns2"].toString();
    config.isDhcp = obj["isDhcp"].toBool();
    config.adapterGuid = obj["adapterGuid"].toString();
    return config;
}

QJsonObject IpConfigManager::serializeIpConfig(const IpConfig &config) const
{
    QJsonObject obj;
    obj["name"] = config.name;
    obj["ipAddress"] = config.ipAddress;
    obj["subnetMask"] = config.subnetMask;
    obj["gateway"] = config.gateway;
    obj["dns1"] = config.dns1;
    obj["dns2"] = config.dns2;
    obj["isDhcp"] = config.isDhcp;
    obj["adapterGuid"] = config.adapterGuid;
    return obj;
}
