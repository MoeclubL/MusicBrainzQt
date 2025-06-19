#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>

/**
 * @file logger.h
 * @brief 统一的分类日志系统
 * 
 * 提供了基于Qt日志框架的分层日志记录系统，支持多个日志类别、
 * 不同的日志级别和灵活的消息格式化。
 * 
 * **核心特性：**
 * - **分类日志**: 按功能模块分类（API、网络、UI、服务）
 * - **级别控制**: Debug、Info、Warning、Error、Critical五个级别
 * - **模板化API**: 类型安全的参数传递和格式化
 * - **便捷宏**: 简化常用日志调用的宏定义
 * - **运行时控制**: 可通过Qt日志规则动态控制输出
 * 
 * **使用示例：**
 * ```cpp
 * // 使用模板方法
 * Logger::info(logApi(), "Search completed: %d results found", count);
 * Logger::error(logNetwork(), "HTTP request failed: %s", error.toUtf8().data());
 * 
 * // 使用便捷宏
 * LOG_API_INFO("Starting search for query: %s", query.toUtf8().data());
 * LOG_SERVICE_DEBUG("Processing %d items in batch", items.size());
 * ```
 * 
 * **日志类别：**
 * - logApi: MusicBrainz API相关操作
 * - logNetwork: 网络请求和响应
 * - logUI: 用户界面操作和事件
 * - logService: 业务服务层操作
 * 
 * **配置控制：**
 * 可通过QT_LOGGING_RULES环境变量或配置文件控制日志输出：
 * ```
 * QT_LOGGING_RULES="musicbrainz.api.debug=true;musicbrainz.network.info=false"
 * ```
 */

// =============================================================================
// 日志类别声明
// =============================================================================

/**
 * @brief API操作日志类别
 * 用于记录MusicBrainz API的调用、响应和错误
 */
Q_DECLARE_LOGGING_CATEGORY(logApi)

/**
 * @brief 网络操作日志类别  
 * 用于记录HTTP请求、响应和网络错误
 */
Q_DECLARE_LOGGING_CATEGORY(logNetwork)

/**
 * @brief UI操作日志类别
 * 用于记录用户界面操作、事件和状态变化
 */
Q_DECLARE_LOGGING_CATEGORY(logUI)

/**
 * @brief 服务层日志类别
 * 用于记录业务逻辑、数据处理和服务间调用
 */
Q_DECLARE_LOGGING_CATEGORY(logService)

/**
 * @enum LogLevel
 * @brief 日志级别枚举
 * 
 * 定义了可用的日志级别，按严重程度递增排序。
 */
enum class LogLevel {
    Debug,      ///< 调试信息，详细的程序执行流程
    Info,       ///< 一般信息，重要的程序状态变化
    Warning,    ///< 警告信息，可能的问题但不影响继续执行
    Error,      ///< 错误信息，操作失败但程序可以继续
    Critical    ///< 严重错误，可能导致程序终止
};

/**
 * @class Logger
 * @brief 统一日志记录器
 * 
 * 提供静态方法接口的日志记录器，支持模板化的类型安全参数传递。
 * 基于Qt的QLoggingCategory系统，支持运行时的日志控制。
 */
class Logger {
public:
    /**
     * @brief 初始化日志系统
     * 
     * 设置日志格式、输出目标和默认规则。
     * 应在应用程序启动时调用。
     */
    static void initialize();
    
    // =============================================================================
    // 模板化日志记录方法
    // =============================================================================
    
