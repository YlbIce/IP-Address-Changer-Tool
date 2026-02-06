#include "NetworkAdapterManager.h"
#include <QProcess>
#include <QRegularExpression>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>

NetworkAdapterManager::NetworkAdapterManager(QObject *parent)
    : QObject(parent)
{
}

QVector<NetworkAdapter> NetworkAdapterManager::getAdapters() const
{
    QVector<NetworkAdapter> adapters;

    // Execute PowerShell command directly with UTF-8 encoding
    QProcess process;
    process.start("powershell", QStringList()
        << "-Command"
        << "[Console]::OutputEncoding = [System.Text.Encoding]::UTF8; Get-NetAdapter | Select-Object Name,InterfaceDescription,InterfaceGuid | ConvertTo-Csv -NoTypeInformation");
    process.waitForFinished(30000);

    QByteArray outputBytes = process.readAllStandardOutput();
    QString output = QString::fromUtf8(outputBytes);
    QStringList lines = output.split('\n');

    // Skip the header line
    if (lines.size() > 0) {
        lines.removeFirst();
    }

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();

        if (trimmed.isEmpty()) {
            continue;
        }

        // Parse CSV line: "以太网","Realtek PCIe GbE Family Controller","{12345678-1234-1234-1234-123456789abc}"
        // Remove quotes and split by comma
        QString unquoted = trimmed;
        unquoted.remove('"');
        QStringList parts = unquoted.split(',');

        if (parts.size() >= 3) {
            NetworkAdapter adapter;
            adapter.name = parts[0].trimmed();
            adapter.description = parts[1].trimmed();
            QString guid = parts[2].trimmed();
            guid.remove('{').remove('}');
            adapter.guid = guid;

            // Skip virtual adapters
            if (adapter.description.contains("Virtual", Qt::CaseInsensitive) ||
                adapter.description.contains("Hyper-V", Qt::CaseInsensitive) ||
                adapter.name.contains("Loopback", Qt::CaseInsensitive) ||
                adapter.description.contains("Bluestacks", Qt::CaseInsensitive) ||
                adapter.description.contains("VMware", Qt::CaseInsensitive) ||
                adapter.description.contains("VirtualBox", Qt::CaseInsensitive)) {
                continue;
            }

            adapters.append(adapter);
        }
    }

    return adapters;
}

bool NetworkAdapterManager::setIpAddress(const QString &adapterName,
                                         const QString &ipAddress,
                                         const QString &subnetMask,
                                         const QString &gateway,
                                         const QString &dns1,
                                         const QString &dns2)
{
    // Check if running as administrator
    if (!isAdmin()) {
        emit operationFinished(false, "错误：需要管理员权限修改IP地址。请右键点击应用程序，选择\"以管理员身份运行\"。");
        return false;
    }

    // Set static IP
    QString setIpCmd = QString("netsh interface ip set address \"%1\" static %2 %3")
                          .arg(adapterName, ipAddress, subnetMask);

    QString result = executeCommand(setIpCmd);

    // Check for errors
    if (result.contains("请求的操作需要提升", Qt::CaseInsensitive) ||
        result.contains("administrator", Qt::CaseInsensitive)) {
        emit operationFinished(false, "错误：需要管理员权限。请以管理员身份运行此应用程序。");
        return false;
    }

    // Set gateway if provided
    if (!gateway.isEmpty()) {
        QString setGatewayCmd = QString("netsh interface ip set address \"%1\" static %2 %3 %4")
                                    .arg(adapterName, ipAddress, subnetMask, gateway);
        result = executeCommand(setGatewayCmd);
    }

    // Set DNS servers
    if (!dns1.isEmpty()) {
        QString setDnsCmd = QString("netsh interface ip set dns \"%1\" static %2")
                                .arg(adapterName, dns1);
        result = executeCommand(setDnsCmd);
    }

    if (!dns2.isEmpty()) {
        QString addDnsCmd = QString("netsh interface ip add dns \"%1\" %2")
                                .arg(adapterName, dns2);
        result = executeCommand(addDnsCmd);
    }

    emit operationFinished(true, "IP地址修改成功！");
    return true;
}

bool NetworkAdapterManager::setDhcp(const QString &adapterName)
{
    // Check if running as administrator
    if (!isAdmin()) {
        emit operationFinished(false, "错误：需要管理员权限切换到DHCP。请右键点击应用程序，选择\"以管理员身份运行\"。");
        return false;
    }

    QString cmd = QString("netsh interface ip set address \"%1\" dhcp").arg(adapterName);
    QString result = executeCommand(cmd);

    // Check for errors
    if (result.contains("请求的操作需要提升", Qt::CaseInsensitive) ||
        result.contains("administrator", Qt::CaseInsensitive)) {
        emit operationFinished(false, "错误：需要管理员权限。请以管理员身份运行此应用程序。");
        return false;
    }

    QString dnsCmd = QString("netsh interface ip set dns \"%1\" dhcp").arg(adapterName);
    result = executeCommand(dnsCmd);

    emit operationFinished(true, "已成功切换到DHCP模式！");
    return true;
}

QString NetworkAdapterManager::getCurrentIpAddress(const QString &adapterName) const
{
    // Use PowerShell to get IP address (works even if adapter is disconnected)
    QProcess process;
    QString psCommand = QString(
        "Get-NetAdapter -Name '%1' | Get-NetIPAddress -AddressFamily IPv4 -ErrorAction SilentlyContinue | Select-Object -ExpandProperty IPAddress"
    ).arg(adapterName);

    process.start("powershell", QStringList() << "-Command" << psCommand);
    process.waitForFinished(5000);  // Reduced timeout to 5 seconds

    QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();

    if (!output.isEmpty()) {
        // Remove any quotes or extra whitespace
        output.remove('"');
        output = output.trimmed();
        return output;
    }

    return QString();
}

QString NetworkAdapterManager::executeCommand(const QString &command) const
{
    QProcess process;
    process.start("cmd", QStringList() << "/c" << command);
    process.waitForFinished(30000); // 30 seconds timeout

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString error = QString::fromLocal8Bit(process.readAllStandardError());

    if (!error.isEmpty()) {
        // Log error only if it's critical
        if (!output.isEmpty() && !output.contains("successfully", Qt::CaseInsensitive)) {
            qDebug() << "Command error:" << error;
        }
    }

    return output;
}

bool NetworkAdapterManager::runElevated(const QString &command)
{
    // This would be used if we need to run with admin privileges
    // For now, the user needs to run the application as administrator
    Q_UNUSED(command);
    return false;
}

bool NetworkAdapterManager::isAdmin()
{
    // Check if running as administrator on Windows
    QProcess process;
    process.start("net", QStringList() << "session");
    process.waitForFinished(3000);

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString error = QString::fromLocal8Bit(process.readAllStandardError());

    // If we get "access denied" or similar errors, we're not admin
    if (error.contains("拒绝访问", Qt::CaseInsensitive) ||
        error.contains("access denied", Qt::CaseInsensitive) ||
        output.contains("拒绝访问", Qt::CaseInsensitive) ||
        output.contains("access denied", Qt::CaseInsensitive)) {
        return false;
    }

    // Alternative method: try to write to a system-protected location
    // This is more reliable
    QString testPath = "C:\\Windows\\Temp\\admin_test_temp_file_12345.tmp";
    QFile testFile(testPath);

    // Try to create and write to a file in Windows Temp
    if (testFile.open(QIODevice::WriteOnly)) {
        testFile.write("test");
        testFile.close();
        testFile.remove();
        return true;  // Successfully wrote to system directory
    }

    return false;  // Failed to write - not admin
}
