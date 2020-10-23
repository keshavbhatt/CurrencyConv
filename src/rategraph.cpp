#include "rategraph.h"
#include "ui_rategraph.h"


RateGraph::RateGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RateGraph)
{
    ui->setupUi(this);

    QSplitter *split1 = new QSplitter;
    split1->setObjectName("split1");
    split1->setOrientation(Qt::Horizontal);
    split1->addWidget(ui->plotViewWidget);
    split1->addWidget(ui->dataViewWidget);
    split1->setStretchFactor(0,2);
    split1->setStretchFactor(1,1);
    split1->setCollapsible(0,false);
    split1->setCollapsible(1,false);

    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->addWidget(split1);

    if(settings.value("geometry").isValid())
        restoreGeometry(settings.value("geometry").toByteArray());
    if(settings.value("splitPos").isValid())
        split1->restoreGeometry(settings.value("splitPos").toByteArray());
    if(settings.value("splitState").isValid())
        split1->restoreState(settings.value("splitState").toByteArray());

    ui->fromDateEdit->setDisplayFormat("dd-MMM-yyyy");
    ui->toDateEdit->setDisplayFormat("dd-MMM-yyyy");

    ui->fromDateEdit->setDateRange(QDate(1999,1,1),QDate::currentDate());
    ui->toDateEdit->setDateRange(QDate(1999,1,1),QDate::currentDate());

    ui->fromDateEdit->setDate(QDate::currentDate().addDays(-365));
    ui->toDateEdit->setDate(QDate::currentDate());

    dataModel = new core::datamodel::DataModel(this);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setModel(dataModel);


    setStatus(tr("idle"));

    connect(ui->plotWidget, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));

    load_currencies();

    init_loader();

    init_request();
}

void RateGraph::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

  qDebug()<<"clicked"<<dataIndex;
  // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
  // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
  ui->tableView->selectRow(dataIndex);
  ui->tableView->setCurrentIndex(ui->tableView->model()->index(2, 0));
  double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
  QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
  setStatus(message);
}

RateGraph::~RateGraph()
{
    delete ui;
}

void RateGraph::load_currencies()
{
   ui->currencyComboBox->blockSignals(true);
       QJsonDocument currenciesdoc    =  utils::loadJson(":/resources/currencies.json");
       QJsonObject resultsObj         =  currenciesdoc.object().value("symbols").toObject();
       foreach (const QString &key, resultsObj.keys()) {
           QString name = resultsObj.value(key).toObject().value("description").toString();
           QString id = resultsObj.value(key).toObject().value("code").toString();
           ui->currencyComboBox->addItem(id+" ("+name+")",QVariant(id));
       }
       //load last selected curr
       ui->currencyComboBox->setCurrentIndex(settings.value("currencyIndex",66).toInt());
   ui->currencyComboBox->blockSignals(false);
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
        _loader->setColor(QColor("#3DAEE9"));
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
            QString errorString = tr("Empty response returned from API.");
            setStatus(errorString);
            QMessageBox::information(this,tr("Error"),errorString);
        }else{
            setRates(reply);
        }
    });

    connect(_request,&Request::downloadError,[=](QString errorString)
    {
        _loader->stop();
        QMessageBox::information(this,tr("Error"),errorString);
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
        auto rateValue = rateObj.value(key).toObject().value(ui->currencyComboBox->currentData(Qt::UserRole).toString()).toDouble(0.00);
        currencyRates.push_front(qMakePair(QDate::fromString(key,Qt::ISODate), rateValue));
    }
    if(!currencyRates.isEmpty()){
        dataModel->insertCustomData(currencyRates);
        ui->plotWidget->initGraph(currencyRates);
    }
    setStatus(tr("Loaded exchange rates."));
}

void RateGraph::on_updatePushBtn_clicked()
{
    auto fromdate = ui->fromDateEdit->date().toString("yyyy-M-dd");
    auto todate   = ui->toDateEdit->date().toString("yyyy-M-dd");

    QUrl base_url("https://api.exchangerate.host/timeseries");
    QUrlQuery query;
    query.addQueryItem("start_date",fromdate);
    query.addQueryItem("end_date",todate);
    query.addQueryItem("symbols",ui->currencyComboBox->currentData(Qt::UserRole).toString());
    query.addQueryItem("base","USD");
    query.addQueryItem("places","4"); //NOTE:change this is datamodel too
    base_url.setQuery(query);
    //qDebug()<<base_url;
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

void RateGraph::on_currencyComboBox_currentIndexChanged(int index)
{
    settings.setValue("currencyIndex",index);
}

void RateGraph::closeEvent(QCloseEvent *ev)
{
    qDebug()<<"closed";
    settings.setValue("geometry",this->saveGeometry());
    QSplitter *split1 = this->findChild<QSplitter*>("split1");
    settings.setValue("splitPos",split1->saveGeometry());
    settings.setValue("splitState",split1->saveState());

    QWidget::closeEvent(ev);
}
