#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H

#include <QString>
#include <QMetaType>
#include <optional>

/**
 * 现代化的错误处理系统
 * 使用结构化的错误类型和 std::optional
 */

enum class ErrorCode {
    None = 0,
    
    // 网络相关错误
    NetworkTimeout = 1000,
    NetworkConnectionError,
    NetworkHostNotFound,
    NetworkServiceUnavailable,
    
    // API相关错误
    ApiInvalidResponse = 2000,
    ApiRateLimitExceeded,
    ApiInvalidQuery,
    ApiNotFound,
    ApiServerError,
    
    // 数据相关错误
    DataParsingError = 3000,
    DataValidationError,
    DataNotFound,
    
    // 应用相关错误
    AppConfigurationError = 4000,
    AppInternalError,
    
    // 未知错误
    Unknown = 9999
};

struct ErrorInfo {
    ErrorCode code = ErrorCode::None;
    QString message;
    QString details;
    int httpStatusCode = 0;  // HTTP状态码(如果适用)
    
    ErrorInfo() = default;
    ErrorInfo(ErrorCode c, const QString &msg, const QString &det = QString())
        : code(c), message(msg), details(det) {}
    
    bool isValid() const { return code != ErrorCode::None; }
    
    QString toString() const {
        QString result = QString("Error %1: %2").arg(static_cast<int>(code)).arg(message);
        if (!details.isEmpty()) {
            result += QString(" (%1)").arg(details);
        }
        return result;
    }
};

// 便捷的结果类型，使用 std::optional 模式
template<typename T>
class Result {
public:
    Result() : m_hasValue(false) {}
    Result(const T &value) : m_value(value), m_hasValue(true) {}
    Result(const ErrorInfo &error) : m_error(error), m_hasValue(false) {}
    
    bool isSuccess() const { return m_hasValue && !m_error.isValid(); }
    bool isError() const { return !m_hasValue || m_error.isValid(); }
    
    const T& value() const { 
        Q_ASSERT(isSuccess()); 
        return m_value; 
    }
    
    T& value() { 
        Q_ASSERT(isSuccess()); 
        return m_value; 
    }
    
    const ErrorInfo& error() const { return m_error; }
    
    // 类似 std::optional 的接口
    bool has_value() const { return isSuccess(); }
    const T& operator*() const { return value(); }
    T& operator*() { return value(); }
    const T* operator->() const { return &value(); }
    T* operator->() { return &value(); }
    
    // 便捷的创建方法
    static Result<T> success(const T &value) { return Result<T>(value); }
    static Result<T> failure(const ErrorInfo &error) { return Result<T>(error); }
    static Result<T> failure(ErrorCode code, const QString &message, const QString &details = QString()) {
        return Result<T>(ErrorInfo(code, message, details));
    }
    
private:
    T m_value = T{};
    ErrorInfo m_error;
    bool m_hasValue = false;
};

// 为void类型特化
template<>
class Result<void> {
public:
    Result() = default;
    Result(const ErrorInfo &error) : m_error(error) {}
    
    bool isSuccess() const { return !m_error.isValid(); }
    bool isError() const { return m_error.isValid(); }
    
    const ErrorInfo& error() const { return m_error; }
    
    static Result<void> success() { return Result<void>(); }
    static Result<void> failure(const ErrorInfo &error) { return Result<void>(error); }
    static Result<void> failure(ErrorCode code, const QString &message, const QString &details = QString()) {
        return Result<void>(ErrorInfo(code, message, details));
    }
    
private:
    ErrorInfo m_error;
};

// 注册元类型以便在Qt信号槽中使用
Q_DECLARE_METATYPE(ErrorInfo)
Q_DECLARE_METATYPE(ErrorCode)

#endif // ERROR_TYPES_H
