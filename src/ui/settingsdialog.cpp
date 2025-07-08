#include "settingsdialog.h"
#include "../utils/config_manager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QApplication>
#include <QTranslator>
#include <QMessageBox>
#include <QDebug>
#include <QFile>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(400);
    
    setupUI();
    connectSignals();
    loadCurrentSettings();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 语言设置组
    QGroupBox *langGroup = new QGroupBox(tr("Language"));
    QVBoxLayout *langLayout = new QVBoxLayout(langGroup);
    m_languageCombo = new QComboBox();
    setupLanguageOptions();
    langLayout->addWidget(m_languageCombo);
    mainLayout->addWidget(langGroup);
    
    // 功能设置组
    QGroupBox *featGroup = new QGroupBox(tr("Features"));
    QVBoxLayout *featLayout = new QVBoxLayout(featGroup);
    m_enableLoggingCheck = new QCheckBox(tr("Enable Debug Logging"));
    featLayout->addWidget(m_enableLoggingCheck);
    mainLayout->addWidget(featGroup);
    
    // 重启提示
    m_restartLabel = new QLabel(tr("<i>Some settings will take effect after restart</i>"));
    m_restartLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_restartLabel);
    
    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_resetButton = new QPushButton(tr("Reset"));
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_saveButton = new QPushButton(tr("Save"));
    
    btnLayout->addWidget(m_resetButton);
    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelButton);
    btnLayout->addWidget(m_saveButton);
    
    mainLayout->addLayout(btnLayout);
    
    m_saveButton->setDefault(true);
}

void SettingsDialog::connectSignals()
{
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
}

void SettingsDialog::setupLanguageOptions()
{
    m_languageCombo->addItem(tr("System Default"), "system");
    m_languageCombo->addItem(tr("English"), "en");
    m_languageCombo->addItem(tr("Chinese (Simplified)"), "zh_CN");
}

void SettingsDialog::loadCurrentSettings()
{
    ConfigManager &config = ConfigManager::instance();
    
    // 语言设置
    QString language = config.getValue("language", QString("system"));
    int langIndex = m_languageCombo->findData(language);
    if (langIndex >= 0) {
        m_languageCombo->setCurrentIndex(langIndex);
    }
    
    bool enableLogging = config.getValue("enableDebugLogging", false);
    m_enableLoggingCheck->setChecked(enableLogging);
}

void SettingsDialog::saveSettings()
{
    ConfigManager &config = ConfigManager::instance();
    
    // 检查是否有需要重启的设置变更
    bool needsRestart = false;
    
    // 保存语言设置
    QString oldLang = config.getValue("language", QString("system"));
    QString newLang = m_languageCombo->currentData().toString();
    
    if (oldLang != newLang) {
        needsRestart = true;
    }
    
    config.setValue("language", newLang);
    
    // 保存其他设置
    config.setValue("enableDebugLogging", m_enableLoggingCheck->isChecked());
    
    // 应用设置
    applySettings();
    
    // 如果需要重启，提示用户
    if (needsRestart) {
        QMessageBox::information(this, tr("Settings Saved"),
                               tr("Some settings will take effect after restarting the application."));
    }
    
    accept();
}

void SettingsDialog::resetToDefaults()
{
    // 设置默认值
    m_languageCombo->setCurrentIndex(m_languageCombo->findData("system"));
    m_enableLoggingCheck->setChecked(false);
}

void SettingsDialog::applySettings()
{
    // 应用能够即时生效的设置
    
    // 日志设置
    bool enableLogging = m_enableLoggingCheck->isChecked();
    qDebug() << "Debug logging" << (enableLogging ? "enabled" : "disabled");
}
