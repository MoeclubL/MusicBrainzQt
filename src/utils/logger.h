#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>

/**
 * 统一的日志系统
 * 提供不同级别的日志输出和格式化
 */

// 定义日志类别
Q_DECLARE_LOGGING_CATEGORY(logApi)
Q_DECLARE_LOGGING_CATEGORY(logNetwork)
Q_DECLARE_LOGGING_CATEGORY(logUI)
Q_DECLARE_LOGGING_CATEGORY(logService)

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger {
public:
    static void initialize();
    
    // 便捷的日志记录方法
    template<class... Args>
    static void debug(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isDebugEnabled()) {
            qCDebug(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
    template<class... Args>
    static void info(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isInfoEnabled()) {
            qCInfo(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
    template<class... Args>
    static void warning(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isWarningEnabled()) {
            qCWarning(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
    template<class... Args>
    static void error(const QLoggingCategory &category, const QString &format, Args&&... args) {
        if (category.isCriticalEnabled()) {
            qCCritical(category) << formatMessage(format, std::forward<Args>(args)...);
        }
    }
    
private:
    template<class... Args>
    static QString formatMessage(const QString &format, Args&&... args) {
        return QString::asprintf(format.toUtf8().constData(), std::forward<Args>(args)...);
    }
};

// 便捷的日志宏
#define LOG_API_DEBUG(format, ...) Logger::debug(logApi(), format, ##__VA_ARGS__)
#define LOG_API_INFO(format, ...) Logger::info(logApi(), format, ##__VA_ARGS__)
#define LOG_API_WARNING(format, ...) Logger::warning(logApi(), format, ##__VA_ARGS__)
#define LOG_API_ERROR(format, ...) Logger::error(logApi(), format, ##__VA_ARGS__)

#define LOG_NETWORK_DEBUG(format, ...) Logger::debug(logNetwork(), format, ##__VA_ARGS__)
#define LOG_NETWORK_INFO(format, ...) Logger::info(logNetwork(), format, ##__VA_ARGS__)
#define LOG_NETWORK_WARNING(format, ...) Logger::warning(logNetwork(), format, ##__VA_ARGS__)
#define LOG_NETWORK_ERROR(format, ...) Logger::error(logNetwork(), format, ##__VA_ARGS__)

#define LOG_UI_DEBUG(format, ...) Logger::debug(logUI(), format, ##__VA_ARGS__)
#define LOG_UI_INFO(format, ...) Logger::info(logUI(), format, ##__VA_ARGS__)
#define LOG_UI_WARNING(format, ...) Logger::warning(logUI(), format, ##__VA_ARGS__)
#define LOG_UI_ERROR(format, ...) Logger::error(logUI(), format, ##__VA_ARGS__)

#define LOG_SERVICE_DEBUG(format, ...) Logger::debug(logService(), format, ##__VA_ARGS__)
#define LOG_SERVICE_INFO(format, ...) Logger::info(logService(), format, ##__VA_ARGS__)
#define LOG_SERVICE_WARNING(format, ...) Logger::warning(logService(), format, ##__VA_ARGS__)
#define LOG_SERVICE_ERROR(format, ...) Logger::error(logService(), format, ##__VA_ARGS__)

#endif // LOGGER_H
