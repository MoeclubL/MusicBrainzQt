#include "entitylistwidget.h"
#include "ui_entitylistwidget.h"
#include "../models/resultitem.h"
#include "../models/resulttablemodel.h"
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QGuiApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>

EntityListWidget::EntityListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EntityListWidget)
    , m_model(nullptr)
    , m_contextMenu(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupActions();
    setupContextMenu();
}

EntityListWidget::~EntityListWidget()
{
    delete ui;
}

void EntityListWidget::setupUI()
{
    // 创建模型
    m_model = new ResultTableModel(this);
    ui->tableView->setModel(m_model);    // 配置表格视图
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSectionsMovable(true);  // 允许拖动列
    ui->tableView->horizontalHeader()->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSortingEnabled(true); // 使用 Qt 默认排序
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // 配置表头右键菜单
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &EntityListWidget::onHeaderContextMenuRequested);
    
    // 连接信号
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &EntityListWidget::onTableDoubleClicked);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EntityListWidget::onTableSelectionChanged);    connect(ui->tableView, &QTableView::customContextMenuRequested,
            this, &EntityListWidget::onContextMenuRequested);
    
    // 连接分页信号
    connect(ui->prevButton, &QPushButton::clicked,
            this, &EntityListWidget::onPrevPageClicked);
    connect(ui->nextButton, &QPushButton::clicked,
            this, &EntityListWidget::onNextPageClicked);
    
    // 默认隐藏分页控件
    setPaginationVisible(false);
}

void EntityListWidget::setupActions()
{
    // 打开（与双击效果相同）
    m_openAction = new QAction(tr("Open"), this);
    m_openAction->setIcon(QIcon(":/icons/app_icon.svg"));
    connect(m_openAction, &QAction::triggered,
            this, &EntityListWidget::onOpen);
    
    // 在浏览器中打开
    m_openInBrowserAction = new QAction(tr("Open in Browser"), this);
    m_openInBrowserAction->setIcon(QIcon(":/icons/browser.svg"));
    connect(m_openInBrowserAction, &QAction::triggered,
            this, &EntityListWidget::onOpenInBrowser);
    
    // 复制ID
    m_copyIdAction = new QAction(tr("Copy ID"), this);
    connect(m_copyIdAction, &QAction::triggered,
            this, &EntityListWidget::onCopyId);
      // 列设置
    m_columnSettingsAction = new QAction(tr("Column Settings..."), this);
    connect(m_columnSettingsAction, &QAction::triggered,
            this, &EntityListWidget::onShowColumnSettings);
      // 不再添加到工具栏，只保留右键菜单功能
    // ui->toolBar->addAction(m_columnSettingsAction);
    // ui->toolBar->addSeparator();
    // ui->toolBar->addAction(m_openInBrowserAction);
    // ui->toolBar->addAction(m_copyIdAction);
    
    // 隐藏工具栏，因为所有功能都通过右键菜单提供
    ui->toolBar->setVisible(false);
}

void EntityListWidget::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(m_openAction);
    m_contextMenu->addAction(m_openInBrowserAction);
    m_contextMenu->addAction(m_copyIdAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_columnSettingsAction);
}

void EntityListWidget::setItems(const QList<QSharedPointer<ResultItem>> &items)
{
    // 如果列表不为空，使用第一个项目的类型作为表格类型
    EntityType type = EntityType::Unknown;
    if (!items.isEmpty() && items.first()) {
        type = items.first()->getType();
    }
    
    m_model->setItems(items, type);
    qDebug() << "EntityListWidget::setItems - Set" << items.count() 
                   << "items of type" << static_cast<int>(type);
}



QSharedPointer<ResultItem> EntityListWidget::getCurrentItem() const
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        return nullptr;
    }
    
    int row = selection.first().row();
    return m_model->getItem(row);
}

QList<QSharedPointer<ResultItem>> EntityListWidget::getSelectedItems() const
{
    QList<QSharedPointer<ResultItem>> items;
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    
    for (const QModelIndex &index : selection) {
        auto item = m_model->getItem(index.row());
        if (item) {
            items << item;
        }
    }
    
    return items;
}

void EntityListWidget::setVisibleColumns(const QStringList &columnKeys)
{
    m_model->setVisibleColumns(columnKeys);
}

QStringList EntityListWidget::getVisibleColumns() const
{
    return m_model->getVisibleColumns();
}

void EntityListWidget::resetToDefaultColumns()
{
    m_model->resetToDefaultColumns();
}

void EntityListWidget::setToolBarVisible(bool visible)
{
    ui->toolBar->setVisible(visible);
}

QToolBar* EntityListWidget::getToolBar() const
{
    return ui->toolBar;
}

void EntityListWidget::onTableDoubleClicked(const QModelIndex &index)
{
    auto item = m_model->getItem(index.row());
    if (item) {
        emit itemDoubleClicked(item);
    }
}

void EntityListWidget::onTableSelectionChanged()
{
    emit itemSelectionChanged();
}

