#include "controlpanel.h"
#include "ui_controlpanel.h"
#include <QDebug>
bool TurnFlowMode_Flag = false;

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

