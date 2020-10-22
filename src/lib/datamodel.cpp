#include "datamodel.h"
#include <QBrush>
#include <QDebug>

core::datamodel::DataModel::DataModel(QObject *parent) : QAbstractTableModel (parent)
{

}

int core::datamodel::DataModel::rowCount(const QModelIndex &) const
{
    return currencyRate.size();
}

int core::datamodel::DataModel::columnCount(const QModelIndex &) const
{
    return static_cast<int>(CurrencyColumns::Count);
}
QVariant core::datamodel::DataModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:

        switch (index.column()) {
        case 0:
            return  currencyRate[index.row()].first;
        case 1:
            return  currencyRate[index.row()].second;
        case 2:
            const QModelIndex currentValue = this->index(index.row(), 1);
            const QModelIndex prevValue = this->index(index.row()+1, 1);
            if(currentValue.isValid() && prevValue.isValid()){
                auto val = currentValue.data(Qt::DisplayRole).toDouble() -
                                        prevValue.data(Qt::DisplayRole).toDouble();
                //qDebug()<<QString::number(val,'f');
                return QString::number(val,'f',4);
            }
            else
                return "";
           }

    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;

    case Qt::BackgroundRole:
        if(index.row() % 2 == 0)
            return QBrush(QColor(245,245,245));
        else
            return QBrush(Qt::white);

    case Qt::TextColorRole:
        if(index.column() == 2) {
            if(index.data(Qt::DisplayRole).toDouble() > 0)
                return QColor(Qt::green);
            else
                return  QColor(Qt::red);
        }
        return QColor(Qt::black);

    default:
        return QVariant();
    }
}

QVariant core::datamodel::DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
        if(orientation == Qt::Horizontal) {
            switch (section) {
            case static_cast<int>(CurrencyColumns::Date):
                return QString(tr("Date"));
            case static_cast<int>(CurrencyColumns::Amount):
                return QString(tr("Rate"));
            case static_cast<int>(CurrencyColumns::Change):
                return QString("+/-");
            }
        }

    return QVariant();
}

Qt::ItemFlags core::datamodel::DataModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled;
}

void core::datamodel::DataModel::reset()
{
    beginResetModel();
    currencyRate.clear();
    endResetModel();
}

void core::datamodel::DataModel::insertCustomData(const QList<CurrencyRate>& list)
{
    reset();
    currencyRate = list;
}
