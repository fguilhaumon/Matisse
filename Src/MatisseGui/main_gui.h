﻿#ifndef ASSEMBLYGUI_H
#define ASSEMBLYGUI_H

#include <QMainWindow>
#include <QModelIndex>
#include <QResizeEvent>
#include <QGraphicsScene>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QPair>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QToolButton>
#include <QMenuBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QDesktopServices>
#include <QRegExp>

#include <QtDebug>

#include "element_widget_provider.h"
#include "assembly_graphics_scene.h"
#include "key_value_list.h"
#include "matisse_engine.h"
#include "graphical_charter.h"
#include "assembly_dialog.h"
#include "job_dialog.h"
#include "preferences_dialog.h"
#include "duplicate_dialog.h"
#include "restore_jobs_dialog.h"
#include "data_viewer.h"
#include "assembly_editor.h"
#include "dim2_file_reader.h"
#include "status_message_widget.h"
#include "matisse_preferences.h"
#include "home_widget.h"
#include "matisse_menu.h"
#include "live_process_wheel.h"
#include "about_dialog.h"
#include "system_data_manager.h"
#include "process_data_manager.h"
#include "platform_comparison_status.h"
#include "string_utils.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"
#include "iconized_label_wrapper.h"
#include "iconized_tree_item_wrapper.h"
#include "matisse_tree_item.h"
#include "welcome_dialog.h"
#include "camera_manager_tool.h"
#include "camera_calib_dialog.h"
#include "remote_job_helper.h"


namespace Ui {
class MainGui;
}

namespace MatisseServer {

enum MessageIndicatorLevel {
        IDLE,
        OK,
        WARNING,
        ERROR
};


enum UserAction {
    SYSTEM_INIT,
    SWAP_VIEW,
    CHANGE_APP_MODE,
    CREATE_ASSEMBLY,
    MODIFY_ASSEMBLY,
    SAVE_ASSEMBLY,
    SAVE_JOB,
    SELECT_ASSEMBLY,
    SELECT_JOB,
    RUN_JOB,
    JOB_COMPLETE,
    STOP_JOB
};

class UserActionContext {
public:
    UserActionContext() :
        _lastActionPerformed(SYSTEM_INIT) {}
    UserAction lastActionPerformed() const { return _lastActionPerformed; }
    void setLastActionPerformed(UserAction lastActionPerformed) {
        qDebug() << "Last action performed : " << lastActionPerformed;
        _lastActionPerformed = lastActionPerformed;
    }

private:
    UserAction _lastActionPerformed;
};

class MainGui : public QMainWindow, ElementWidgetProvider
{
    Q_OBJECT
    
public:
    explicit MainGui(QWidget *parent = 0);
    ~MainGui();
//    bool setSettingsFile(QString settings);
//    bool isShowable();
    void init();
    void loadDefaultStyleSheet();

    virtual SourceWidget * getSourceWidget(QString name);
    virtual ProcessorWidget * getProcessorWidget(QString name);
    virtual DestinationWidget * getDestinationWidget(QString name);

    void applyNewApplicationContext();
    void handleAssemblyModified();
    void checkAndSelectAssembly(QString selectedAssemblyName);
    void checkAndSelectJob(QTreeWidgetItem* selectedItem);
    void resetOngoingProcessIndicators();
    void updatePreferredDatasetParameters();

    void setRemoteJobHelper(RemoteJobHelper *remoteJobHelper);

    void initMapFeatures();
private:
    Ui::MainGui *_ui;
    bool _isMapView;
    MatisseEngine m_engine;
//    bool _canShow;
    UserActionContext _context;

    RemoteJobHelper *m_remote_job_helper;

    QString _appVersion;

    QString _exportPath;
    QString _importPath;
    QString _archivePath;
    QString m_remote_output_path;

    MatissePreferences* _preferences;
    QTranslator* _toolsTranslator_en;
    QTranslator* _toolsTranslator_fr;
    QTranslator* _serverTranslator_en;
    QTranslator* _serverTranslator_fr;
    QString _currentLanguage;

    bool _jobParameterModified;
    bool _isAssemblyModified;
    bool _isAssemblyComplete;

    static const QString PREFERENCES_FILEPATH;
    static const QString ASSEMBLY_EXPORT_PREFIX;
    static const QString JOB_EXPORT_PREFIX;
    static const QString JOB_REMOTE_PREFIX;
    static const QString DEFAULT_EXCHANGE_PATH;
    static const QString DEFAULT_ARCHIVE_PATH;
    static const QString DEFAULT_REMOTE_PATH;
    static const QString DEFAULT_RESULT_PATH;
    static const QString DEFAULT_MOSAIC_PREFIX;

