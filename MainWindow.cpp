#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QTimer>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_adapterCombo(nullptr)
    , m_configTableWidget(nullptr)
    , m_ipConfigManager(new IpConfigManager(this))
    , m_networkManager(new NetworkAdapterManager(this))
{
    setupUi();
    createMenuBar();

    // Check for administrator privileges
    if (!NetworkAdapterManager::isAdmin()) {
        m_statusLabel->setText("警告：未以管理员身份运行。修改IP需要管理员权限。");
        m_statusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; }");
    }

    // Show loading message
    m_statusLabel->setText("正在加载网卡列表...");
    m_statusLabel->setStyleSheet("QLabel { color: blue; }");

    // Load adapters asynchronously to prevent UI freezing
    QTimer::singleShot(100, this, &MainWindow::loadAdapters);

    // Connect signals
    connect(m_ipConfigManager, &IpConfigManager::configListChanged,
            this, &MainWindow::onConfigListChanged);
    connect(m_networkManager, &NetworkAdapterManager::operationFinished,
            this, [this](bool success, const QString &message) {
        m_statusLabel->setText(message);
        if (success) {
            m_statusLabel->setStyleSheet("QLabel { color: green; }");
            onRefreshAdapters();
        } else {
            m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
        }
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Adapter selection group
    QGroupBox *adapterGroup = new QGroupBox(tr("Network Adapter"), this);
    QHBoxLayout *adapterLayout = new QHBoxLayout(adapterGroup);

    m_adapterCombo = new QComboBox(this);
    m_adapterCombo->setMinimumWidth(300);
    connect(m_adapterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAdapterChanged);

    m_refreshButton = new QPushButton(tr("Refresh"), this);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshAdapters);

    adapterLayout->addWidget(new QLabel(tr("Select Adapter:"), this));
    adapterLayout->addWidget(m_adapterCombo);
    adapterLayout->addWidget(m_refreshButton);
    adapterLayout->addStretch();

    // IP Configuration table group
    QGroupBox *configGroup = new QGroupBox(QString("IP配置列表"), this);
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);

    m_configTableWidget = new QTableWidget(this);
    m_configTableWidget->setMinimumHeight(250);
    m_configTableWidget->setColumnCount(4);
    m_configTableWidget->setHorizontalHeaderLabels(QStringList() << "配置名称" << "IP地址" << "子网掩码" << "默认网关");
    m_configTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_configTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_configTableWidget->horizontalHeader()->setStretchLastSection(true);
    m_configTableWidget->verticalHeader()->setVisible(false);
    connect(m_configTableWidget, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::onConfigSelected);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_applyButton = new QPushButton(QString("应用"), this);
    m_applyButton->setEnabled(false);
    connect(m_applyButton, &QPushButton::clicked, this, &MainWindow::onApplyConfig);

    m_addButton = new QPushButton(QString("添加"), this);
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddConfig);

    m_editButton = new QPushButton(QString("编辑"), this);
    m_editButton->setEnabled(false);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditConfig);

    m_deleteButton = new QPushButton(QString("删除"), this);
    m_deleteButton->setEnabled(false);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteConfig);

    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);

    configLayout->addWidget(m_configTableWidget);
    configLayout->addLayout(buttonLayout);

    // Current IP info
    QGroupBox *infoGroup = new QGroupBox(tr("Current Status"), this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);

    m_currentIpLabel = new QLabel(tr("Current IP: Not available"), this);
    m_adapterInfoLabel = new QLabel(tr("Adapter Info: Not selected"), this);
    m_adapterInfoLabel->setWordWrap(true);
    m_statusLabel = new QLabel(tr("Ready"), this);

    infoLayout->addWidget(m_currentIpLabel);
    infoLayout->addWidget(m_adapterInfoLabel);
    infoLayout->addWidget(m_statusLabel);

    // Add all groups to main layout
    mainLayout->addWidget(adapterGroup);
    mainLayout->addWidget(configGroup);
    mainLayout->addWidget(infoGroup);

    setCentralWidget(centralWidget);
    setWindowTitle(QString("IP地址切换工具"));

    // Apply dark theme
    applyDarkTheme();
}

