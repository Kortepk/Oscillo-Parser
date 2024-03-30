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
    float ShiftMid_x = 0;

signals:
    void TurnFlowMode_Signal(bool); // (Событие на изменение режима виджета) arg: плавающий/не плавающий
    void GroupSize_Signal(int); // (Событие изменения размера графиков) arg: новый размер
    void CounterChannel_Signal(int); // (Событие изменения количество каналов) arg: количество каналов
    void ChannelChange_Signal(int, float, float, float, float); // arg: Номер канала,  cдвиг относительно триггера, позиция y, масштаб по x (времени), масштаб по y
    void StartPause_Signal();

private slots:
    void on_TurnFlowMode_clicked();
    void on_GroupSize_Slider_sliderMoved(int position);
    void on_CounterChannel_Box_valueChanged(int arg1);
    void on_GroupSize_Box_valueChanged(int arg1);
    void on_GraphScale_dial_valueChanged(int value);
    void on_GraphPosition_dial_valueChanged(int value);
    void on_ChannalPosition_dial_valueChanged(int value);
    void on_ChannalScale_dial_valueChanged(int value);
    void on_StartPause_Button_clicked();

    void on_ChannalPosition_dial_sliderPressed();

    void on_GraphPosition_dial_sliderPressed();

    void on_ChannalScale_dial_sliderPressed();

    void on_GraphScale_dial_sliderPressed();

    void on_ChannelScale_Box_valueChanged(int arg1);

    void on_GraphScale_Box_valueChanged(int arg1);

private:
    Ui::ControlPanel *ui;
    QTimer *UItimer = new QTimer(this);

    void CheckTurnFlowMode();
    void CheckUI();

    int ChannalSize = 1;
    bool KnobBlockFlag = false;
};

#endif // CONTROLPANEL_H
