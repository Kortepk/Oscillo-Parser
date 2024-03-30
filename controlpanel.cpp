#include "controlpanel.h"
#include "ui_controlpanel.h"
#include <QDebug>
bool TurnFlowMode_Flag = false;

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    for(int i=1; i<=9; i++)
        ui->ChannelSelection_comboBox->addItem("Channel " + QString::number(i), i);

    QTreeWidgetItem *itm = new QTreeWidgetItem(ui->treeWidget);
    QTreeWidgetItem *itm2 = new QTreeWidgetItem(itm);
    itm->setText(0, "Hello");
    //ui->treeWidget->addTopLevelItem(itm);
    itm2->setText(0, "World");
    ui->treeWidget->addTopLevelItem(itm2);
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

void ControlPanel::CheckTurnFlowMode()
{
    if(TurnFlowMode_Flag) // Оконный\плавающий режим панели
    {
        ui->TurnFlowMode->setText("▲");
    }
    else
    {
        ui->TurnFlowMode->setText("▼");
    }

    emit TurnFlowMode_Signal(TurnFlowMode_Flag);
}

void ControlPanel::on_TurnFlowMode_clicked()
{
    TurnFlowMode_Flag = !TurnFlowMode_Flag;

    CheckTurnFlowMode();
}

void ControlPanel::Change_TurnFlowMode(bool FlowMode_Flag)
{
    TurnFlowMode_Flag = FlowMode_Flag;

    CheckTurnFlowMode();
}

int ControlPanel::Get_MaxPointSlider()
{
    return ui->MaxPoint_Slider->value();
}

int ControlPanel::Get_GroupSizeValue()
{
    return ui->GroupSize_Box->value();
}


void ControlPanel::on_GroupSize_Slider_sliderMoved(int position)
{
    ui->GroupSize_Box->setValue(position);

    emit GroupSize_Signal(position);
}


void ControlPanel::on_GroupSize_Box_valueChanged(int arg1)
{

    if(arg1 <= 1000)
        ui->GroupSize_Slider->setValue(arg1);
    else
        if(ui->GroupSize_Slider->value() != 1000)
            ui->GroupSize_Slider->setValue(1000);

    emit GroupSize_Signal(arg1);
}


void ControlPanel::on_CounterChannel_Box_valueChanged(int arg1)
{
    emit CounterChannel_Signal(arg1);
}

void ControlPanel::on_GraphPosition_dial_valueChanged(int value)
{
    emit ChannelChange_Signal(-1,
                              value - 50,
                              0,
                              ui->GraphScale_dial->value() * 10,
                              0
                              );
    // Если мы меняем настройки для всех каналов, то мы должны сохранить значения для y => можно не передавать значения
}

void ControlPanel::on_GraphScale_dial_valueChanged(int value)
{
    emit ChannelChange_Signal(-1,
                              ui->GraphPosition_dial->value() - 50,
                              0,
                              value * 10,
                              0
                              );
}


void ControlPanel::on_ChannalPosition_dial_valueChanged(int value)
{
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    emit ChannelChange_Signal(chan_num,
                              ui->GraphPosition_dial->value() - 50,
                              value - 50,
                              ui->GraphScale_dial->value() * 10,
                              ui->ChannalScale_dial->value()
                              );
}


void ControlPanel::on_ChannalScale_dial_valueChanged(int value)
{
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    emit ChannelChange_Signal(chan_num,
                              ui->GraphPosition_dial->value() - 50,
                              ui->ChannalPosition_dial->value() - 50,
                              ui->GraphScale_dial->value() * 10,
                              value
                              );
}

