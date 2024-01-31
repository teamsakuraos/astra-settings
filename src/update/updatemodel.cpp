#include "updatemodel.h"

const QString baseUrl = "https://packages.lingmo.org"
const QString updateUrl = "https://packages.lingmo.org/release/update/user/2.0/changelog.json"
const QString pkgUrl = "https://packages.lingmo.org/release/update/user/2.0/update.upkgimg"

UpdateModel::UpdateModel(const QString& downloadUrl, const QString& savePath, QObject* parent)
    : QObject(parent)
{
    m_downloadUrl = downloadUrl;
    m_savePath = savePath;
//    m_backend->init();
}

UpdateModel::~UpdateModel() {}

void UpdateModel::startDownloadVer()
{
    const QUrl url = QUrl(m_downloadUrl);

    if (!url.isValid()) {
        return false;
    }

    QString fileName = url.fileName();
    if (fileName.isEmpty()) fileName = defaultFileName;
    if (m_savePath.isEmpty()) { m_savePath = QApplication::applicationDirPath() + "/tmp"; }
    if (!QFileInfo(m_savePath).isDir()) {
        QDir dir;
        dir.mkpath(m_savePath);
    }

    fileName.prepend(m_savePath + '/');
    if (QFile::exists(fileName)) { QFile::remove(fileName); }
    file = openFileForWrite(fileName);
    if (!file) return;

    startRequest(url);
}

void UpdateModel::httpFinished()
{
    QFileInfo fi;
    if (file) {
        fi.setFile(file->fileName());
        file->close();
        file.reset();
    }
 
    if (httpRequestAborted) {
        return;
    }
 
    if (reply->error()) {
        QFile::remove(fi.absoluteFilePath());
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Download failed: %1.").arg(reply->errorString());
#endif // DOWNLOAD_DEBUG 
        return;
    }
    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());
        file = openFileForWrite(fi.absoluteFilePath());
        if (!file) { return; }
        startRequest(redirectedUrl);
        return;
    }
    Q_EMIT sigDownloadFinished();

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString(tr("Downloaded %1 bytes to %2 in %3")
        .arg(fi.size()).arg(fi.fileName(), QDir::toNativeSeparators(fi.absolutePath())));
    qDebug() << "Finished";
#endif // DOWNLOAD_DEBUG 
}

void UpdateModel::httpReadyRead()
{
    if (file) file->write(reply->readAll());
}

void UpdateModel::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    qreal progress = qreal(bytesRead) / qreal(totalBytes);
    Q_EMIT sigProgress(bytesRead, totalBytes, progress);

#ifdef DOWNLOAD_DEBUG
    qDebug() << QString::number(progress * 100, 'f', 2) << "%    "
        << bytesRead / (1024 * 1024) << "MB" << "/" << totalBytes / (1024 * 1024) << "MB";
#endif // DOWNLOAD_DEBUG
}

void UpdateModel::startRequest(const QUrl& requestedUrl)
{
     url = requestedUrl;
    httpRequestAborted = false;
 
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &DownloadTool::httpFinished);
    connect(reply, &QIODevice::readyRead, this, &DownloadTool::httpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadTool::networkReplyProgress);
 
#ifdef DOWNLOAD_DEBUG
    qDebug() << QString(tr("Downloading %1...").arg(url.toString()));
#endif // DOWNLOAD_DEBUG
}

std::unique_ptr<QFile> UpdateModel::openFileForWrite(const QString& fileName)
{
    std::unique_ptr<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly)) {
#ifdef DOWNLOAD_DEBUG
        qDebug() << QString("Unable to save the file %1: %2.")
            .arg(QDir::toNativeSeparators(fileName), file->errorString());
#endif // DOWNLOAD_DEBUG  
        return nullptr;
    }
    return file;
}


bool UpdateModel::CheckUpdate()
{
    if (!QFile::exists("/opt/update/update.upkgimg"))
    return false;

    QSettings settins("/opt/update/log/changelog.json", QSettings::IniFormat)
    return settings.value("CHANGE").toString();
}

void UpdateModel::DownloadUpdate()
{
    QProcess process;
    process.start("wget", QStringList() << updateUrl << "-O" << "/opt/update/update.upkgimg");
    QByteArray poss = process.readAllStandardOutput();

    qDebug() << result_;
}

void UpdateModel::onTransactionStatusChanged(QApt::TransactionStatus status)
{
}
