#include "statisticsview.h"

StatisticsView::StatisticsView(QWidget *parent) : QWidget(parent)
{
    // 创建UI
    lblTitle = new QLabel("统计信息", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-size: 20px; font-weight: bold; margin: 10px;");

    textStats = new QTextEdit(this);
    textStats->setReadOnly(true);
    textStats->setPlaceholderText("统计信息将在这里显示...");

    btnBack = new QPushButton("返回", this);
    btnBack->setMinimumSize(80, 30);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(lblTitle);
    mainLayout->addWidget(textStats, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnBack);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(btnBack, &QPushButton::clicked, this, [this]() {
        emit close();
    });
}
