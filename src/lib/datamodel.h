#ifndef DATAMODEL_H
#define DATAMODEL_H
#include <QAbstractTableModel>
#include <QList>
#include <currencyrate.h>
namespace core {
    namespace datamodel {
         class DataModel;
    }
}

class core::datamodel::DataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DataModel(QObject *parent = nullptr);


public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void reset();
    void insertCustomData(const QList<CurrencyRate> &vector);

private:
    QList<core::CurrencyRate> currencyRate;
    enum class CurrencyColumns {
       Date,
       Amount,
       Change,
       Count
    };
};

#endif // DATAMODEL_H