void EntityListWidget::onContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tableView->indexAt(pos);
    if (index.isValid()) {
        // 将表格视图中的位置映射到全局坐标
        QPoint globalPos = ui->tableView->viewport()->mapToGlobal(pos);
        m_contextMenu->exec(globalPos);
    }
}

void EntityListWidget::onOpen()
{
    auto item = getCurrentItem();
    if (item) {
        emit itemDoubleClicked(item);
    }
}

void EntityListWidget::onOpenInBrowser()
{
    auto item = getCurrentItem();
    if (item) {
        emit openInBrowser(item->getId(), item->getType());
    }
}

void EntityListWidget::onCopyId()
{
    auto item = getCurrentItem();
    if (item) {
        emit copyId(item->getId());
    }
}

void EntityListWidget::onShowColumnSettings()
{
    showColumnSettingsDialog();
}

void EntityListWidget::showColumnSettingsDialog()
{
    if (!m_model || m_model->currentType() == EntityType::Unknown) {
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Column Settings"));
    dialog.setModal(true);
    dialog.resize(400, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // 说明文字
    QLabel *infoLabel = new QLabel(tr("Select columns to display:"));
    mainLayout->addWidget(infoLabel);
    
    // 获取可用列和当前显示列
    QList<ColumnInfo> availableColumns = m_model->getAvailableColumns(m_model->currentType());
    QStringList visibleColumns = m_model->getVisibleColumns();
    
    // 创建复选框组
    QGroupBox *columnGroup = new QGroupBox(tr("Available Columns"));
    QGridLayout *gridLayout = new QGridLayout(columnGroup);
    
    QList<QCheckBox*> checkBoxes;
    int row = 0, col = 0;
    
    for (const ColumnInfo &columnInfo : availableColumns) {
        QCheckBox *checkBox = new QCheckBox(columnInfo.header);
        checkBox->setToolTip(columnInfo.description);
        checkBox->setProperty("columnKey", columnInfo.key);
        checkBox->setChecked(visibleColumns.contains(columnInfo.key));
        
        gridLayout->addWidget(checkBox, row, col);
        checkBoxes << checkBox;
        
        col++;
        if (col >= 2) { // 两列布局
            col = 0;
            row++;
        }
    }
    
    mainLayout->addWidget(columnGroup);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *selectAllBtn = new QPushButton(tr("Select All"));
    QPushButton *selectNoneBtn = new QPushButton(tr("Select None"));
    QPushButton *resetBtn = new QPushButton(tr("Reset to Default"));
    QPushButton *okBtn = new QPushButton(tr("OK"));
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"));
    
    buttonLayout->addWidget(selectAllBtn);
    buttonLayout->addWidget(selectNoneBtn);
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接按钮信号
    connect(selectAllBtn, &QPushButton::clicked, [checkBoxes]() {
        for (QCheckBox *cb : checkBoxes) {
            cb->setChecked(true);
        }
    });
    
    connect(selectNoneBtn, &QPushButton::clicked, [checkBoxes]() {
        for (QCheckBox *cb : checkBoxes) {
            cb->setChecked(false);
        }
    });
    
    connect(resetBtn, &QPushButton::clicked, [this, checkBoxes]() {
        QList<ColumnInfo> defaultColumns = m_model->getDefaultColumns(m_model->currentType());
        QStringList defaultKeys;
        for (const ColumnInfo &info : defaultColumns) {
            defaultKeys << info.key;
        }
        
        for (QCheckBox *cb : checkBoxes) {
            QString key = cb->property("columnKey").toString();
            cb->setChecked(defaultKeys.contains(key));
        }
    });
    
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedColumns;
        for (QCheckBox *checkBox : checkBoxes) {
            if (checkBox->isChecked()) {
                selectedColumns << checkBox->property("columnKey").toString();
            }
        }
        
        if (!selectedColumns.isEmpty()) {
            setVisibleColumns(selectedColumns);
        }
    }
}

void EntityListWidget::setPaginationVisible(bool visible)
{
    // 设置分页控件的可见性
    ui->pageInfoLabel->setVisible(visible);
    ui->prevButton->setVisible(visible);
    ui->nextButton->setVisible(visible);
}

void EntityListWidget::setPaginationInfo(const QString &info)
{
    ui->pageInfoLabel->setText(info);
}

void EntityListWidget::setPaginationEnabled(bool prevEnabled, bool nextEnabled)
{
    ui->prevButton->setEnabled(prevEnabled);
    ui->nextButton->setEnabled(nextEnabled);
}

void EntityListWidget::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
    ui->titleLabel->setVisible(!title.isEmpty());
}

void EntityListWidget::onPrevPageClicked()
{
    emit prevPageRequested();
}

void EntityListWidget::onNextPageClicked()
{
    emit nextPageRequested();
}

void EntityListWidget::onHeaderContextMenuRequested(const QPoint &pos)
{
    // 创建表头右键菜单
    QMenu headerMenu(this);
    headerMenu.addAction(m_columnSettingsAction);
    
    // 显示菜单
    QPoint globalPos = ui->tableView->horizontalHeader()->mapToGlobal(pos);
    headerMenu.exec(globalPos);
}
