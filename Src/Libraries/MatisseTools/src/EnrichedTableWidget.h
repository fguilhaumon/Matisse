#ifndef ENRICHEDTABLEWIDGET_H
#define ENRICHEDTABLEWIDGET_H

#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QtDebug>

#include "EnrichedFormWidget.h"

namespace MatisseTools {

class EnrichedTableWidget : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedTableWidget(QWidget *parent, QString label, quint8 cols, quint8 rows, QStringList defaultValues);
    bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();
    virtual void setValue(QString newValue);

signals:

public slots:

private:
    QTableWidget * _table;
};
}

#endif // ENRICHEDTABLEWIDGET_H