    /**
     * @brief 记录调试级别日志
     * @tparam Args 可变参数类型
     * @param category 日志类别
     * @param format 格式字符串（类似printf格式）
     * @param args 格式化参数
     * 
     * 记录详细的调试信息，仅在Debug模式或显式启用时输出。
     */
    template<class... Args>
    static void debug(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isDebugEnabled()) {
            qCDebug(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
    /**
     * @brief 记录信息级别日志
     * @tparam Args 可变参数类型  
     * @param category 日志类别
     * @param format 格式字符串
     * @param args 格式化参数
     * 
     * 记录重要的程序状态信息和操作结果。
     */
    template<class... Args>
    static void info(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isInfoEnabled()) {
            qCInfo(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
    /**
     * @brief 记录警告级别日志
     * @tparam Args 可变参数类型
     * @param category 日志类别  
     * @param format 格式字符串
     * @param args 格式化参数
     * 
     * 记录可能的问题和异常情况，但不影响程序继续执行。
     */
    template<class... Args>
    static void warning(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isWarningEnabled()) {
            qCWarning(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }    
    /**
     * @brief 记录错误级别日志
     * @tparam Args 可变参数类型
     * @param category 日志类别
     * @param format 格式字符串
     * @param args 格式化参数
     * 
     * 记录操作失败和错误情况，需要注意但程序可以继续运行。
     */
    template<class... Args>
    static void error(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isCriticalEnabled()) {
            qCCritical(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
private:
    /**
     * @brief 格式化消息字符串
     * @tparam Args 可变参数类型
     * @param format 格式字符串
     * @param args 格式化参数
     * @return 格式化后的字符串
     * 
     * 内部使用的消息格式化函数，支持printf风格的格式化。
     */
    template<class... Args>
    static QString formatMessage(const QString &format, Args&&... args) {
        return QString::asprintf(format.toUtf8().constData(), std::forward<Args>(args)...);
    }
};

// =============================================================================
// 便捷日志宏定义
// =============================================================================

/**
 * @name API日志宏
 * @brief MusicBrainz API操作的便捷日志宏
 * @{
 */
#define LOG_API_DEBUG(format, ...) Logger::debug(logApi(), format, ##__VA_ARGS__)
#define LOG_API_INFO(format, ...) Logger::info(logApi(), format, ##__VA_ARGS__)
#define LOG_API_WARNING(format, ...) Logger::warning(logApi(), format, ##__VA_ARGS__)
#define LOG_API_ERROR(format, ...) Logger::error(logApi(), format, ##__VA_ARGS__)
/** @} */

/**
 * @name 网络日志宏
 * @brief 网络操作的便捷日志宏
 * @{
 */
#define LOG_NETWORK_DEBUG(format, ...) Logger::debug(logNetwork(), format, ##__VA_ARGS__)
#define LOG_NETWORK_INFO(format, ...) Logger::info(logNetwork(), format, ##__VA_ARGS__)
#define LOG_NETWORK_WARNING(format, ...) Logger::warning(logNetwork(), format, ##__VA_ARGS__)
#define LOG_NETWORK_ERROR(format, ...) Logger::error(logNetwork(), format, ##__VA_ARGS__)
/** @} */

/**
 * @name UI日志宏
 * @brief 用户界面操作的便捷日志宏
 * @{
 */
#define LOG_UI_DEBUG(format, ...) Logger::debug(logUI(), format, ##__VA_ARGS__)
#define LOG_UI_INFO(format, ...) Logger::info(logUI(), format, ##__VA_ARGS__)
#define LOG_UI_WARNING(format, ...) Logger::warning(logUI(), format, ##__VA_ARGS__)
#define LOG_UI_ERROR(format, ...) Logger::error(logUI(), format, ##__VA_ARGS__)
/** @} */

/**
 * @name 服务日志宏
 * @brief 业务服务层的便捷日志宏
 * @{
 */
#define LOG_SERVICE_DEBUG(format, ...) Logger::debug(logService(), format, ##__VA_ARGS__)
#define LOG_SERVICE_INFO(format, ...) Logger::info(logService(), format, ##__VA_ARGS__)
#define LOG_SERVICE_WARNING(format, ...) Logger::warning(logService(), format, ##__VA_ARGS__)
#define LOG_SERVICE_ERROR(format, ...) Logger::error(logService(), format, ##__VA_ARGS__)
/** @} */

#endif // LOGGER_H
