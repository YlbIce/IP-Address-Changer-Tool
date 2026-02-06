#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include "IpConfigManager.h"
#include "NetworkAdapterManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAdapterChanged(int index);
    void onConfigSelected();
    void onApplyConfig();
    void onAddConfig();
    void onEditConfig();
    void onDeleteConfig();
    void onRefreshAdapters();
    void onConfigListChanged();

private:
    void setupUi();
    void createMenuBar();
    void loadAdapters();
    void refreshConfigList();
    void refreshConfigList(const QString &adapterGuid);
    void showAddConfigDialog();
    void showEditConfigDialog(int index);
    void applyConfig(const IpConfig &config);
    QString getCurrentAdapterName() const;
    QString getCurrentAdapterGuid() const;
    void applyDarkTheme();

    // UI Components
    QComboBox *m_adapterCombo;
    QTableWidget *m_configTableWidget;
    QPushButton *m_applyButton;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_refreshButton;
    QLabel *m_currentIpLabel;
    QLabel *m_adapterInfoLabel;
    QLabel *m_statusLabel;

    // Managers
    IpConfigManager *m_ipConfigManager;
    NetworkAdapterManager *m_networkManager;

    QVector<NetworkAdapter> m_adapters;
};

#endif // MAINWINDOW_H
