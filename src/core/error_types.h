#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H

#include <QString>
#include <QMetaType>

/**
 * @file error_types.h
 * @brief 简化的错误处理系统
 * 
 * 最小化的错误处理基础设施，仅保留实际使用的功能。
 * 
 * @author MusicBrainzQt Team
 * @version 2.0.0
 * @date 2025-07-03
 */

/**
 * @enum ErrorCode
 * @brief 最小化的错误代码枚举
 */
enum class ErrorCode {
    None = 0,
    ApiServerError = 2004,
    Unknown = 9999
};

/**
 * @struct ErrorInfo
 * @brief 简化的错误信息结构体
 */
struct ErrorInfo {
    ErrorCode code = ErrorCode::None;
    QString message;
    QString details;
    
    ErrorInfo() = default;
    ErrorInfo(ErrorCode c, const QString &msg, const QString &det = QString())
        : code(c), message(msg), details(det) {}
    
    bool isValid() const { return code != ErrorCode::None; }
    
    QString toString() const {
        return message + (details.isEmpty() ? "" : " (" + details + ")");
    }
};

// 注册元类型以便在Qt信号槽中使用
Q_DECLARE_METATYPE(ErrorInfo)
Q_DECLARE_METATYPE(ErrorCode)

#endif // ERROR_TYPES_H
