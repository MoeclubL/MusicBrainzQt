#ifndef ITEMDETAILTAB_H
#define ITEMDETAILTAB_H

#include <QWidget>
#include <QSharedPointer>
#include "../core/types.h"

QT_BEGIN_NAMESPACE
class QTabWidget;
class QScrollArea;
class QLabel;
class QTextEdit;
class QVBoxLayout;
QT_END_NAMESPACE

class ResultItem;
class EntityListWidget;

namespace Ui {
class ItemDetailTab;
}

/**
 * 项目详细信息标签页组件
 * 显示某个项目的详细信息，包含多个子标签页
 */
class ItemDetailTab : public QWidget
{
    Q_OBJECT

public:
    explicit ItemDetailTab(const QSharedPointer<ResultItem> &item, QWidget *parent = nullptr);
    ~ItemDetailTab();

    // 获取项目信息
    QSharedPointer<ResultItem> getItem() const { return m_item; }
    QString getItemId() const;
    QString getItemName() const;

signals:
    void loadSubItems(const QString &itemId, EntityType parentType, const QString &subType);
    void openDetailTab(QSharedPointer<ResultItem> item);
    
    // 右键菜单信号
    void openInBrowser(const QString &entityId, EntityType entityType);
    void copyId(const QString &entityId);

public slots:
    void refreshItemInfo();

private slots:
    void onSubTabChanged(int index);
    void onSubItemDoubleClicked(QSharedPointer<ResultItem> item);
    void onOpenInBrowser(const QString &itemId, EntityType type);
    void onCopyId(const QString &itemId);

private:
    void setupUI();    void setupSubTabs();
    void populateItemInfo();
    void populateSubTabs();
    void createSubTab(const QString &title, const QString &key, EntityType entityType);
    void createOverviewTab();
    void createAliasesTab();
    void createTagsTab();
    void createRelationshipsTab();
    QString buildMusicBrainzUrl(const QString &itemId, EntityType type) const;
    
    // 结构化信息显示辅助方法
    void clearInfoLayout(QVBoxLayout *layout);
    void populateInfoSection(QVBoxLayout *layout, const QMap<QString, QVariant> &data);
    void populateDetailsSection(QVBoxLayout *layout, const QVariantMap &data);
    void createInfoItem(QVBoxLayout *layout, const QString &label, const QString &value);
    QString formatComplexValue(const QVariantMap &map);
    
    Ui::ItemDetailTab *ui;
    QSharedPointer<ResultItem> m_item;
    
    // 子标签页组件映射
    QHash<QString, EntityListWidget*> m_subTabWidgets;
    
    // 子标签页键名映射
    QStringList m_subTabKeys;
};

#endif // ITEMDETAILTAB_H
