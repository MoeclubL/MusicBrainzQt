/**
 * @file main.cpp
 * @brief MusicBrainzQt应用程序入口点
 * 
 * 此文件包含应用程序的主函数，负责初始化Qt应用程序环境、
 * 配置应用程序属性、初始化核心系统组件并启动主窗口。
 * 
 * **初始化序列：**
 * 1. 创建QApplication实例
 * 2. 设置应用程序元信息（名称、版本、组织）
 * 3. 初始化日志系统
 * 4. 加载应用程序配置
 * 5. 创建并显示主窗口
 * 6. 启动Qt事件循环
 * 
 * **依赖组件：**
 * - MainWindow: 主窗口界面
 * - Qt标准日志: 使用qDebug()/qWarning()/qCritical()
 * - ConfigManager: 配置管理器
 * 
 * @author MusicBrainzQt Team
 */

#include <QApplication>
#include <QFile>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include "mainwindow.h"
#include "utils/config_manager.h"

/**
 * @brief 应用程序主入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 应用程序退出代码
 * 
 * 按照Qt应用程序的标准模式，创建应用程序实例、初始化必要的
 * 系统组件、显示主界面并启动事件循环。
 * 
 * **错误处理：**
 * 如果初始化过程中出现错误，应用程序将记录错误并尝试优雅退出。
 * 
 * **返回值：**
 * - 0: 正常退出
 * - 非0: 异常退出（具体错误码由Qt框架确定）
 */
int main(int argc, char *argv[]) {
    // =============================================================================
    // Qt应用程序初始化
    // =============================================================================
    
    QApplication app(argc, argv);
    
    // =============================================================================
    // 国际化设置
    // =============================================================================
    
    // 创建翻译器
    QTranslator translator;
    
    // 获取系统语言
    QString locale = QLocale::system().name();
    
    // 尝试加载对应的翻译文件
    QString translationFile = QString("musicbrainzqt_%1").arg(locale);
    
    // 在应用程序目录和翻译子目录中查找翻译文件
    QStringList translationPaths;
    translationPaths << QDir::currentPath() + "/translations"
                     << QCoreApplication::applicationDirPath() + "/translations"
                     << ":/translations";
    
    bool translationLoaded = false;
    for (const QString &path : translationPaths) {
        if (translator.load(translationFile, path)) {
            app.installTranslator(&translator);
            translationLoaded = true;
            break;
        }
    }
    
    // 如果系统语言的翻译文件未找到，尝试加载中文翻译作为备选
    if (!translationLoaded && !locale.startsWith("en")) {
        for (const QString &path : translationPaths) {
            if (translator.load("musicbrainzqt_zh_CN", path)) {
                app.installTranslator(&translator);
                translationLoaded = true;
                break;
            }
        }
    }
    
    // 记录翻译状态
    if (translationLoaded) {
        qDebug() << "Translation loaded for locale:" << locale;
    } else {
        qDebug() << "No translation found for locale:" << locale << ", using default English";
    }
    
    // =============================================================================
    // 样式表加载
    // =============================================================================
    
    // 加载应用程序样式表
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QString::fromLatin1(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }
    
    // =============================================================================
    // 应用程序元信息设置
    // =============================================================================
    
    // 设置应用程序基本信息，用于配置文件存储路径、
    // 关于对话框显示和系统集成
    app.setApplicationName("MusicBrainz Qt Client");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MusicBrainzQt");
    
    // =============================================================================
    // 核心系统初始化
    // =============================================================================
    
    // 设置Qt日志格式
    qSetMessagePattern("[%{time yyyyMMdd h:mm:ss.zzz}] [%{category}] [%{type}] %{message}");
    
    // 加载应用程序配置
    // 包括用户偏好设置、API配置、窗口状态等
    ConfigManager::instance().load();
    
    // =============================================================================
    // 主界面创建和显示
    // =============================================================================
    
    // 创建主窗口实例
    MainWindow mainWindow;
    
    // 显示主窗口
    mainWindow.show();
    
    // =============================================================================
    // 事件循环启动
    // =============================================================================
    
    // 启动Qt事件循环，处理用户交互、网络请求、定时器等事件
    // 应用程序将在此处运行直到用户退出
    return app.exec();
}