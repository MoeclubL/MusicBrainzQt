#ifndef ADVANCEDSEARCHWIDGET_H
#define ADVANCEDSEARCHWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QScrollArea>
#include "../core/types.h"

QT_BEGIN_NAMESPACE
class Ui_AdvancedSearchWidget;
QT_END_NAMESPACE

/**
 * 简化的高级搜索组件
 */
class AdvancedSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedSearchWidget(QWidget *parent = nullptr);
    ~AdvancedSearchWidget();

    // 获取搜索参数
    SearchParameters getSearchParameters() const;
    
    // 设置搜索类型
    void setEntityType(EntityType type);
    
    // 重置表单
    

signals:
    void searchRequested(const SearchParameters &params);

private slots:
    void onSearchClicked();
    
    void onTypeChanged();

private:
    void setupUI();
    void createFieldsForType(EntityType type);
    void addField(const QString &key, const QString &label, const QString &placeholder, int &row);
      // UI对象
    Ui_AdvancedSearchWidget *ui;
    
    // 当前实体类型
    EntityType m_currentType;
    
    // 高级搜索字段
    QWidget *m_fieldsWidget;
    QGridLayout *m_fieldsLayout;
    
    // 字段存储
    QMap<QString, QWidget*> m_fields;
};

#endif // ADVANCEDSEARCHWIDGET_H