void MainWindow::createMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *fileMenu = menuBar->addMenu(tr("&File"));

    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QMenu *helpMenu = menuBar->addMenu(tr("&Help"));

    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("About IP Address Changer"),
                          tr("IP Address Changer Tool v1.0\n\n"
                             "A simple tool to quickly switch between\n"
                             "different IP configurations.\n\n"
                             "Note: This application requires\n"
                             "administrator privileges to change\n"
                             "network settings."));
    });

    setMenuBar(menuBar);
}

void MainWindow::loadAdapters()
{
    m_adapters = m_networkManager->getAdapters();
    m_adapterCombo->clear();

    int maxWidth = 300; // Minimum width

    for (const NetworkAdapter &adapter : m_adapters) {
        QString displayText = adapter.name;
        if (!adapter.description.isEmpty() && adapter.description != adapter.name) {
            displayText += QString(" (%1)").arg(adapter.description);
        }
        m_adapterCombo->addItem(displayText, adapter.name);

        // Calculate width needed for this text
        QFontMetrics fm(m_adapterCombo->font());
        int textWidth = fm.width(displayText) + 50; // Add extra space for padding and dropdown arrow
        if (textWidth > maxWidth) {
            maxWidth = textWidth;
        }
    }

    // Set the combobox width based on content
    m_adapterCombo->setMinimumWidth(maxWidth);

    if (m_adapters.isEmpty()) {
        m_statusLabel->setText(QString("未找到网络适配器"));
        m_statusLabel->setStyleSheet("QLabel { color: orange; }");
    } else {
        // Restore ready message after loading
        if (!NetworkAdapterManager::isAdmin()) {
            m_statusLabel->setText("警告：未以管理员身份运行。修改IP需要管理员权限。");
            m_statusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; }");
        } else {
            m_statusLabel->setText(QString("就绪"));
            m_statusLabel->setStyleSheet("QLabel { color: green; }");
        }
    }

    onAdapterChanged(m_adapterCombo->currentIndex());
}

void MainWindow::onAdapterChanged(int index)
{
    Q_UNUSED(index);
    QString adapterName = getCurrentAdapterName();

    if (!adapterName.isEmpty()) {
        // Find the adapter details first
        QString currentAdapterGuid;
        for (const NetworkAdapter &adapter : m_adapters) {
            if (adapter.name == adapterName) {
                QString infoText = QString("适配器: %1").arg(adapter.description);
                if (!adapter.guid.isEmpty()) {
                    infoText += QString("\nGUID: %1").arg(adapter.guid);
                }
                m_adapterInfoLabel->setText(infoText);
                currentAdapterGuid = adapter.guid;
                break;
            }
        }

        // Refresh config list for this adapter
        refreshConfigList(currentAdapterGuid);

        // Show loading message first
        m_currentIpLabel->setText(QString("Current IP: Loading..."));

        // Use QTimer to delay IP retrieval, preventing UI freezing
        QTimer::singleShot(100, this, [this, adapterName]() {
            QString currentIp = m_networkManager->getCurrentIpAddress(adapterName);
            if (!currentIp.isEmpty()) {
                m_currentIpLabel->setText(QString("Current IP: %1").arg(currentIp));
            } else {
                m_currentIpLabel->setText(QString("Current IP: Not configured or DHCP"));
            }
        });
    } else {
        m_currentIpLabel->setText(QString("Current IP: No adapter selected"));
        m_adapterInfoLabel->setText(QString("Adapter Info: Not selected"));
        m_configTableWidget->setRowCount(0);
    }
}

