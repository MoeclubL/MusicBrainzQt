#include "logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QLoggingCategory>

// 定义日志类别
Q_LOGGING_CATEGORY(logApi, "musicbrainz.api")
Q_LOGGING_CATEGORY(logNetwork, "musicbrainz.network")
Q_LOGGING_CATEGORY(logUI, "musicbrainz.ui")
Q_LOGGING_CATEGORY(logService, "musicbrainz.service")

void Logger::initialize() {
    // 设置日志输出格式
    qSetMessagePattern("[%{time yyyyMMdd h:mm:ss.zzz}] [%{category}] [%{type}] %{message}");
    
    // 可以通过环境变量或配置文件控制日志级别
    // 示例：QLoggingCategory::setFilterRules("musicbrainz.network.debug=false");
    
#ifdef QT_DEBUG
    // Debug模式下启用所有日志
    QLoggingCategory::setFilterRules("musicbrainz.*.debug=true");
#else
    // Release模式下只启用Info及以上级别
    QLoggingCategory::setFilterRules("musicbrainz.*.debug=false\n"
                                    "musicbrainz.*.info=true");
#endif
}
