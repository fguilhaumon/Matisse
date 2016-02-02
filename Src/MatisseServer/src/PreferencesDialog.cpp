#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent, MatissePreferences *prefs) :
    QDialog(parent),
    _ui(new Ui::PreferencesDialog)
{
    _ui->setupUi(this);

    _ui->_CB_languageSelect->addItem("FR");
    _ui->_CB_languageSelect->addItem("EN");

    _prefs = prefs;

    _ui->_LE_importExportPath->setText(_prefs->importExportPath());
    _ui->_LE_archivePath->setText(_prefs->archivePath());
    _ui->_LE_defaultResultPath->setText(_prefs->defaultResultPath());
    _ui->_LE_defaultMosaicPrefix->setText(_prefs->defaultMosaicFilenamePrefix());
    _ui->_CK_enableProgrammingMode->setChecked(_prefs->programmingModeEnabled());

    if (_prefs->language() == "FR") {
        _ui->_CB_languageSelect->setCurrentIndex(0);
    } else { // EN
        _ui->_CB_languageSelect->setCurrentIndex(1);
    }

    // saving mosaic file prefix buffer
    _prefixBuffer = _prefs->defaultMosaicFilenamePrefix();

    _ui->_LE_importExportPath->setReadOnly(true);
    _ui->_LE_archivePath->setReadOnly(true);
    _ui->_LE_defaultResultPath->setReadOnly(true);

    // on limite la saisie aux caractères alphanumériques + '-' et '_'
    QRegExpValidator *prefixVal = new QRegExpValidator(QRegExp("[a-zA-Z\\d_\\-]{2,}"), this);
    _ui->_LE_defaultMosaicPrefix->setValidator(prefixVal);

    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_importExportPathSelect, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_archivePathSelect, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_defaultResultPathSelect, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_LE_defaultMosaicPrefix, SIGNAL(editingFinished()), this, SLOT(slot_validatePrefixInput()));
    //connect(_ui->_LE_defaultMosaicPrefix, SIGNAL(textEdited(QString)), this, SLOT(slot_restorePrefixInput(QString)));
}

PreferencesDialog::~PreferencesDialog()
{
    delete _ui;
}

void PreferencesDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
    }
}

void PreferencesDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        QString newMosaicPrefix = _ui->_LE_defaultMosaicPrefix->text();
        if (newMosaicPrefix != _prefixBuffer) {
            qDebug() << QString("Default mosaic prefix input '%1' is in intermediate validation state, restoring previous : '%2'").arg(newMosaicPrefix, _prefixBuffer);
            newMosaicPrefix = _prefixBuffer;
        }

        _prefs->setImportExportPath(_ui->_LE_importExportPath->text());
        _prefs->setArchivePath(_ui->_LE_archivePath->text());
        _prefs->setDefaultResultPath(_ui->_LE_defaultResultPath->text());
        _prefs->setDefaultMosaicFilenamePrefix(newMosaicPrefix);
        _prefs->setProgrammingModeEnabled(_ui->_CK_enableProgrammingMode->isChecked());
        _prefs->setLanguage(_ui->_CB_languageSelect->currentText());

        accept();
    }
}

void PreferencesDialog::slot_selectDir()
{
    QString selDir = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()));

    if (selDir.isEmpty()) {
        return;
    }

    QFileInfo dirInfo(selDir);
    QString dirPath = dirInfo.filePath();

    QDir current = QDir::current();
    if (dirPath.startsWith(current.path())) {
        // chemin relatif
        dirPath = current.relativeFilePath(dirPath);
        if (dirPath.isEmpty()) {
            dirPath = ".";
        }

    }

    if (sender() == _ui->_PB_importExportPathSelect) {
        _ui->_LE_importExportPath->setText(dirPath);
    } else if (sender() == _ui->_PB_archivePathSelect) {
        _ui->_LE_archivePath->setText(dirPath);
    } else if (sender() == _ui->_PB_defaultResultPathSelect) {
        _ui->_LE_defaultResultPath->setText(dirPath);
    }
}

void PreferencesDialog::slot_validatePrefixInput()
{
    _prefixBuffer = _ui->_LE_defaultMosaicPrefix->text();
}


// Ne marche pas car appelé à chaque saisie de caractère
// Il faudrait traquer la sortie de focus en surchargeant QLineEdit

//void PreferencesDialog::slot_restorePrefixInput(QString newText)
//{
//    int cursor = newText.size();

//    QValidator::State valState = _ui->_LE_defaultMosaicPrefix->validator()->validate(newText, cursor);

//    if (valState != QValidator::Acceptable) {
//        qDebug() << QString("Default mosaic prefix input '%1' is in intermediate state, restoring previous : '%2'").arg(newText, _prefixBuffer);
//        _ui->_LE_defaultMosaicPrefix->setText(_prefixBuffer);
//    }
//}