void MainWindow::onConfigSelected()
{
    bool hasSelection = m_configTableWidget->selectedItems().count() > 0;
    m_applyButton->setEnabled(hasSelection);
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void MainWindow::onApplyConfig()
{
    QString adapterName = getCurrentAdapterName();
    if (adapterName.isEmpty()) {
        QMessageBox::warning(this, QString("错误"), QString("请先选择一个网络适配器。"));
        return;
    }

    // Check for administrator privileges before applying
    if (!NetworkAdapterManager::isAdmin()) {
        QMessageBox::warning(this, QString("权限不足"),
            QString("修改IP地址需要管理员权限。\n\n"
               "请按以下步骤操作：\n"
               "1. 关闭此应用程序\n"
               "2. 右键点击应用程序图标\n"
               "3. 选择\"以管理员身份运行\"\n\n"
               "如果不以管理员身份运行，IP修改将失败。"));
        return;
    }

    int currentRow = m_configTableWidget->currentRow();
    if (currentRow < 0) {
        return;
    }

    // Get the config for current adapter and row
    QString currentAdapterGuid = getCurrentAdapterGuid();
    QVector<IpConfig> adapterConfigs = m_ipConfigManager->getConfigsForAdapter(currentAdapterGuid);

    if (currentRow >= 0 && currentRow < adapterConfigs.size()) {
        IpConfig config = adapterConfigs[currentRow];
        applyConfig(config);
    }
}

void MainWindow::applyConfig(const IpConfig &config)
{
    QString adapterName = getCurrentAdapterName();

    QMessageBox::StandardButton reply;
    QString question = QString("将IP配置 '%1' 应用到网卡 '%2'?\n\n")
                           .arg(config.name, adapterName);

    if (config.isDhcp) {
        question += QString("模式: DHCP (自动获取)");
    } else {
        question += QString("IP地址: %1\n"
                           "子网掩码: %2\n"
                           "默认网关: %3\n"
                           "DNS服务器: %4")
                        .arg(config.ipAddress, config.subnetMask,
                             config.gateway, config.dns1);
    }

    reply = QMessageBox::question(this, QString("确认IP修改"),
                                 question,
                                 QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        bool success;

        if (config.isDhcp) {
            success = m_networkManager->setDhcp(adapterName);
        } else {
            success = m_networkManager->setIpAddress(
                adapterName,
                config.ipAddress,
                config.subnetMask,
                config.gateway,
                config.dns1,
                config.dns2
            );
        }

        if (success) {
            QMessageBox::information(this, QString("成功"),
                                   QString("IP配置已成功应用！\n\n"
                                      "注意：更改可能需要几秒钟才能生效。"));
        } else {
            QMessageBox::critical(this, QString("失败"),
                                QString("应用IP配置失败。\n\n"
                                   "错误信息显示在状态栏中。\n\n"
                                   "请确保您以管理员身份运行此程序。"));
        }
    }
}

QString MainWindow::getCurrentAdapterName() const
{
    if (m_adapterCombo->currentIndex() < 0) {
        return QString();
    }
    return m_adapterCombo->currentData().toString();
}

QString MainWindow::getCurrentAdapterGuid() const
{
    if (m_adapterCombo->currentIndex() < 0) {
        return QString();
    }
    QString adapterName = m_adapterCombo->currentData().toString();
    for (const NetworkAdapter &adapter : m_adapters) {
        if (adapter.name == adapterName) {
            return adapter.guid;
        }
    }
    return QString();
}

void MainWindow::onAddConfig()
{
    QString adapterGuid = getCurrentAdapterGuid();
    if (adapterGuid.isEmpty()) {
        QMessageBox::warning(this, QString("错误"), QString("请先选择一个网络适配器。"));
        return;
    }
    showAddConfigDialog();
}

void MainWindow::showAddConfigDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString("添加IP配置"));

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *ipEdit = new QLineEdit(&dialog);
    QLineEdit *subnetEdit = new QLineEdit(&dialog);
    QLineEdit *gatewayEdit = new QLineEdit(&dialog);
    QLineEdit *dns1Edit = new QLineEdit(&dialog);
    QLineEdit *dns2Edit = new QLineEdit(&dialog);
    QCheckBox *dhcpCheckBox = new QCheckBox(QString("使用DHCP（自动获取IP）"), &dialog);

    ipEdit->setPlaceholderText("192.168.1.100");
    subnetEdit->setPlaceholderText("255.255.255.0");
    gatewayEdit->setPlaceholderText("192.168.1.1");
    dns1Edit->setPlaceholderText("8.8.8.8");
    dns2Edit->setPlaceholderText("8.8.4.4");

    formLayout->addRow(QString("配置名称:"), nameEdit);
    formLayout->addRow(dhcpCheckBox);
    formLayout->addRow(QString("IP地址:"), ipEdit);
    formLayout->addRow(QString("子网掩码:"), subnetEdit);
    formLayout->addRow(QString("默认网关:"), gatewayEdit);
    formLayout->addRow(QString("首选DNS:"), dns1Edit);
    formLayout->addRow(QString("备用DNS:"), dns2Edit);

    // Enable/disable fields based on DHCP checkbox
    auto updateFields = [dhcpCheckBox, ipEdit, subnetEdit, gatewayEdit, dns1Edit, dns2Edit]() {
        bool enabled = !dhcpCheckBox->isChecked();
        ipEdit->setEnabled(enabled);
        subnetEdit->setEnabled(enabled);
        gatewayEdit->setEnabled(enabled);
        dns1Edit->setEnabled(enabled);
        dns2Edit->setEnabled(enabled);
    };

    connect(dhcpCheckBox, &QCheckBox::stateChanged, updateFields);
    updateFields();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText(QString("确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(QString("取消"));
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        IpConfig config;
        config.name = nameEdit->text();
        config.isDhcp = dhcpCheckBox->isChecked();
        config.adapterGuid = getCurrentAdapterGuid();

        if (!config.isDhcp) {
            config.ipAddress = ipEdit->text();
            config.subnetMask = subnetEdit->text();
            config.gateway = gatewayEdit->text();
            config.dns1 = dns1Edit->text();
            config.dns2 = dns2Edit->text();
        }

        if (config.name.isEmpty()) {
            QMessageBox::warning(this, QString("错误"), QString("请输入配置名称。"));
            return;
        }

        m_ipConfigManager->addConfig(config);
        QMessageBox::information(this, QString("成功"), QString("IP配置添加成功。"));
    }
}