    QTreeWidgetItem * _lastJobLaunchedItem;
    AssemblyDefinition *_newAssembly;
    AssemblyDefinition *_currentAssembly;
    JobDefinition *_currentJob;
    DataViewer * _userFormWidget;
    AssemblyEditor * _expertFormWidget;
    QScrollArea * _parametersDock;
    ParametersWidgetSkeleton * _parametersWidget;
    QLabel* _messagesPicto;

    QString m_current_remote_execution_bundle;

    QTreeWidgetItem *_assemblyVersionPropertyItem;
    QTreeWidgetItem *_assemblyCreationDatePropertyItem;
    QTreeWidgetItem *_assemblyAuthorPropertyItem;
    QTreeWidgetItem *_assemblyCommentPropertyHeaderItem;
    QTreeWidgetItem *_assemblyCommentPropertyItem;
    QLabel *_assemblyCommentPropertyItemText;

    ApplicationMode _activeApplicationMode;
    QHash<QString, QTreeWidgetItem*> _assembliesItems;
    QHash<QString, KeyValueList*> _assembliesProperties;
    QMap<ApplicationMode, QString> _stylesheetByAppMode;
    QMap<ApplicationMode, QString> _wheelColorsByMode;
    QMap<ApplicationMode, QString> _colorsByMode1;
    QMap<ApplicationMode, QString> _colorsByMode2;
    QMap<MessageIndicatorLevel, QString> _colorsByLevel;
    QToolButton* _visuModeButton;
    QToolButton* _stopButton;
    QToolButton* _maximizeOrRestoreButton;
    QToolButton* _closeButton;
    QToolButton* _minimizeButton;
    QToolButton* _homeButton;
    QPushButton* _resetMessagesButton;
    HomeWidget *_homeWidget;
    WelcomeDialog *_welcomeDialog;

    CameraManagerTool m_camera_manager_tool_dialog;
    CameraCalibDialog m_camera_calib_tool_dialog;

    bool _isNightDisplayMode;
    QMap<QString, QString> _currentColorSet;

    QLabel *_activeViewOrModeLabel;
    QLabel *_currentDateTimeLabel;
    QTimer *_dateTimeTimer;
    QLabel *_ongoingProcessInfolabel;
    QLabel *_matisseVersionlabel;
    QProgressBar *_ongoingProcessCompletion;
    LiveProcessWheel *_liveProcessWheel;

    // status bar
    StatusMessageWidget* _statusMessageWidget;

    QHash<QString, SourceWidget *> _availableSources;
    QHash<QString, ProcessorWidget *> _availableProcessors;
    QHash<QString, DestinationWidget *> _availableDestinations;

    MatisseIconFactory *_iconFactory;
    IconizedWidgetWrapper *_maxOrRestoreButtonWrapper;
    IconizedWidgetWrapper *_visuModeButtonWrapper;

    /* static menu headers */
    MatisseMenu *_fileMenu;
    MatisseMenu *_displayMenu;
    MatisseMenu *_processMenu;
    MatisseMenu *_toolMenu;
    MatisseMenu *_helpMenu;

    /* static menu actions */
    QAction* _exportMapViewAct;
    QAction* _closeAct;
    QAction* _dayNightModeAct;
    QAction* _mapToolbarAct;
    QAction* _createAssemblyAct;
    QAction* _saveAssemblyAct;
    QAction* _importAssemblyAct;
    QAction* _exportAssemblyAct;
    QAction* _appConfigAct;
    QAction* _preprocessingTool;
    QAction* m_camera_manager_tool;
    QAction* m_camera_calib_tool;
    QAction* _videoToImageToolAct;
    QAction* _userManualAct;
    QAction* _aboutAct;

    /* assembly context menu */
    QAction* _createJobAct;
    QAction* _importJobAct;
    QAction* _deleteAssemblyAct;
    QAction* _restoreJobAct;
    QAction* _cloneAssemblyAct;
    QAction* _updateAssemblyPropertiesAct;

    /* job context menu */
    QAction* _executeJobAct;
    QAction* m_execute_remote_job_act;
    QAction* m_upload_data_act;
    QAction* m_select_remote_data_act;
    QAction* m_download_job_results_act;
    QAction* _saveJobAct;
    QAction* _cloneJobAct;
    QAction* _exportJobAct;
    QAction* _deleteJobAct;
    QAction* _archiveJobAct;
    QAction* _goToResultsAct;

private:
    void dpiScaleWidgets();
    void initMainMenu();
    void initIconFactory();
    void initStylesheetSelection();
    void initContextMenus();
    void enableActions();
    void initDateTimeDisplay();
    void initPreferences();
    void initVersionDisplay();
    void loadAssemblyParameters(AssemblyDefinition *selectedAssembly);
    void initParametersWidget();
    void initProcessorWidgets();
    void lookupChildWidgets();
    void initProcessWheelSignalling();
    void initUserActions();
    void initServer();
    void initRemoteJobHelper();
    void initAssemblyCreationScene();
    void initWelcomeDialog();
    //bool getAssemblyValues(QString filename, QString  name, bool &valid, KeyValueList & assemblyValues);
    void loadAssembliesAndJobsLists(bool doExpand=true);
    void displayAssembly(QString assemblyName);
    void displayJob(QString jobName, bool forceReload = false);
    void selectJob(QString jobName, bool reloadJob = true);
    void selectAssembly(QString assemblyName, bool reloadAssembly = true);
    void showError(QString title, QString message);
    QTreeWidgetItem * addAssemblyInTree(AssemblyDefinition *assembly);
    QTreeWidgetItem * addJobInTree(JobDefinition *job, bool isNewJob = false);
    void selectItem(QTreeWidget wid, QString itemText);

