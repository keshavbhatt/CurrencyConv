#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icons/icon-256.png"));
    this->setWindowTitle(qApp->applicationName()+" v"+qApp->applicationVersion());
    setStyle(":/light.qss");
    setFixedHeight(this->minimumSizeHint().height()+8);
    setFixedWidth(this->minimumSizeHint().width()+400);

    setGeometry(QStyle::alignedRect(
                Qt::LeftToRight,Qt::AlignCenter,size(),
                QApplication::desktop()->availableGeometry()
                ));

    init_appMenu();

    init_loader();

    init_request();

    getRates();
}

void MainWindow::init_appMenu()
{
    connect(ui->actionAbout_Qt,SIGNAL(triggered(bool)),qApp,SLOT(aboutQt()));
    connect(ui->actionQuit,SIGNAL(triggered(bool)),qApp,SLOT(quit()));
    //TODO implement more

}

void MainWindow::init_request()
{
    _request = new Request(this);
    connect(_request,&Request::requestStarted,[=]()
    {
        _loader->start();
    });
    connect(_request,&Request::requestFinished,[=](QString reply)
    {
        _loader->stop();
        setRates(reply);
    });

    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        _loader->stop();
        qDebug()<<errorString;
        ui->statusBar->showMessage(tr("Error while loading exchange rates."));

    });
}

void MainWindow::init_loader()
{
    if(_loader == nullptr){
        _loader = new WaitingSpinnerWidget(this,true,true);
        _loader->setRoundness(70.0);
        _loader->setMinimumTrailOpacity(15.0);
        _loader->setTrailFadePercentage(70.0);
        _loader->setNumberOfLines(40);
        _loader->setLineLength(20);
        _loader->setLineWidth(2);
        _loader->setInnerRadius(2);
        _loader->setRevolutionsPerSecond(3);
        _loader->setColor(QColor(159,160,164));
    }
}

void MainWindow::getRates()
{
    _request->get(QUrl("https://api.exchangerate.host/latest?base=USD&places=2"));
    ui->statusBar->showMessage(tr("Loading exchange rates..."));
}

void MainWindow::setRates(QString reply)
{
    exchange.clear();
    ui->s1ComboBox->clear();
    ui->s2ComboBox->clear();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply.toUtf8());
    if(jsonResponse.isEmpty()){
        qDebug()<<"API:"<<"Empty response returned from API call. Please report to developer.";
        return;
    }
    QString dateStr = jsonResponse.object().value("date").toString();
    ui->statusBar->showMessage(tr("Exchange rates from ")+dateStr);
    QJsonObject rateObj = jsonResponse.object().value("rates").toObject();
    //popluate exchange
    ui->s1ComboBox->blockSignals(true);
    ui->s2ComboBox->blockSignals(true);
    foreach (const QString &key, rateObj.keys()) {
        QJsonValue val = rateObj.value(key);
        ui->s1ComboBox->addItem(key);
        ui->s2ComboBox->addItem(key);
        exchange.insert(key,val.toDouble(0.00));
    }
    ui->s1ComboBox->setCurrentText(settings.value("baseCurr","USD").toString());
    ui->s1ComboBox->blockSignals(false);
    ui->s2ComboBox->blockSignals(false);
    //init exchange
    ui->s1SpinBox->setValue(1.00);
}

void MainWindow::setStyle(QString fname)
{
    QFile styleSheet(fname);
    if (!styleSheet.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open file");
    return; }
    qApp->setStyleSheet(styleSheet.readAll());
    //linedit bg
    foreach (QDoubleSpinBox* lEd, this->findChildren<QDoubleSpinBox*>()) {
        lEd->setStyleSheet("QDoubleSpinBox{"
                           "border:none;"
                           "color: gray;"
                           "margin-right: -16px;"
                           "background-color:transparent;}");
        lEd->setButtonSymbols(QAbstractSpinBox::NoButtons);
        lEd->setGroupSeparatorShown(true);
        lEd->setRange(0.01,99999999999999.99);
    }
    styleSheet.close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(_request!=nullptr){
        _request->blockSignals(true);
        _request->cancelAll();
        _request->deleteLater();
    }
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_s1SpinBox_valueChanged(double arg1)
{
    ui->s1SpinBox->blockSignals(true);
    ui->s1ComboBox->blockSignals(true);
    ui->s2SpinBox->setValue(convert(arg1,ui->s1ComboBox->currentText(),ui->s2ComboBox->currentText()));
    ui->s1ComboBox->blockSignals(false);
    ui->s1SpinBox->blockSignals(false);
}

void MainWindow::on_s2SpinBox_valueChanged(double arg1)
{
    ui->s2SpinBox->blockSignals(true);
    ui->s2ComboBox->blockSignals(true);
    ui->s1SpinBox->setValue(convert(arg1,ui->s2ComboBox->currentText(),ui->s1ComboBox->currentText()));
    ui->s2ComboBox->blockSignals(false);
    ui->s2SpinBox->blockSignals(false);
}

void MainWindow::on_s1ComboBox_currentIndexChanged(const QString &arg1)
{
    ui->s2SpinBox->blockSignals(true);
    ui->s2ComboBox->blockSignals(true);
    ui->s1SpinBox->setValue(convert(ui->s2SpinBox->value(),ui->s2ComboBox->currentText(),arg1));
    ui->s2ComboBox->blockSignals(false);
    ui->s2SpinBox->blockSignals(false);
}

void MainWindow::on_s2ComboBox_currentIndexChanged(const QString &arg1)
{
    ui->s1SpinBox->blockSignals(true);
    ui->s1ComboBox->blockSignals(true);
    ui->s2SpinBox->setValue(convert(ui->s1SpinBox->value(),ui->s1ComboBox->currentText(),arg1));
    ui->s1ComboBox->blockSignals(false);
    ui->s1SpinBox->blockSignals(false);
}

double MainWindow::convert(double value, QString s_cur, QString t_cur)
{
    qDebug()<<"CONVERT:"<<value<<s_cur<<" to "<<t_cur;
    double result = 0.00;
    result = (exchange.value("USD")/exchange.value(s_cur))*value;
    result = result*exchange.value(t_cur);
    return result;
}
