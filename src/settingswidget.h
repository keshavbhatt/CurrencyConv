#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

signals:
    void updateDoubleSpinBox();

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

private slots:
    void on_rounding_valueChanged(int arg1);

private:
    Ui::SettingsWidget *ui;
    QSettings settings;
};

#endif // SETTINGSWIDGET_H
