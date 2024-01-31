#include <QProcess>
#include <QDebug>

#include "update.h"

const static QString s_dbusName = "com.lingmo.Session";
const static QString s_pathName = "/Session";
const static QString s_interfaceName = "com.lingmo.Session";

const QString LocaVer = "https://packages.lingmo.org/release/update/user/2.0/2.0.0/lover.upkginf";
const QString pkgUrl = "https://packages.lingmo.org/release/update/user/2.0/update.deb"
const QString DownSavePath = "/opt/update/";
const QString Logurl = "https://packages.lingmo.org/release/update/user/2.0/changelog.upkginf";

static QString formatByteSize(double size, int precision)
{
    int unit = 0;
    double multiplier = 1024.0;

    while (qAbs(size) >= multiplier && unit < int(8)) {
        size /= multiplier;
        ++unit;
    }

    if (unit == 0) {
        precision = 0;
    }

    QString numString = QString::number(size, 'f', precision);

    switch (unit) {
    case 0:
        return QString("%1 B").arg(numString);
    case 1:
        return QString("%1 KB").arg(numString);
    case 2:
        return QString("%1 MB").arg(numString);
    case 3:
        return QString("%1 GB").arg(numString);
    case 4:
        return QString("%1 TB").arg(numString);
    case 5:
        return QString("%1 PB").arg(numString);
    case 6:
        return QString("%1 EB").arg(numString);
    case 7:
        return QString("%1 ZB").arg(numString);
    case 8:
        return QString("%1 YB").arg(numString);
    default:
        return QString();
    }

    return QString();
}

UpdateSys::UpdateSys(const QString& DownSavePath, const QString& url, QObject *parent) :
    : QObject(parent)
{
    m_savepath = DownSavePath;

    QSettings settings("/etc/os-release",QSettings::IniFormat);
    m_currentVersion = settings.value("PRETTY_NAME").toString();
}

UpdateSys::~UpdateSys() 
{
}

bool UpdateSys::isLingmoOS()
{
    if (!QFile::exists("/etc/lingmoos"))
        return false;

    QSettings settings("/etc/lingmoos", QSettings::IniFormat);
    return settings.value("LingmoOS", false).toBool();
}

QString UpdateSys::GetLocalVersion()
{
    QSettings settings("/etc/os-release",QSettings::IniFormat);
    return settings.value("BUILDID").toString();
}

void UpdateSys::getUpdateInfo()
{
    if (!LocaVer.isValid()) {
        return false;
    }

    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finished(int)));
    process->start("wget", QStringList() << "-O " << DownSavePath << LocaVer);
    process.waitForFinished();

    QByteArray logd = process.readAllStandardOutput();

    qDebug() << logd;

    QString UpdateSys::GetWEBlVersion()
    {
        QSettings settings("/opt/update/lover.upkginf",QSettings::IniFormat);
        return settings.value("BUILDID").toString();
    }

    if (GetWEBlVersion > GetLocalVersion)
    {
        QProcess *process = new QProcess(this);
        process->start("wget", QStringList() << "-O " << DownSavePath << Logurl);
        emit haneupdate();
    } else {
        emit noupdate();
    }
}

void UpdateSys::DownloadPkg()
{
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finished(int)));
    process->start("wget", QStringList() << "-O " << DownSavePath << pkgUrl);
    process.waitForFinished();
    emit downloaddown();
}

void UpdateSys::installPkg()
{
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finished(int)));
    process->start("apt", QStringList() << "-y install " << DownSavePath << "update.deb");
    process.waitForFinished();
    emit installDone();
}

void UpdateSys::rebootSys()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("reboot");
    }
}

void UpdateSys::version()
{
    return m_currentVersion;
}

QString UpdateSys::changeLog()
{
    QSettings settings("/opt/update/changelog.upkginf",QSettings::IniFormat);
    return settings.value("changelog").toString();
}