#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icons/icon-256.png"));
    this->setWindowTitle(qApp->applicationName()+" v"+qApp->applicationVersion());

    int height = minimumSizeHint().height()+8;
    int width  = minimumSizeHint().width()+300;
    QSize size(width,height);
    setMinimumSize(size);

    if(settings.value("geometry").isValid()){
        restoreGeometry(settings.value("geometry").toByteArray());
        if(settings.value("windowState").isValid()){
            restoreState(settings.value("windowState").toByteArray());
        }else{
            QScreen* pScreen = QApplication::primaryScreen();// (this->mapToGlobal({this->width()/2,0}));
            QRect availableScreenSize = pScreen->availableGeometry();
            this->move(availableScreenSize.center()-this->rect().center());
        }
    }else{
        setGeometry(QStyle::alignedRect(
                    Qt::LeftToRight,Qt::AlignCenter,size,
                    QApplication::desktop()->availableGeometry()
                    ));
    }

    ui->s1ComboBox->installEventFilter(this);
    ui->s2ComboBox->installEventFilter(this);

    set_style();

    init_appMenu();

    init_loader();

    init_request();

    load_currencies();

    getRates(false);

}

void MainWindow::load_currencies()
{
   QJsonDocument currenciesdoc    =  utils::loadJson(":/resources/currencies.json");
   QJsonObject resultsObj         =  currenciesdoc.object().value("symbols").toObject();
   foreach (const QString &key, resultsObj.keys()) {
       QString name = resultsObj.value(key).toObject().value("description").toString();
       QString id = resultsObj.value(key).toObject().value("code").toString();
       currencyMeta.insert(id,name);
   }
}
void MainWindow::set_style()
{
    //:/qbreeze/dark.qss
    setStyle(":/qbreeze/"+settings.value("theme","light").toString()+".qss");
    bool light = false;
    light = settings.value("theme","light").toString()=="light"?true:false;
    ui->actionDark->setChecked(!light);
    ui->actionLight->setChecked(light);
}

void MainWindow::init_appMenu()
{
    connect(ui->actionAbout_Qt,SIGNAL(triggered(bool)),qApp,SLOT(aboutQt()));
    connect(ui->actionQuit,SIGNAL(triggered(bool)),qApp,SLOT(quit()));
    connect(ui->actionHistorical_rates,SIGNAL(triggered(bool)),this,SLOT(showHistoricalRate()));
    connect(ui->actionDark,&QAction::toggled,[=](bool checked){
        if (checked){
            settings.setValue("theme","dark");
            set_style();
        }
    });
    connect(ui->actionLight,&QAction::toggled,[=](bool checked){
        if (checked){
            settings.setValue("theme","light");
            set_style();
        }
    });
    connect(ui->actionAbout_Application,SIGNAL(triggered(bool)),this,SLOT(showAbout()));
    connect(ui->actionRate_in_store,&QAction::triggered,[=](){
        QDesktopServices::openUrl(QUrl("snap://currencyconv"));
    });

    connect(ui->actionRate_Graph,&QAction::triggered,[=](){
        _rateGraph = new RateGraph(this);
        _rateGraph->setWindowTitle(QApplication::applicationName()+" | "+tr("Rate Graph"));
        _rateGraph->setWindowModality(Qt::NonModal);
        _rateGraph->setWindowFlags(Qt::Window);
        _rateGraph->setAttribute(Qt::WA_DeleteOnClose);
        _rateGraph->show();
    });
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
        if(reply.isEmpty()){
            ui->statusBar->showMessage(tr("Empty response returned from API."));
        }else{
            setRates(reply);
        }
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
        _loader->setColor(QColor("#3DAEE9"));
    }
}

void MainWindow::showHistoricalRate()
{
    if(_calWidget==nullptr){
        _calWidget= new CalenderWidget(this);
        _calWidget->setWindowTitle(QApplication::applicationName()+" | "+tr("Historical data"));
        connect(_calWidget,&CalenderWidget::selectionChanged,[=](const QDate date){
            historicalDate = date.toString("yyyy-M-dd");
            if(settings.value("calendarStayVisible",true).toBool()==false)
                _calWidget->close();
            getRates(true);
        });
    }
    _calWidget->setWindowFlags(Qt::Dialog);
    _calWidget->setWindowModality(Qt::NonModal);
    _calWidget->show();
}

void MainWindow::getRates(bool historical)
{
    QUrl base_url("https://api.exchangerate.host/latest");
    if(historical){
        base_url = QUrl("https://api.exchangerate.host/"+historicalDate);
    }
    QUrlQuery query;
    query.addQueryItem("base","USD");
    //query.addQueryItem("places","2");
    base_url.setQuery(query);

    if(!historical){
        _homeUrl = base_url;
    }
    _currentUrl = base_url;
    _request->get(_currentUrl);
    ui->statusBar->showMessage(tr("Loading exchange rates..."));
}