void MainWindow::onEditConfig()
{
    int currentRow = m_configTableWidget->currentRow();
    if (currentRow < 0) {
        return;
    }

    showEditConfigDialog(currentRow);
}

void MainWindow::showEditConfigDialog(int index)
{
    QString currentAdapterGuid = getCurrentAdapterGuid();
    QVector<IpConfig> adapterConfigs = m_ipConfigManager->getConfigsForAdapter(currentAdapterGuid);

    if (index < 0 || index >= adapterConfigs.size()) {
        return;
    }

    IpConfig config = adapterConfigs[index];

    QDialog dialog(this);
    dialog.setWindowTitle(QString("编辑IP配置"));

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit(config.name, &dialog);
    QLineEdit *ipEdit = new QLineEdit(config.ipAddress, &dialog);
    QLineEdit *subnetEdit = new QLineEdit(config.subnetMask, &dialog);
    QLineEdit *gatewayEdit = new QLineEdit(config.gateway, &dialog);
    QLineEdit *dns1Edit = new QLineEdit(config.dns1, &dialog);
    QLineEdit *dns2Edit = new QLineEdit(config.dns2, &dialog);
    QCheckBox *dhcpCheckBox = new QCheckBox(QString("使用DHCP（自动获取IP）"), &dialog);
    dhcpCheckBox->setChecked(config.isDhcp);

    formLayout->addRow(QString("配置名称:"), nameEdit);
    formLayout->addRow(dhcpCheckBox);
    formLayout->addRow(QString("IP地址:"), ipEdit);
    formLayout->addRow(QString("子网掩码:"), subnetEdit);
    formLayout->addRow(QString("默认网关:"), gatewayEdit);
    formLayout->addRow(QString("首选DNS:"), dns1Edit);
    formLayout->addRow(QString("备用DNS:"), dns2Edit);

    // Enable/disable fields based on DHCP checkbox
    auto updateFields = [dhcpCheckBox, ipEdit, subnetEdit, gatewayEdit, dns1Edit, dns2Edit]() {
        bool enabled = !dhcpCheckBox->isChecked();
        ipEdit->setEnabled(enabled);
        subnetEdit->setEnabled(enabled);
        gatewayEdit->setEnabled(enabled);
        dns1Edit->setEnabled(enabled);
        dns2Edit->setEnabled(enabled);
    };

    connect(dhcpCheckBox, &QCheckBox::stateChanged, updateFields);
    updateFields();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttonBox->button(QDialogButtonBox::Ok)->setText(QString("确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(QString("取消"));
    formLayout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        config.name = nameEdit->text();
        config.isDhcp = dhcpCheckBox->isChecked();

        if (!config.isDhcp) {
            config.ipAddress = ipEdit->text();
            config.subnetMask = subnetEdit->text();
            config.gateway = gatewayEdit->text();
            config.dns1 = dns1Edit->text();
            config.dns2 = dns2Edit->text();
        }

        // Update in the main list using the new method
        m_ipConfigManager->updateConfigForAdapter(currentAdapterGuid, index, config);
        QMessageBox::information(this, QString("成功"), QString("IP配置更新成功。"));
    }
}

void MainWindow::onDeleteConfig()
{
    int currentRow = m_configTableWidget->currentRow();
    if (currentRow < 0) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        QString("确认删除"),
        QString("确定要删除此IP配置吗？"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QString currentAdapterGuid = getCurrentAdapterGuid();
        m_ipConfigManager->removeConfigForAdapter(currentAdapterGuid, currentRow);
        QMessageBox::information(this, QString("成功"), QString("IP配置删除成功。"));
    }
}

void MainWindow::onRefreshAdapters()
{
    loadAdapters();
    m_statusLabel->setText(tr("Network adapters refreshed"));
    m_statusLabel->setStyleSheet("QLabel { color: green; }");
}

void MainWindow::refreshConfigList()
{
    // Refresh with current adapter
    QString currentAdapterGuid = getCurrentAdapterGuid();
    if (!currentAdapterGuid.isEmpty()) {
        refreshConfigList(currentAdapterGuid);
    } else {
        m_configTableWidget->setRowCount(0);
    }
}

void MainWindow::refreshConfigList(const QString &adapterGuid)
{
    m_configTableWidget->setRowCount(0);
    QVector<IpConfig> configs = m_ipConfigManager->getConfigsForAdapter(adapterGuid);

    m_configTableWidget->setRowCount(configs.size());

    for (int i = 0; i < configs.size(); ++i) {
        const IpConfig &config = configs[i];

        // Configuration name
        QTableWidgetItem *nameItem = new QTableWidgetItem(config.name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_configTableWidget->setItem(i, 0, nameItem);

        if (config.isDhcp) {
            // Show DHCP mode
            QTableWidgetItem *ipItem = new QTableWidgetItem("DHCP");
            ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
            m_configTableWidget->setItem(i, 1, ipItem);

            QTableWidgetItem *maskItem = new QTableWidgetItem("-");
            maskItem->setFlags(maskItem->flags() & ~Qt::ItemIsEditable);
            m_configTableWidget->setItem(i, 2, maskItem);

            QTableWidgetItem *gatewayItem = new QTableWidgetItem("-");
            gatewayItem->setFlags(gatewayItem->flags() & ~Qt::ItemIsEditable);
            m_configTableWidget->setItem(i, 3, gatewayItem);
        } else {
            // Show static IP details
            QTableWidgetItem *ipItem = new QTableWidgetItem(config.ipAddress);
            ipItem->setFlags(ipItem->flags() & ~Qt::ItemIsEditable);
            m_configTableWidget->setItem(i, 1, ipItem);

            QTableWidgetItem *maskItem = new QTableWidgetItem(config.subnetMask);
            maskItem->setFlags(maskItem->flags() & ~Qt::ItemIsEditable);
            m_configTableWidget->setItem(i, 2, maskItem);

            QTableWidgetItem *gatewayItem = new QTableWidgetItem(config.gateway);
            gatewayItem->setFlags(gatewayItem->flags() & ~Qt::ItemIsEditable);
            m_configTableWidget->setItem(i, 3, gatewayItem);
        }
    }

    m_configTableWidget->setCurrentCell(-1, -1);
    onConfigSelected();
}

void MainWindow::onConfigListChanged()
{
    refreshConfigList();
}

void MainWindow::applyDarkTheme()
{
    // Dark theme stylesheet
    QString darkStyle = R"(
        QMainWindow {
            background-color: #2b2b2b;
        }

        QWidget {
            background-color: #2b2b2b;
            color: #ffffff;
        }

        QGroupBox {
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 5px;
            margin-top: 20px;
            padding-top: 10px;
            font-weight: bold;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }

        QPushButton {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px 15px;
            min-width: 80px;
        }

        QPushButton:hover {
            background-color: #4d4d4d;
            border: 1px solid #666666;
        }

        QPushButton:pressed {
            background-color: #2d2d2d;
        }

        QPushButton:disabled {
            background-color: #2b2b2b;
            color: #666666;
            border: 1px solid #333333;
        }

        QComboBox {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 3px 18px 3px 5px;
            min-width: 80px;
        }

        QComboBox:hover {
            border: 1px solid #666666;
        }

        QComboBox::drop-down {
            border: none;
        }

        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #ffffff;
            width: 0;
            height: 0;
        }

        QComboBox QAbstractItemView {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
            selection-background-color: #4a6fa5;
            selection-color: #ffffff;
        }

        QTableWidget {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
            gridline-color: #555555;
            selection-background-color: #4a6fa5;
            alternate-background-color: #353535;
        }

        QTableWidget::item:selected {
            background-color: #4a6fa5;
            color: #ffffff;
        }

        QTableWidget::item:hover {
            background-color: #454545;
        }

        QHeaderView::section {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
            padding: 5px;
            font-weight: bold;
        }

        QLabel {
            color: #ffffff;
        }

        QLineEdit {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 3px;
        }

        QLineEdit:focus {
            border: 1px solid #4a6fa5;
        }

        QCheckBox {
            color: #ffffff;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 1px solid #555555;
            border-radius: 3px;
            background-color: #3d3d3d;
        }

        QCheckBox::indicator:checked {
            background-color: #4a6fa5;
            border: 1px solid #4a6fa5;
        }

        QMenuBar {
            background-color: #2b2b2b;
            color: #ffffff;
            border-bottom: 1px solid #555555;
        }

        QMenuBar::item {
            background-color: transparent;
            padding: 5px 10px;
        }

        QMenuBar::item:selected {
            background-color: #3d3d3d;
        }

        QMenu {
            background-color: #3d3d3d;
            color: #ffffff;
            border: 1px solid #555555;
        }

        QMenu::item {
            padding: 5px 20px;
        }

        QMenu::item:selected {
            background-color: #4a6fa5;
        }

        QDialog {
            background-color: #2b2b2b;
        }

        QDialogButtonBox QPushButton {
            min-width: 80px;
        }

        QScrollBar:vertical {
            background-color: #2b2b2b;
            width: 12px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background-color: #555555;
            min-height: 20px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #666666;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            background-color: #2b2b2b;
            height: 12px;
            margin: 0px;
        }

        QScrollBar::handle:horizontal {
            background-color: #555555;
            min-width: 20px;
            border-radius: 4px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: #666666;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )";

    setStyleSheet(darkStyle);
}
