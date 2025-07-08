#include "advancedsearchwidget.h"
#include "ui_advancedsearchwidget.h"
#include <QDebug>

AdvancedSearchWidget::AdvancedSearchWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_AdvancedSearchWidget)
    , m_currentType(EntityType::Artist)
    , m_fieldsWidget(nullptr)
    , m_fieldsLayout(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setEntityType(EntityType::Artist);
}

AdvancedSearchWidget::~AdvancedSearchWidget()
{
    delete ui;
}

void AdvancedSearchWidget::setupUI()
{
    // 填充搜索类型下拉框
    ui->typeCombo->addItem(tr("Artist"), static_cast<int>(EntityType::Artist));
    ui->typeCombo->addItem(tr("Release"), static_cast<int>(EntityType::Release));
    ui->typeCombo->addItem(tr("Release Group"), static_cast<int>(EntityType::ReleaseGroup));
    ui->typeCombo->addItem(tr("Recording"), static_cast<int>(EntityType::Recording));
    ui->typeCombo->addItem(tr("Label"), static_cast<int>(EntityType::Label));
    ui->typeCombo->addItem(tr("Work"), static_cast<int>(EntityType::Work));
    ui->typeCombo->addItem(tr("Area"), static_cast<int>(EntityType::Area));
    ui->typeCombo->addItem(tr("Place"), static_cast<int>(EntityType::Place));
    ui->typeCombo->addItem(tr("Annotation"), static_cast<int>(EntityType::Annotation));
    ui->typeCombo->addItem(tr("CD Stub"), static_cast<int>(EntityType::CDStub));
    ui->typeCombo->addItem(tr("Editor"), static_cast<int>(EntityType::Editor));
    ui->typeCombo->addItem(tr("Tag"), static_cast<int>(EntityType::Tag));
    ui->typeCombo->addItem(tr("Instrument"), static_cast<int>(EntityType::Instrument));
    ui->typeCombo->addItem(tr("Series"), static_cast<int>(EntityType::Series));
    ui->typeCombo->addItem(tr("Event"), static_cast<int>(EntityType::Event));
    ui->typeCombo->addItem(tr("Documentation"), static_cast<int>(EntityType::Documentation));
    
    // 连接信号
    connect(ui->typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdvancedSearchWidget::onTypeChanged);
    connect(ui->queryEdit, &QLineEdit::returnPressed,
            this, &AdvancedSearchWidget::onSearchClicked);
    connect(ui->searchButton, &QPushButton::clicked,
            this, &AdvancedSearchWidget::onSearchClicked);
    
}

SearchParameters AdvancedSearchWidget::getSearchParameters() const
{
    SearchParameters params;
    params.type = m_currentType;
    params.limit = ui->limitSpinBox->value(); // 使用用户选择的每页结果数
    
    if (ui->tabWidget->currentIndex() == 0) {
        // 简单搜索
        params.query = ui->queryEdit->text().trimmed();
    } else {
        // 高级搜索
        QStringList queryParts;
        
        for (auto it = m_fields.constBegin(); it != m_fields.constEnd(); ++it) {
            QString value;
            
            if (QLineEdit *edit = qobject_cast<QLineEdit*>(it.value())) {
                value = edit->text().trimmed();
            } else if (QSpinBox *spin = qobject_cast<QSpinBox*>(it.value())) {
                if (spin->value() > 0) {
                    value = QString::number(spin->value());
                }
            }
            
            if (!value.isEmpty()) {
                params.advancedFields[it.key()] = value;
                
                // 构建查询字符串
                if (value.contains(' ')) {
                    value = "\"" + value + "\"";
                }
                queryParts << QString("%1:%2").arg(it.key(), value);
            }
        }
        
        params.query = queryParts.join(" AND ");
    }
    
    return params;
}

void AdvancedSearchWidget::setEntityType(EntityType type)
{
    m_currentType = type;
    
    // 更新组合框
    for (int i = 0; i < ui->typeCombo->count(); ++i) {
        if (static_cast<EntityType>(ui->typeCombo->itemData(i).toInt()) == type) {
            ui->typeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    // 更新高级搜索字段
    createFieldsForType(type);
}



void AdvancedSearchWidget::createFieldsForType(EntityType type)
{
    // 清理现有字段
    qDeleteAll(m_fields);
    m_fields.clear();
    
    // 清理旧的字段组件
    if (m_fieldsWidget) {
        delete m_fieldsWidget;
    }
    
    // 创建新的字段组件
    m_fieldsWidget = new QWidget();
    m_fieldsLayout = new QGridLayout(m_fieldsWidget);
    ui->scrollArea->setWidget(m_fieldsWidget);
    
    int row = 0;
    
    switch (type) {
    case EntityType::Artist:
        addField("artist", tr("Artist"), tr("Artist name"), row);
        addField("release", tr("Release"), tr("Release title"), row);
        addField("track", tr("Track"), tr("Track title"), row);
        addField("area", tr("Area"), tr("Area or country"), row);
        break;
        
    case EntityType::Release:
        addField("release", tr("Release"), tr("Release title"), row);
        addField("artist", tr("Artist"), tr("Artist name"), row);
        addField("barcode", tr("Barcode"), tr("Barcode"), row);
        addField("catno", tr("Catalog Number"), tr("Catalog number"), row);
        addField("date", tr("Date"), tr("Release date"), row);
        break;
        
    case EntityType::Recording:
        addField("recording", tr("Recording"), tr("Recording title"), row);
        addField("artist", tr("Artist"), tr("Artist name"), row);
        addField("release", tr("Release"), tr("Release title"), row);
        addField("isrc", tr("ISRC"), tr("ISRC code"), row);
        break;
        
    case EntityType::Label:
        addField("label", tr("Label"), tr("Label name"), row);
        addField("country", tr("Country"), tr("Country code"), row);
        addField("begin", tr("Begin Date"), tr("Begin date"), row);
        break;
        
    default:
        addField("query", tr("Search"), tr("Search terms"), row);
        break;
    }
}

void AdvancedSearchWidget::addField(const QString &key, const QString &label, const QString &placeholder, int &row)
{
    m_fieldsLayout->addWidget(new QLabel(label + ":"), row, 0);
    
    QLineEdit *edit = new QLineEdit();
    edit->setPlaceholderText(placeholder);
    m_fieldsLayout->addWidget(edit, row, 1);
    
    m_fields[key] = edit;
    row++;
}

void AdvancedSearchWidget::onSearchClicked()
{
    SearchParameters params = getSearchParameters();
    if (params.isValid()) {
        emit searchRequested(params);
    }
}



void AdvancedSearchWidget::onTypeChanged()
{
    EntityType newType = static_cast<EntityType>(ui->typeCombo->currentData().toInt());
    setEntityType(newType);
}
