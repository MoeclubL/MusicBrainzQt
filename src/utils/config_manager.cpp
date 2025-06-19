#include "config_manager.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    // 设置配置文件路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    m_settings = std::make_unique<QSettings>(
        configPath + "/config.ini", 
        QSettings::IniFormat
    );
    
    // 加载配置
    load();
}

void ConfigManager::save() {
    m_settings->beginGroup("Network");
    m_networkConfig.save(*m_settings);
    m_settings->endGroup();
    
    m_settings->beginGroup("Api");
    m_apiConfig.save(*m_settings);
    m_settings->endGroup();
    
    m_settings->beginGroup("UI");
    m_uiConfig.save(*m_settings);
    m_settings->endGroup();
    
    m_settings->beginGroup("Log");
    m_logConfig.save(*m_settings);
    m_settings->endGroup();
    
    m_settings->sync();
}

void ConfigManager::load() {
    m_settings->beginGroup("Network");
    m_networkConfig.load(*m_settings);
    m_settings->endGroup();
    
    m_settings->beginGroup("Api");
    m_apiConfig.load(*m_settings);
    m_settings->endGroup();
    
    m_settings->beginGroup("UI");
    m_uiConfig.load(*m_settings);
    m_settings->endGroup();
    
    m_settings->beginGroup("Log");
    m_logConfig.load(*m_settings);
    m_settings->endGroup();
}

void ConfigManager::reset() {
    m_networkConfig = NetworkConfig();
    m_apiConfig = ApiConfig();
    m_uiConfig = UiConfig();
    m_logConfig = LogConfig();
    
    save();
    
    emit networkConfigChanged();
    emit apiConfigChanged();
    emit uiConfigChanged();
    emit logConfigChanged();
}

// NetworkConfig 实现
void ConfigManager::NetworkConfig::save(QSettings &settings) const {
    settings.setValue("timeoutMs", timeoutMs);
    settings.setValue("maxRetries", maxRetries);
    settings.setValue("retryDelayMs", retryDelayMs);
    settings.setValue("userAgent", userAgent);
}

void ConfigManager::NetworkConfig::load(const QSettings &settings) {
    timeoutMs = settings.value("timeoutMs", 60000).toInt();
    maxRetries = settings.value("maxRetries", 3).toInt();
    retryDelayMs = settings.value("retryDelayMs", 1000).toInt();
    userAgent = settings.value("userAgent", "MusicBrainzQt/1.0 ( https://github.com/MoeclubL/MusicBrainzQt )").toString();
}

// ApiConfig 实现
void ConfigManager::ApiConfig::save(QSettings &settings) const {
    settings.setValue("baseUrl", baseUrl);
    settings.setValue("defaultLimit", defaultLimit);
    settings.setValue("rateLimit", rateLimit);
}

void ConfigManager::ApiConfig::load(const QSettings &settings) {
    baseUrl = settings.value("baseUrl", "https://musicbrainz.org/ws/2").toString();
    defaultLimit = settings.value("defaultLimit", 25).toInt();
    rateLimit = settings.value("rateLimit", 1000).toInt();
}

// UiConfig 实现
void ConfigManager::UiConfig::save(QSettings &settings) const {
    settings.setValue("showWelcomeTab", showWelcomeTab);
    settings.setValue("maxTabs", maxTabs);
    settings.setValue("rememberColumnSettings", rememberColumnSettings);
    settings.setValue("lastEntityType", lastEntityType);
}

void ConfigManager::UiConfig::load(const QSettings &settings) {
    showWelcomeTab = settings.value("showWelcomeTab", true).toBool();
    maxTabs = settings.value("maxTabs", 20).toInt();
    rememberColumnSettings = settings.value("rememberColumnSettings", true).toBool();
    lastEntityType = settings.value("lastEntityType", "Artist").toString();
}

// LogConfig 实现
void ConfigManager::LogConfig::save(QSettings &settings) const {
    settings.setValue("enableFileLogging", enableFileLogging);
    settings.setValue("logLevel", logLevel);
    settings.setValue("logFilePath", logFilePath);
    settings.setValue("maxLogFiles", maxLogFiles);
    settings.setValue("maxLogSizeMB", maxLogSizeMB);
}

void ConfigManager::LogConfig::load(const QSettings &settings) {
    enableFileLogging = settings.value("enableFileLogging", false).toBool();
    logLevel = settings.value("logLevel", "Info").toString();
    logFilePath = settings.value("logFilePath", QString()).toString();
    maxLogFiles = settings.value("maxLogFiles", 5).toInt();
    maxLogSizeMB = settings.value("maxLogSizeMB", 10).toInt();
}
