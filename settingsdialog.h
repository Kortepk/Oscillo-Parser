#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:

    void on_Apply_pushButton_released();

private:
    Ui::SettingsDialog *ui;
    void fillPortsParameters(void);
    void fillPortsInfo(void);
};

#endif // SETTINGSDIALOG_H
