#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "QDebug"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    qDebug() << "create";

    hide();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
