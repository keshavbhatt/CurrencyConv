#include "rategraph.h"
#include "ui_rategraph.h"


RateGraph::RateGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RateGraph)
{
    ui->setupUi(this);

    ui->fromDateEdit->setDisplayFormat("yyyy-M-dd");
    ui->toDateEdit->setDisplayFormat("yyyy-M-dd");

    ui->fromDateEdit->setDateRange(QDate(1999,1,1),QDate::currentDate());
    ui->toDateEdit->setDateRange(QDate(1999,1,1),QDate::currentDate());

    ui->fromDateEdit->setDate(QDate(2020,10,10));
    ui->toDateEdit->setDate(QDate::currentDate());

    setStatus(tr("idle"));

    init_loader();

    init_request();
}

RateGraph::~RateGraph()
{
    delete ui;
}

void RateGraph::init_loader()
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

void RateGraph::init_request()
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
            setStatus(tr("Empty response returned from API."));
        }else{
            setRates(reply);
        }
    });

    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        _loader->stop();
        qDebug()<<errorString;
        setStatus(tr("Error while loading exchange rates."));
    });
}

void RateGraph::setRates(QString reply)
{
    currencyRates.clear();

    QJsonDocument jsonResponse = QJsonDocument::fromJson(reply.toUtf8());
    if(jsonResponse.isEmpty()){
        qDebug()<<"API:"<<"Empty response returned from API call. Please report to developer.";
        return;
    }
    QJsonObject rateObj = jsonResponse.object().value("rates").toObject();
    foreach (const QString &key, rateObj.keys()) {
        auto rateValue = rateObj.value(key).toObject().value(ui->currencyComboBox->currentText()).toDouble(0.00);
        currencyRates.push_front(qMakePair(QDate::fromString(key,Qt::ISODate), rateValue));
    }
    if(!currencyRates.isEmpty())
        ui->plotWidget->initGraph(currencyRates);
    setStatus(tr("Loaded exchange rates"));
}

void RateGraph::on_updatePushBtn_clicked()
{
    auto fromdate = ui->fromDateEdit->date().toString("yyyy-M-dd");
    auto todate   = ui->toDateEdit->date().toString("yyyy-M-dd");

    QUrl base_url("https://api.exchangerate.host/timeseries");
    QUrlQuery query;
    query.addQueryItem("start_date",fromdate);
    query.addQueryItem("end_date",todate);
    query.addQueryItem("symbols",ui->currencyComboBox->currentText());
    query.addQueryItem("base","USD");
//    query.addQueryItem("places","2");
    base_url.setQuery(query);
    qDebug()<<base_url;
    _currentUrl = base_url;
    _request->get(_currentUrl);
    setStatus(tr("Loading exchange rates..."));
}

void RateGraph::setStatus(QString message)
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    ui->status->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(500);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    ui->status->setText(message);
    ui->status->show();
}
