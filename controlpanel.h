#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "qtimer.h"
#include <QWidget>

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();
    int Get_MaxPointSlider();
    int Get_GroupSizeValue();
    void Change_TurnFlowMode(bool FlowMode_Flag);
    void SetTrigValue(float val);
    void SetDialPositionScale(float x, float y, float dx, float dy);

    struct GraphSettings
    {
        float GraphShiftX = 0; // Предыдущее полоджение ручки
        float ChannelShiftY = 0;
        float GraphScaleX = 1;
        float ChannelScaleY = 1;
        float ScalePrefixX = 1;
        int DialTurnoversX = 0; // Количество оборотов ручки сдвига
        int DialTurnoversY = 0;
        float ShiftMid_x = 0;
        float ShiftMid_y = 0;
    } ViewGraphSet;

signals:
    void TurnFlowMode_Signal(bool); // (Событие на изменение режима виджета) arg: плавающий/не плавающий
    void GroupSize_Signal(int); // (Событие изменения размера графиков) arg: новый размер
    void CounterChannel_Signal(int); // (Событие изменения количество каналов) arg: количество каналов
    void ChannelChange_Signal(int); // arg: Номер канала,  cдвиг относительно триггера, позиция y, масштаб по x (времени), масштаб по y
    void StartPause_Signal();
    void TestPushButton_Signal();
    void TriggerChanged_Signal(int, float); // arg: Канал, значение триггера
    void ChangeParseMode_Signal(int); // Сообщает об единичном, постоянном или тригерном режиме
    void ClickHalfTrig_Signal();
    void AutoSize_Signal();

public slots:
    void on_StartPause_Button_clicked();

private slots:
    void on_TurnFlowMode_clicked();
    void on_GroupSize_Slider_sliderMoved(int position);
    void on_CounterChannel_Box_valueChanged(int arg1);
    void on_GroupSize_Box_valueChanged(int arg1);
    void on_GraphScale_dial_valueChanged(int value);
    void on_GraphPosition_dial_valueChanged(int value);
    void on_ChannalPosition_dial_valueChanged(int value);
    void on_ChannalScale_dial_valueChanged(int value);
    void on_ChannalPosition_dial_sliderPressed();
    void on_GraphPosition_dial_sliderPressed();
    void on_ChannalScale_dial_sliderPressed();
    void on_GraphScale_dial_sliderPressed();
    void on_pushButtonTest_clicked();
    void on_MaxPoint_Slider_sliderMoved(int position);
    void on_Allways_rb_clicked();
    void on_Single_rb_clicked();
    void on_Trigger_rb_clicked();
    void on_TriggerPosition_dial_valueChanged(int value);

    void on_SetHalf_Button_clicked();

    void on_AutoSize_Button_clicked();

private:
    Ui::ControlPanel *ui;
    QTimer *UItimer = new QTimer(this);

    void CheckTurnFlowMode();
    void CheckUI();

    int ChannalSize = 1;
    bool KnobBlockFlag = false;
};

#endif // CONTROLPANEL_H
