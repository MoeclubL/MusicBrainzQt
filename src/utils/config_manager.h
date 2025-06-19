#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <memory>

/**
 * @class ConfigManager
 * @brief 现代化的类型安全配置管理系统
 * 
 * ConfigManager提供了一个集中式的应用程序配置管理解决方案，
 * 采用单例模式确保全局一致性，并通过结构化的配置对象提供
 * 类型安全的访问接口。
 * 
 * **核心特性：**
 * - **单例模式**: 全局唯一实例，确保配置一致性
 * - **类型安全**: 结构化配置对象，避免字符串键错误
 * - **自动持久化**: 配置变更自动保存到系统配置存储
 * - **变更通知**: 信号机制通知配置变更
 * - **模板接口**: 泛型方法支持任意类型的配置值
 * - **默认值**: 所有配置项都有合理的默认值
 * 
 * **配置分类：**
 * - NetworkConfig: 网络请求相关配置
 * - ApiConfig: MusicBrainz API相关配置
 * - UiConfig: 用户界面和交互配置
 * - LogConfig: 日志系统配置
 * 
 * **使用示例：**
 * ```cpp
 * // 获取配置
 * auto& config = ConfigManager::instance();
 * int timeout = config.network().timeoutMs;
 * QString apiUrl = config.api().baseUrl;
 * 
 * // 修改配置
 * config.network().timeoutMs = 30000;
 * config.save();  // 自动保存
 * 
 * // 使用模板方法
 * QString customValue = config.getValue<QString>("custom.setting", "default");
 * config.setValue("custom.setting", QString("new value"));
 * ```
 * 
 * **配置文件位置：**
 * - Windows: HKEY_CURRENT_USER\Software\MusicBrainzQt\MusicBrainz Qt Client
 * - macOS: ~/Library/Preferences/com.MusicBrainzQt.MusicBrainz Qt Client.plist
 * - Linux: ~/.config/MusicBrainzQt/MusicBrainz Qt Client.conf
 * 
 * @note 配置变更会自动触发相应的信号，UI组件可以监听这些信号来更新显示
 * 
 * @author MusicBrainzQt Team
 * @see QSettings
 */
class ConfigManager : public QObject {    Q_OBJECT

public:
    /**
     * @brief 获取配置管理器的单例实例
     * @return ConfigManager的唯一实例引用
     * 
     * 线程安全的单例实现，首次调用时创建实例。
     */
    static ConfigManager& instance();
    
    // =============================================================================
    // 配置结构定义
    // =============================================================================
    
    /**
     * @struct NetworkConfig
     * @brief 网络请求相关配置
     * 
     * 包含HTTP请求超时、重试机制、用户代理等网络层配置。
     */
    struct NetworkConfig {
        int timeoutMs = 60000;          ///< 请求超时时间（毫秒）
        int maxRetries = 3;             ///< 最大重试次数
        int retryDelayMs = 1000;        ///< 重试延迟时间（毫秒）
        QString userAgent = "MusicBrainzQt/1.0 ( https://github.com/MoeclubL/MusicBrainzQt )";  ///< HTTP用户代理字符串
        
        /**
         * @brief 保存网络配置到QSettings
         * @param settings QSettings实例
         */
        void save(QSettings &settings) const;
        
        /**
         * @brief 从QSettings加载网络配置
         * @param settings QSettings实例
         */
        void load(const QSettings &settings);
    };
    
    /**
     * @struct ApiConfig
     * @brief MusicBrainz API相关配置
     * 
     * 包含API基础URL、默认查询限制、速率限制等配置。
     */
    struct ApiConfig {
        QString baseUrl = "https://musicbrainz.org/ws/2";   ///< API基础URL
        int defaultLimit = 25;                              ///< 默认查询结果数量限制
        int rateLimit = 1000;                               ///< 请求速率限制（毫秒间隔）
        
        /**
         * @brief 保存API配置到QSettings
         * @param settings QSettings实例
         */
        void save(QSettings &settings) const;
        
        /**
         * @brief 从QSettings加载API配置
         * @param settings QSettings实例
         */
        void load(const QSettings &settings);
    };
    
    /**
     * @struct UiConfig
     * @brief 用户界面相关配置
     * 
     * 包含界面显示选项、标签页管理、用户偏好等设置。
     */
    struct UiConfig {
        bool showWelcomeTab = true;             ///< 是否显示欢迎标签页
        int maxTabs = 20;                       ///< 最大标签页数量
        bool rememberColumnSettings = true;     ///< 是否记住列设置
        QString lastEntityType = "Artist";     ///< 上次选择的实体类型
        
        /**
         * @brief 保存UI配置到QSettings
         * @param settings QSettings实例
         */
        void save(QSettings &settings) const;
        
