#include "calenderwidget.h"
#include "ui_calenderwidget.h"
#include <QCalendarWidget>
#include <QDebug>
#include <QToolButton>

CalenderWidget::CalenderWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalenderWidget)
{
    ui->setupUi(this);
    ui->stayVisible->setChecked(settings.value("calendarStayVisible",true).toBool());
    ui->todayPushButon->setEnabled(ui->calendarWidget->selectedDate()!=QDate::currentDate());
    ui->calendarWidget->setMaximumDate(QDate::currentDate());
    ui->calendarWidget->setMinimumDate(QDate(1999,1,1));
    connect(ui->calendarWidget,&QCalendarWidget::selectionChanged,[=](){
       emit selectionChanged(ui->calendarWidget->selectedDate());
       ui->todayPushButon->setEnabled(ui->calendarWidget->selectedDate()!=QDate::currentDate());
    });

//    foreach (QWidget *wid, this->findChildren<QWidget*>()) {

//        if(wid->objectName()=="qt_calendar_prevmonth"
//                || wid->objectName()=="qt_calendar_nextmonth"){
//            wid->setStyleSheet("padding:4px");
//        }
//        if(wid->objectName()=="qt_calendar_monthbutton"){
//            QToolButton *btn = qobject_cast<QToolButton*>(wid);
//            btn->setStyleSheet("QToolButton::menu-indicator{width:0px;}");
//        }
//        if(wid->objectName()=="qt_calendar_calendarview"){
//            wid->setStyleSheet("border:none;");
//        }
//        if(wid->objectName()=="qt_calendar_navigationbar"){
//            wid->setContentsMargins(0, 0, 0, 4);
//        }
//    }
}

CalenderWidget::~CalenderWidget()
{
    delete ui;
}

void CalenderWidget::on_todayPushButon_clicked()
{
    ui->calendarWidget->setSelectedDate(QDate::currentDate());
}

void CalenderWidget::on_stayVisible_toggled(bool checked)
{
   settings.setValue("calendarStayVisible",checked);
}
