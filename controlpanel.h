#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

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

signals:
    void TurnFlowMode_Signal(bool);
    void GroupSize_Signal(int);
    void CounterChannel_Signal(int);

private slots:
    void on_TurnFlowMode_clicked();
    void on_GroupSize_Slider_sliderMoved(int position);
    void on_CounterChannel_Box_valueChanged(int arg1);
    void on_GroupSize_Box_valueChanged(int arg1);

private:
    Ui::ControlPanel *ui;
    void CheckTurnFlowMode();
};

#endif // CONTROLPANEL_H
