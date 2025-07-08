#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

/**
 * @class SettingsDialog
 * @brief 应用程序设置对话框
 * 
 * 提供用户配置应用程序各项设置的界面，包括语言选择、UI选项等。
 * 设置将自动保存到应用程序配置中。
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 加载当前配置
     * 根据当前应用程序配置更新界面控件的状态
     */
    void loadCurrentSettings();
    
private slots:
    /**
     * @brief 保存设置
     * 将当前界面中的设置保存到配置文件
     */
    void saveSettings();
    
    /**
     * @brief 重置设置到默认值
     */
    void resetToDefaults();
    
    /**
     * @brief 应用当前设置
     */
    void applySettings();
    
private:
    /**
     * @brief 初始化界面
     */
    void setupUI();
    
    /**
     * @brief 连接信号槽
     */
    void connectSignals();
    
    /**
     * @brief 添加语言选项
     */
    void setupLanguageOptions();
    
    // UI 控件
    QComboBox *m_languageCombo;
    
    QCheckBox *m_enableLoggingCheck;
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    QPushButton *m_resetButton;
    QLabel *m_restartLabel;
};

#endif // SETTINGSDIALOG_H
