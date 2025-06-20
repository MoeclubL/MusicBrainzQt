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
 * @brief 实体详细信息标签页组件
 * 
 * ItemDetailTab是显示MusicBrainz实体详细信息的核心UI组件。
 * 它为不同类型的实体（艺术家、专辑、录音等）提供统一的详情界面。
 * 
 * 主要功能：
 * - 显示实体的基本信息（名称、类型、消歧等）
 * - 根据实体类型动态创建相应的标签页
 * - 支持Overview、Aliases、Tags、Relationships等标准标签页
 * - 显示相关实体列表（如艺术家的专辑、专辑的录音等）
 * - 提供右键菜单操作（在浏览器中打开、复制ID等）
 * 
 * 设计模式：
 * - 策略模式：根据实体类型选择不同的标签页创建策略
 * - 观察者模式：通过信号槽监听用户交互
 * - 组合模式：组合多个EntityListWidget显示相关实体
 * 
 * @see ResultItem 实体数据容器
 * @see EntityListWidget 实体列表显示组件
 */
class ItemDetailTab : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param item 要显示详细信息的实体项目
     * @param parent 父窗口组件
     */
    explicit ItemDetailTab(const QSharedPointer<ResultItem> &item, QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~ItemDetailTab();

    // =============================================================================
    // 访问器方法
    // =============================================================================
    
    /**
     * @brief 获取当前显示的实体项目
     * @return 实体项目的共享指针
     */
    QSharedPointer<ResultItem> getItem() const { return m_item; }
    
    /**
     * @brief 获取实体ID
     * @return 实体的MusicBrainz ID
     */
    QString getItemId() const;
    
    /**
     * @brief 获取实体名称
     * @return 实体的显示名称
     */
    QString getItemName() const;

signals:
    // =============================================================================
    // 信号定义
    // 这些信号用于与其他组件通信，实现松耦合的架构
    // =============================================================================
    
    /**
     * @brief 请求加载子项目信号
     * @param itemId 父项目ID
     * @param parentType 父项目类型
     * @param subType 子项目类型标识
     */
    void loadSubItems(const QString &itemId, EntityType parentType, const QString &subType);
    
    /**
     * @brief 请求打开详细信息标签页信号
     * @param item 要打开的实体项目
     */
    void openDetailTab(QSharedPointer<ResultItem> item);
    
    // 右键菜单相关信号
    /**
     * @brief 在浏览器中打开实体信号
     * @param entityId 实体ID
     * @param entityType 实体类型
     */
    void openInBrowser(const QString &entityId, EntityType entityType);
    
    /**
     * @brief 复制实体ID信号
     * @param entityId 要复制的实体ID
     */
    void copyId(const QString &entityId);

public slots:
    // =============================================================================
    // 公共槽函数
    // =============================================================================
    
    /**
     * @brief 刷新实体信息槽函数
     * 
     * 重新加载并显示实体的详细信息，通常在数据更新后调用。
     */
    void refreshItemInfo();

private slots:
    void onSubTabChanged(int index);
    void onSubItemDoubleClicked(QSharedPointer<ResultItem> item);
    void onOpenInBrowser(const QString &itemId, EntityType type);
    void onCopyId(const QString &itemId);

private:
    void setupUI();
    void setupSubTabs();
    void populateItemInfo();
    void populateSubTabs();
    void createSubTab(const QString &title, const QString &key, EntityType entityType);    void createOverviewTab();
    void createAliasesTab();
    void createTagsTab();
    void createRelationshipsTab();
    
    // 实体类型特定的概览页面填充方法
    void populateArtistOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent);
    void populateReleaseOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent);
    void populateRecordingOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent);
    void populateReleaseGroupOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent);
    void populateLabelOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent);
    void populateWorkOverview(QVBoxLayout *layout, const QVariantMap &detailData, QWidget *parent);    // 通用组件创建方法
    QWidget* createArtistCreditWidget(const QVariantList &artistCredits);
    QWidget* createGenresWidget(const QVariantList &genres);
    QWidget* createGenresSection(const QVariantList &genres);
    QWidget* createTagsSection(const QVariantList &tags);
    
    // 结构化信息显示辅助方法
    void clearInfoLayout(QVBoxLayout *layout);
    void populateInfoSection(QVBoxLayout *layout, const QMap<QString, QVariant> &data);
    void populateDetailsSection(QVBoxLayout *layout, const QVariantMap &data);
    QString formatComplexValue(const QVariantMap &map);
    
    Ui::ItemDetailTab *ui;
    QSharedPointer<ResultItem> m_item;
    
    // 子标签页组件映射
    QHash<QString, EntityListWidget*> m_subTabWidgets;
    
    // 子标签页键名映射
    QStringList m_subTabKeys;
};

#endif // ITEMDETAILTAB_H
