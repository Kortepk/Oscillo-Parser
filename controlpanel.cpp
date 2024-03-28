#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    QStringList strlst;

    for(int i=1; i<=9; i++)
        strlst.append("Channel " + QString::number(i));

    ui->ChannelSelection_comboBox->addItems(strlst);
}

ControlPanel::~ControlPanel()
{
    delete ui;
}
