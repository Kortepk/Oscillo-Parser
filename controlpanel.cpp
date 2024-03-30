#include "controlpanel.h"
#include "ui_controlpanel.h"
#include <QDebug>
bool TurnFlowMode_Flag = false;

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    // Настройка комбобоксов
    for(int i=1; i<=9; i++)
        ui->ChannelSelection_comboBox->addItem("Channel " + QString::number(i), i);

    ui->PrefixScaleY_comboBox->setCurrentIndex(0);

    ui->PrefixScaleX_comboBox->addItem(tr("kilo"), 3);
    ui->PrefixScaleX_comboBox->addItem(tr("^0"), 0);
    ui->PrefixScaleX_comboBox->addItem(tr("milli"), -3);
    ui->PrefixScaleX_comboBox->addItem(tr("micro"), -6);
    ui->PrefixScaleX_comboBox->addItem(tr("nano"), -9);
    ui->PrefixScaleX_comboBox->setCurrentIndex(1);

    ui->PrefixScaleY_comboBox->addItem(tr("kilo"), 3);
    ui->PrefixScaleY_comboBox->addItem(tr("^0"), 0);
    ui->PrefixScaleY_comboBox->addItem(tr("milli"), -3);
    ui->PrefixScaleY_comboBox->addItem(tr("micro"), -6);
    ui->PrefixScaleY_comboBox->addItem(tr("nano"), -9);
    ui->PrefixScaleY_comboBox->setCurrentIndex(1);

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
                              value,
                              0,
                              ui->GraphScale_dial->value(),
                              0
                              );
    // Если мы меняем настройки для всех каналов, то мы должны сохранить значения для y => можно не передавать значения
}

void ControlPanel::on_GraphScale_dial_valueChanged(int value)
{
    ui->GraphScale_Box->setValue(value);
    emit ChannelChange_Signal(-1,
                              ui->GraphPosition_dial->value(),
                              0,
                              value,
                              0
                              );
}


void ControlPanel::on_ChannalPosition_dial_valueChanged(int value)
{
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    emit ChannelChange_Signal(chan_num,
                              ui->GraphPosition_dial->value(),
                              value,
                              ui->GraphScale_dial->value(),
                              ui->ChannalScale_dial->value()
                              );
}


void ControlPanel::on_ChannalScale_dial_valueChanged(int value)
{
    ui->ChannelScale_Box->setValue(value);
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    emit ChannelChange_Signal(chan_num,
                              ui->GraphPosition_dial->value(),
                              ui->ChannalPosition_dial->value(),
                              ui->GraphScale_dial->value(),
                              value
                              );
}


void ControlPanel::on_StartPause_Button_clicked()
{
    emit StartPause_Signal();

    // Обратной связи от MainWindow нету, поэтмоу такое изменение кнопки

    if(ui->StartPause_Button->text() == "Pause")
    {
        ui->StartPause_Button->setText("Start");
        ui->StartPause_Button->setStyleSheet(
            "QPushButton{border-radius: 4px;background-color: rgb(156, 255, 156);}"
            );
    }
    else
    {
        ui->StartPause_Button->setText("Pause");
        ui->StartPause_Button->setStyleSheet(
            "QPushButton{border-radius: 4px;background-color: rgb(255, 156, 156);}"
            );
    }
}

