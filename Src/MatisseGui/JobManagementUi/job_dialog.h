#ifndef JOBDIALOG_H
#define JOBDIALOG_H

#include <QFileInfo>
#include <QDialog>
#include <QDir>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QFileDialog>

#include <QtDebug>

#include "key_value_list.h"
#include "parameters_common.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"


namespace Ui {
class JobDialog;
}

namespace matisse {

class JobDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JobDialog(QWidget *parent,  MatisseIconFactory *iconFactory, KeyValueList * keyValues, QString jobsPath, QStringList existingJobNames, QStringList archivedJobNames);
    ~JobDialog();

//    static QString newJobName(QWidget * parent = NULL, KeyValueList * keyValues = NULL, QString jobsPath = QString());

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::JobDialog *_ui;
    KeyValueList * _keyValues;
    bool _isRealTime;
    QString _jobsPath;
    QStringList _existingJobNames;
    QStringList _archivedJobNames;

public slots:
    void slot_formatName(QString text);
    void slot_close();
    void slot_selectDir();
    void slot_selectFile();
};

} // namespace matisse

#endif // JOBDIALOG_H
