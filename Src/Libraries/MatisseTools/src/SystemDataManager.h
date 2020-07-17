#ifndef SYSTEMDATAMANAGER_H
#define SYSTEMDATAMANAGER_H

#include <QString>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QtXml>
#include <QXmlAttributes>
#include <QtDebug>

#include "MatissePreferences.h"
#include "PlatformInspector.h"
#include "PlatformDump.h"
#include "PlatformComparator.h"
#include "PlatformComparisonStatus.h"

namespace MatisseTools {

///
/// \brief The SystemDataManager class class is used for reading writing system data
///

class SystemDataManager
{
public:
    SystemDataManager(QString _bin_root_dir = ".");

    int port() const { return _port; }
    QString getUserDataPath() const { return _userDataPath; }
    QString getDllPath() const { return _dllPath; }
    QString getVersion() const { return _version; }
    QString getPlatformSummaryFilePath() const;
    QString getPlatformEnvDumpFilePath() const;
    QString getDataRootDir() const;
    QString getBinRootDir() const { return m_bin_root_dir; }
    QMap<QString, QString> getExternalTools() const;

    QString getDefaultRemoteServerAddress() const { return _defaultRemoteServerAddress; }
    QString getDefaultRemoteQueueName() const { return _defaultRemoteQueueName; }
    QString getDefaultRemoteDataPath() const { return _defaultRemoteDataPath; }
    QString getDefaultRemoteResultPath() const { return _defaultRemoteResultPath; }

    bool readMatisseSettings(QString filename);
    bool readMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writeMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writePlatformSummary();
    bool readRemotePlatformSummary(QString filename);
    bool writePlatformEnvDump();

    PlatformComparisonStatus *compareRemoteAndLocalPlatform();


private:
    void getPlatformDump();

    QString _userDataPath;
    QString _dataRootDir;
    QString m_bin_root_dir;
    QString _dllPath;
    QString _platformDumpPath;
    
    QString _defaultRemoteServerAddress;
    QString _defaultRemoteQueueName;
    QString _defaultRemoteDataPath;
    QString _defaultRemoteResultPath;
    
    int _port;
    QString _version;
    QString _platformSummaryFilePath;
    QString _platformEnvDumpFilePath;
    QMap<QString,QString> _externalTools;
    PlatformInspector _platformInspector;
    PlatformDump *_platformDump;
    PlatformDump *_remotePlatformDump;
    PlatformComparator _platformComparator;
};

}

#endif // SYSTEMDATAMANAGER_H