void MainWindow::setRates(QString reply)
{
    QString currentS2Cur = ui->s2ComboBox->currentData(Qt::UserRole).toString();
    QString currentS1Cur = ui->s1ComboBox->currentData(Qt::UserRole).toString();

    exchange.clear();
    ui->s1ComboBox->clear();
    ui->s2ComboBox->clear();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply.toUtf8());
    if(jsonResponse.isEmpty()){
        qDebug()<<"API:"<<"Empty response returned from API call. Please report to developer.";
        return;
    }
    QString dateStr = jsonResponse.object().value("date").toString().trimmed();
    QDate date = QDate::fromString(dateStr,Qt::ISODate);
    dateStr = date.toString("d-MMM-yyyy");
    if(date == QDate::currentDate())
        dateStr.append(tr(" (Today)"));
    ui->statusBar->showMessage(tr("Exchange rates from ")+dateStr);
    QJsonObject rateObj = jsonResponse.object().value("rates").toObject();

    //popluate exchange
    ui->s1ComboBox->blockSignals(true);
    ui->s2ComboBox->blockSignals(true);

    foreach (const QString key, rateObj.keys()) {
        QJsonValue val = rateObj.value(key);
        ui->s1ComboBox->addItem(key+" ("+currencyMeta.value(key)+")",key);
        ui->s2ComboBox->addItem(key+" ("+currencyMeta.value(key)+")",key);
        exchange.insert(key,val.toDouble(0.00));
    }

    //combobox restore curr
    if(!currentS1Cur.isEmpty() || !currentS2Cur.isEmpty()){
        //keep last set currency only if available in the current exchange
        int iCb1 = getIndexByDataRole(currentS1Cur,ui->s1ComboBox);
        int iCb2 = getIndexByDataRole(currentS2Cur,ui->s2ComboBox);
        if(exchange.keys().indexOf(currentS1Cur)!=-1
                && iCb1 != -1)
         ui->s1ComboBox->setCurrentIndex(iCb1);
        if(exchange.keys().indexOf(currentS2Cur)!=-1
                && iCb2 != -1)
         ui->s2ComboBox->setCurrentIndex(iCb2);
    }else{
        //set last changed/saved currency only if available in the current exchange
        int s1CIndex = getIndexByDataRole(settings.value("s1CurrCode","USD").toString(),ui->s1ComboBox);
        int s2CIndex = getIndexByDataRole(settings.value("s2CurrCode","INR").toString(),ui->s2ComboBox);
        ui->s1ComboBox->setCurrentIndex(s1CIndex == -1 ? 0 : s1CIndex);
        ui->s2ComboBox->setCurrentIndex(s2CIndex == -1 ? 0 : s2CIndex);
    }

    ui->s1ComboBox->blockSignals(false);
    ui->s2ComboBox->blockSignals(false);
    //init exchange
    ui->s1SpinBox->setValue(1.00);
    foreach(QSizeGrip *wid, ui->statusBar->findChildren<QSizeGrip*>()){
         wid->hide();
    }
    s1ComboFix();
}

int MainWindow::getIndexByDataRole(QString dataValue,QComboBox *cBox)
{
    int index = -1;
    for (int i = 0; i < cBox->count(); ++i) {
        if(dataValue == cBox->itemData(i).toString()){
            index = i;
            break;
        }
    }
    return index;
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
        lEd->setGroupSeparatorShown(false);
        lEd->setRange(0.01,99999999999999.99);
    }
    styleSheet.close();
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Link,QColor("skyblue"));
    qApp->setPalette(palette);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if((obj==ui->s1ComboBox || obj==ui->s2ComboBox) && ev->type()==QEvent::Wheel)
    {
        return true;
    }
    return QMainWindow::eventFilter(obj,ev);
}

void MainWindow::s1ComboFix()
{
    //set width of s1combobox to match that of s2combobox
    int width = 0;
    for (int i = 0; i < ui->buttonsLayout->count(); ++i) {
        QWidget *wid = ui->buttonsLayout->itemAt(i)->widget();
        width += wid->width();
    }
    int spaces = ui->buttonsLayout->count() * ui->buttonsLayout->spacing();
    ui->spacerLayout->invalidate();
    ui->s1comboFix->changeSize(width+spaces,ui->s1comboFix->geometry().height());
    qWarning()<<"called"<<QStyleFactory::keys()<<width+spaces;
}

void MainWindow::resizeEvent(QResizeEvent *ev)
{
    s1ComboFix();
    QMainWindow::resizeEvent(ev);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("s1CurrCode",ui->s1ComboBox->currentData());
    settings.setValue("s2CurrCode",ui->s2ComboBox->currentData());

    settings.setValue("geometry",saveGeometry());
    settings.setValue("windowState", saveState());
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
    ui->s2SpinBox->blockSignals(true);
    ui->s2SpinBox->setValue(convert(arg1,ui->s1ComboBox->currentData(Qt::UserRole).toString(),ui->s2ComboBox->currentData(Qt::UserRole).toString()));
    ui->s2SpinBox->blockSignals(false);
    ui->s1ComboBox->blockSignals(false);
    ui->s1SpinBox->blockSignals(false);
}