    void loadStyleSheet(ApplicationMode mode);

    void saveAssemblyAndReload(AssemblyDefinition *assembly);
    void displayAssemblyProperties(AssemblyDefinition *selectedAssembly);

    void initStatusBar();
    void showStatusMessage(QString message = "", MessageIndicatorLevel level = IDLE);

    void initLanguages();
    void updateLanguage(QString language, bool forceRetranslation = false);
    void retranslate();
    
    bool loadResultToCartoView(QString resultFile_p, bool remove_previous_scenes=true);
    
    void doFoldUnfoldParameters(bool doUnfold, bool isExplicitAction = false);

    void freezeJobUserAction(bool freeze_p);

    void handleJobModified();
    QString getActualAssemblyOrJobName(QTreeWidgetItem* currentItem);
    QString getActualNewAssemblyName();
    bool promptAssemblyNotSaved();
    void promptJobNotSaved();

    void deleteAssemblyAndReload(bool promptUser);

    void createExportDir();
    void createImportDir();
    void executeImportWorkflow(bool isJobImportAction = false);
    void executeExportWorkflow(bool isJobExportAction, bool isForRemoteExecution = false);
    void checkArchiveDirCreated();
    void checkRemoteDirCreated();
    bool checkArchivePathChange();

    void updateJobStatus(QString _job_name, QTreeWidgetItem* _item,
                         MessageIndicatorLevel _indicator, QString _message);

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

protected slots:
    void slot_saveAssembly();
    void slot_deleteAssembly();
    void slot_newJob();
    void slot_saveJob();
    void slot_deleteJob();
    void slot_assemblyContextMenuRequested(const QPoint &pos);

    void slot_maximizeOrRestore();
    void slot_quit();
    void slot_moveWindow(const QPoint &pos);

    void slot_clearAssembly();
    void slot_newAssembly();
    void slot_swapMapOrCreationView();
    void slot_launchJob();
    void sl_uploadJobData();
    void sl_selectRemoteJobData();
    void sl_launchRemoteJob();
    void sl_downloadJobResults();
    void sl_onRemoteJobResultsReceived(QString _job_name);
    void slot_stopJob();
    void slot_jobShowImageOnMainView(QString name, Image *image);
    void slot_userInformation(QString userText);
    void slot_processCompletion(quint8 percentComplete);
    void slot_showInformationMessage(QString title, QString message);
    void slot_showErrorMessage(QString title, QString message);
    void slot_jobProcessed(QString name, bool isCancelled);
    void slot_assembliesReload();
    void slot_modifiedParameters(bool changed);
    void slot_modifiedAssembly();
    void slot_assemblyComplete(bool isComplete);
    void slot_selectAssemblyOrJob(QTreeWidgetItem *selectedItem, int column=0);
    void slot_updateTimeDisplay();
    void slot_updatePreferences();
    void slot_foldUnfoldParameters();
    void slot_showUserManual();
    void slot_showAboutBox();
    void slot_exportAssembly();
    void slot_importAssembly();
    void slot_exportJob();
    void slot_importJob();
    void slot_goToResult();
    void slot_archiveJob();
    void slot_restoreJobs();
    void slot_duplicateJob();
    void slot_duplicateAssembly();
    void slot_swapDayNightDisplay();
    void slot_exportMapToImage();
    void slot_launchPreprocessingTool();
    void slot_launchCameraManagerTool();
    void slot_launchCameraCalibTool();

public slots:
    void slot_showApplicationMode(ApplicationMode mode);
    void slot_goHome();
    void slot_show3DFileOnMainView(QString filepath_p);
    void slot_addRasterFileToMap(QString filepath_p);
    void slot_addToLog(QString _loggin_text);

signals:
    void signal_processRunning();
    void signal_processStopped();
    void signal_processFrozen();
    void signal_updateWheelColors(QString colors);
    void signal_updateColorPalette(QMap<QString,QString>);
    void signal_updateExecutionStatusColor(QString newStatusColorAlias);
    void signal_updateAppModeColors(QString newAppModeColorAlias1, QString newAppModeColorAlias2);
};
}

#endif // ASSEMBLYGUI_H
