﻿
#include "SkillAssetNavigator.h"

//==============================================================================
// SkillAssetTreeView

SkillAssetTreeView::SkillAssetTreeView(QWidget* parent)
    : QTreeView(parent)
{
    m_model = new SkillAssetTreeModel(this);
    //QModelIndex rootModelIndex = m_model->setRootPath("D:/Proj/LN/PrivateProjects/HC0/Assets/Skill");

    //setHeaderHidden(true);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setTextElideMode(Qt::ElideNone);
    setModel(m_model);
    //setRootIndex(rootModelIndex);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //setFixedHeight(500);
    //setColumnWidth(0, INT_MAX);

    // ダブルクリックで編集を開始しないようにする
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onDoubleClicked(const QModelIndex &)));

    //connect(this, SIGNAL(doubleclicked(QModelIndex)), this, SLOT(OnDoubleclicked(QModelIndex)));
}

void SkillAssetTreeView::setRootDir(QString dir)
{
    QModelIndex rootModelIndex = m_model->setRootPath(dir);
    setRootIndex(rootModelIndex);
}

void SkillAssetTreeView::onDoubleClicked(const QModelIndex &index)
{
    QString filePath = m_model->fileInfo(index).absoluteFilePath();
}

//==============================================================================
// SkillContentsViewProvider

SkillContentsViewProvider::SkillContentsViewProvider(QWidget* parent)
    : ContentsViewProvider(parent)
    , m_treeView(nullptr)
{
	m_rootLayout = new QVBoxLayout(this);
	m_expander = new Expander("Test", this);
	m_treeView = new SkillAssetTreeView(this);

	m_expander->setContent(m_treeView);

	m_rootLayout->addWidget(m_expander);
	setLayout(m_rootLayout);
}