        /**
         * @brief 从QSettings加载UI配置
         * @param settings QSettings实例
         */
        void load(const QSettings &settings);
    };    
    /**
     * @struct LogConfig
     * @brief 日志系统相关配置
     * 
     * 包含日志级别、文件输出、日志轮转等配置。
     */
    struct LogConfig {
        bool enableFileLogging = false;     ///< 是否启用文件日志
        QString logLevel = "Info";          ///< 日志级别
        QString logFilePath;                ///< 日志文件路径（空则使用默认）
        int maxLogFiles = 5;                ///< 最大日志文件数量
        int maxLogSizeMB = 10;              ///< 单个日志文件最大大小（MB）
        
        /**
         * @brief 保存日志配置到QSettings
         * @param settings QSettings实例
         */
        void save(QSettings &settings) const;
        
        /**
         * @brief 从QSettings加载日志配置
         * @param settings QSettings实例
         */
        void load(const QSettings &settings);
    };
    
    // =============================================================================
    // 配置访问接口
    // =============================================================================
    
    /**
     * @brief 获取网络配置（只读）
     * @return 网络配置的常量引用
     */
    const NetworkConfig& network() const { return m_networkConfig; }
    
    /**
     * @brief 获取API配置（只读）
     * @return API配置的常量引用
     */
    const ApiConfig& api() const { return m_apiConfig; }
    
    /**
     * @brief 获取UI配置（只读）
     * @return UI配置的常量引用
     */
    const UiConfig& ui() const { return m_uiConfig; }
    
    /**
     * @brief 获取日志配置（只读）
     * @return 日志配置的常量引用
     */
    const LogConfig& log() const { return m_logConfig; }
    
    // =============================================================================
    // 配置修改接口
    // =============================================================================
    
    /**
     * @brief 获取网络配置（可修改）
     * @return 网络配置的引用
     */
    NetworkConfig& network() { return m_networkConfig; }
    
    /**
     * @brief 获取API配置（可修改）
     * @return API配置的引用
     */
    ApiConfig& api() { return m_apiConfig; }
    
    /**
     * @brief 获取UI配置（可修改）
     * @return UI配置的引用
     */
    UiConfig& ui() { return m_uiConfig; }
    
    /**
     * @brief 获取日志配置（可修改）
     * @return 日志配置的引用
     */
    LogConfig& log() { return m_logConfig; }
    
    // =============================================================================
    // 配置管理方法
    // =============================================================================
    
    /**
     * @brief 保存所有配置到持久化存储
     * 
     * 将当前内存中的配置保存到系统配置存储中。
     */
    void save();
    
    /**
     * @brief 从持久化存储加载配置
     * 
     * 从系统配置存储中加载配置，覆盖当前内存中的值。
     */
    void load();
    
    /**
     * @brief 重置所有配置为默认值
     * 
     * 将所有配置项重置为默认值，不会自动保存。
     */
    void reset();
    
    // =============================================================================
    // 泛型配置访问方法
    // =============================================================================
    
    /**
     * @brief 获取任意类型的配置值
     * @tparam T 配置值的类型
     * @param key 配置键名
     * @param defaultValue 默认值
     * @return 配置值
     * 
     * 提供类型安全的配置值访问，如果键不存在则返回默认值。
     */
    template<typename T>
    T getValue(const QString &key, const T &defaultValue = T{}) const {
        return m_settings->value(key, QVariant::fromValue(defaultValue)).template value<T>();
    }
    
    /**
     * @brief 设置任意类型的配置值
     * @tparam T 配置值的类型
     * @param key 配置键名
     * @param value 配置值
     * 
     * 设置配置值并触发变更信号。设置后需要调用save()才能持久化。
     */
    template<typename T>
    void setValue(const QString &key, const T &value) {
        m_settings->setValue(key, QVariant::fromValue(value));
        emit configChanged(key);
    }

signals:
    /**
     * @brief 配置变更信号
     * @param key 变更的配置键名
     * 
     * 当任意配置项发生变更时发出。
     */
    void configChanged(const QString &key);
    
    /**
     * @brief 网络配置变更信号
     * 
     * 当网络相关配置发生变更时发出。
     */
    void networkConfigChanged();
    
    /**
     * @brief API配置变更信号
     * 
     * 当API相关配置发生变更时发出。
     */
    void apiConfigChanged();
    
    /**
     * @brief UI配置变更信号
     * 
     * 当UI相关配置发生变更时发出。
     */
    void uiConfigChanged();
    
    /**
     * @brief 日志配置变更信号
     * 
     * 当日志相关配置发生变更时发出。
     */
    void logConfigChanged();

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    ConfigManager(QObject *parent = nullptr);
    
    /**
     * @brief 私有析构函数（单例模式）
     */
    ~ConfigManager() = default;
    
    // 禁用拷贝和赋值操作符
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // =============================================================================
    // 成员变量
    // =============================================================================
    
    std::unique_ptr<QSettings> m_settings;      ///< Qt配置存储对象
    NetworkConfig m_networkConfig;              ///< 网络配置实例
    ApiConfig m_apiConfig;                      ///< API配置实例
    UiConfig m_uiConfig;                        ///< UI配置实例
    LogConfig m_logConfig;                      ///< 日志配置实例
};

#endif // CONFIG_MANAGER_H
