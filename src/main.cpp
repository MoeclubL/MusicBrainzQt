#include <QApplication>
#include "mainwindow.h"
#include "utils/logger.h"
#include "utils/config_manager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("MusicBrainz Qt Client");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MusicBrainzQt");
    
    // 初始化日志系统
    Logger::initialize();
    
    // 加载配置
    ConfigManager::instance().load();
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}