#ifndef CALENDERWIDGET_H
#define CALENDERWIDGET_H

#include <QDialog>
#include <QDate>

namespace Ui {
class CalenderWidget;
}

class CalenderWidget : public QDialog
{
    Q_OBJECT

public:
    explicit CalenderWidget(QWidget *parent = nullptr);
    ~CalenderWidget();
signals:
    void selectionChanged(const QDate date);

private slots:
    void on_todayPushButon_clicked();

private:
    Ui::CalenderWidget *ui;
};

#endif // CALENDERWIDGET_H