void MainWindow::on_s2SpinBox_valueChanged(double arg1)
{
    ui->s2SpinBox->blockSignals(true);
    ui->s2ComboBox->blockSignals(true);
    ui->s1SpinBox->blockSignals(true);
    ui->s1SpinBox->setValue(convert(arg1,ui->s2ComboBox->currentData(Qt::UserRole).toString(),ui->s1ComboBox->currentData(Qt::UserRole).toString()));
    ui->s1SpinBox->blockSignals(false);
    ui->s2ComboBox->blockSignals(false);
    ui->s2SpinBox->blockSignals(false);
}

void MainWindow::on_s1ComboBox_currentIndexChanged(const QString &arg1)
{
    ui->s2SpinBox->blockSignals(true);
    ui->s2ComboBox->blockSignals(true);
    ui->s1SpinBox->blockSignals(true);
    ui->s1SpinBox->setValue(convert(ui->s2SpinBox->value(),ui->s2ComboBox->currentData(Qt::UserRole).toString(),arg1.split(" ").first()));
    ui->s1SpinBox->blockSignals(false);
    ui->s2ComboBox->blockSignals(false);
    ui->s2SpinBox->blockSignals(false);
}

void MainWindow::on_s2ComboBox_currentIndexChanged(const QString &arg1)
{
    ui->s1SpinBox->blockSignals(true);
    ui->s1ComboBox->blockSignals(true);
    ui->s2SpinBox->blockSignals(true);
    ui->s2SpinBox->setValue(convert(ui->s1SpinBox->value(),ui->s1ComboBox->currentData(Qt::UserRole).toString(),arg1.split(" ").first()));
    ui->s2SpinBox->blockSignals(false);
    ui->s1ComboBox->blockSignals(false);
    ui->s1SpinBox->blockSignals(false);
}

double MainWindow::convert(double value, QString s_cur, QString t_cur)
{
    if(s_cur.isEmpty()|| t_cur.isEmpty())
        return 0.00;
    qDebug()<<"CONVERT:"<<value<<s_cur<<" to "<<t_cur;
    double result = 0.00;
    result = (exchange.value("USD")/exchange.value(s_cur))*value;
    result = result*exchange.value(t_cur);
    return result;
}

void MainWindow::on_reload_clicked()
{
    //delete cached data if not historical
    //_request->clearCache(_currentUrl);
    _request->get(_currentUrl,false);
    ui->statusBar->showMessage(tr("Loading exchange rates..."));
}

void MainWindow::on_home_clicked()
{
    _request->clearCache(_homeUrl);
    getRates(false);
}

//about
void MainWindow::showAbout()
{
    QDialog *aboutDialog = new QDialog(this,Qt::Dialog);
    aboutDialog->setWindowModality(Qt::WindowModal);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *message = new QLabel(aboutDialog);
    connect(message,&QLabel::linkActivated,[=](QString link){
        QDesktopServices::openUrl(QUrl(link));
    });
    QPushButton *btn = new QPushButton("Donate",aboutDialog);
    connect(btn,&QPushButton::clicked,[=](){
        QDesktopServices::openUrl(QUrl("https://paypal.me/keshavnrj/4"));
    });
    btn->setIcon(QIcon(":/icons/others/paypal-line.png"));
    layout->addWidget(message);
    layout->addWidget(btn);
    aboutDialog->setLayout(layout);
    aboutDialog->setWindowTitle(tr("About")+" | "+QApplication::applicationName());
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose,true);
    aboutDialog->show();

    QString mes =
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><img src=':/icons/icon-64.png' /></p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Designed and Developed</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>by <span style=' font-weight:600;'>Keshav Bhatt</span> &lt;keshavnrj@gmail.com&gt;</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Website: https://ktechpit.com</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Runtime: Qt 5.5.1</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Version: "+QApplication::applicationVersion()+"</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><a href='https://snapcraft.io/search?q=keshavnrj'>More Apps</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>";

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    message->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(1000);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    message->setText(mes);
    message->show();
}

void MainWindow::on_settings_clicked()
{
    if(_settingsWidget==nullptr){
        _settingsWidget = new SettingsWidget(this);
        _settingsWidget->setWindowTitle(QApplication::applicationName()+" | "+tr("Settings"));
        _settingsWidget->setWindowFlags(Qt::Dialog);
        _settingsWidget->setWindowModality(Qt::NonModal);
        connect(_settingsWidget,&SettingsWidget::updateDoubleSpinBox,
                [=](){
            ui->s1SpinBox->setDecimals(settings.value("decimalValue",2).toInt());
            ui->s2SpinBox->setDecimals(settings.value("decimalValue",2).toInt());
        });
    }
    if(!_settingsWidget->isVisible())
        _settingsWidget->show();
    else
        _settingsWidget->raise();
}
