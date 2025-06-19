#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <memory>

/**
 * 现代化的配置管理系统
 * 使用单例模式和类型安全的配置访问
 */
class ConfigManager : public QObject {
    Q_OBJECT

public:
    // 获取单例实例
    static ConfigManager& instance();
    
    // 网络配置
    struct NetworkConfig {
        int timeoutMs = 60000;
        int maxRetries = 3;
        int retryDelayMs = 1000;
        QString userAgent = "MusicBrainzQt/1.0 ( musicbrainzqt@example.com )";
        
        void save(QSettings &settings) const;
        void load(const QSettings &settings);
    };
    
    // API配置
    struct ApiConfig {
        QString baseUrl = "https://musicbrainz.org/ws/2";
        int defaultLimit = 25;
        int rateLimit = 1000;  // ms between requests
        
        void save(QSettings &settings) const;
        void load(const QSettings &settings);
    };
    
    // UI配置  
    struct UiConfig {
        bool showWelcomeTab = true;
        int maxTabs = 20;
        bool rememberColumnSettings = true;
        QString lastEntityType = "Artist";
        
        void save(QSettings &settings) const;
        void load(const QSettings &settings);
    };
    
    // 日志配置
    struct LogConfig {
        bool enableFileLogging = false;
        QString logLevel = "Info";
        QString logFilePath;
        int maxLogFiles = 5;
        int maxLogSizeMB = 10;
        
        void save(QSettings &settings) const;
        void load(const QSettings &settings);
    };
    
    // 访问器方法
    const NetworkConfig& network() const { return m_networkConfig; }
    const ApiConfig& api() const { return m_apiConfig; }
    const UiConfig& ui() const { return m_uiConfig; }
    const LogConfig& log() const { return m_logConfig; }
    
    // 修改器方法
    NetworkConfig& network() { return m_networkConfig; }
    ApiConfig& api() { return m_apiConfig; }
    UiConfig& ui() { return m_uiConfig; }
    LogConfig& log() { return m_logConfig; }
    
    // 保存和加载
    void save();
    void load();
    void reset();  // 重置为默认值
    
    // 便捷方法
    template<typename T>
    T getValue(const QString &key, const T &defaultValue = T{}) const {
        return m_settings->value(key, QVariant::fromValue(defaultValue)).template value<T>();
    }
    
    template<typename T>
    void setValue(const QString &key, const T &value) {
        m_settings->setValue(key, QVariant::fromValue(value));
        emit configChanged(key);
    }

signals:
    void configChanged(const QString &key);
    void networkConfigChanged();
    void apiConfigChanged();
    void uiConfigChanged();
    void logConfigChanged();

private:
    ConfigManager(QObject *parent = nullptr);
    ~ConfigManager() = default;
    
    // 禁用拷贝和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    std::unique_ptr<QSettings> m_settings;
    NetworkConfig m_networkConfig;
    ApiConfig m_apiConfig;
    UiConfig m_uiConfig;
    LogConfig m_logConfig;
};

#endif // CONFIG_MANAGER_H
