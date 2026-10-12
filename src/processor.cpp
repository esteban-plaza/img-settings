#include "processor.h"

#include "watermark.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

using namespace Qt::StringLiterals;

Processor::Processor(QObject *parent)
    : QObject(parent)
{
    m_pool.setMaxThreadCount(QThread::idealThreadCount());
}

Processor::~Processor()
{
    if (m_cancelled)
        *m_cancelled = true;
    m_pool.waitForDone();
}

int Processor::process(const QList<QUrl> &urls, bool recurse, double opacity, const QString &customOutDir)
{
    if (m_busy)
        return 0;

    QStringList paths;
    for (const QUrl &url : urls) {
        const QString path = pathFromUrl(url);
        if (!path.isEmpty())
            paths.append(path);
    }
    if (paths.isEmpty())
        return 0;

    QString outDir = customOutDir;
    if (outDir.isEmpty()) {
        const QFileInfo first(paths.first());
        outDir = QDir(first.isDir() ? first.absoluteFilePath() : first.absolutePath()).filePath(u"out"_s);
    }
    outDir = QDir::cleanPath(outDir);

    const QStringList files = Watermark::collectFiles(paths, recurse, outDir);
    if (files.isEmpty())
        return 0;

    if (!QDir().mkpath(outDir)) {
        emit problem(tr("Couldn't create the output folder %1").arg(displayName(outDir)));
        return -1;
    }

    opacity = qBound(0.0, opacity, 1.0);
    m_outDir = outDir;
    emit outDirChanged();

    ++m_generation;
    m_total = int(files.size());
    m_completed = m_failed = m_reported = 0;
    m_cancelled = std::make_shared<std::atomic_bool>(false);
    emit progressChanged();
    setBusy(true);

    const int generation = m_generation;
    const auto cancelled = m_cancelled;
    for (const QString &file : files) {
        m_pool.start([this, generation, cancelled, file, outDir, opacity] {
            const bool skip = cancelled->load();
            const QString error = skip ? QString() : Watermark::processFile(file, outDir, opacity);
            QMetaObject::invokeMethod(this, [this, generation, file, error, skip] {
                onFileDone(generation, file, error, skip);
            }, Qt::QueuedConnection);
        });
    }
    return m_total;
}

QStringList Processor::formats() const
{
    return Watermark::supportedFormats();
}

void Processor::cancel()
{
    if (m_busy && m_cancelled)
        *m_cancelled = true;
}

void Processor::onFileDone(int generation, const QString &input, const QString &error, bool skipped)
{
    if (generation != m_generation)
        return;

    ++m_reported;
    if (!skipped) {
        ++m_completed;
        if (!error.isEmpty())
            ++m_failed;
        const QString output = error.isEmpty() ? Watermark::outputPath(input, m_outDir) : QString();
        emit progressChanged();
        emit fileFinished(QFileInfo(input).fileName(), output,
                          output.isEmpty() ? QUrl() : QUrl::fromLocalFile(output),
                          output.isEmpty() ? 0 : QFileInfo(output).size(), error);
    }

    if (m_reported == m_total) {
        setBusy(false);
        emit finished(m_cancelled->load());
    }
}

void Processor::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void Processor::reveal(const QString &path) const
{
#if defined(Q_OS_MACOS)
    QProcess::startDetached(u"/usr/bin/open"_s, { path });
#elif defined(Q_OS_WIN)
    QProcess::startDetached(u"explorer"_s, { QDir::toNativeSeparators(path) });
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
#endif
}

void Processor::openUrl(const QUrl &url) const
{
    QDesktopServices::openUrl(url);
}

QString Processor::pathFromUrl(const QUrl &url) const
{
    return url.isLocalFile() ? QDir::cleanPath(url.toLocalFile()) : QString();
}

QString Processor::displayName(const QString &path) const
{
    QString shown = QDir::toNativeSeparators(path);
    const QString home = QDir::toNativeSeparators(QDir::homePath());
    if (shown.startsWith(home))
        shown.replace(0, home.size(), u"~"_s);
    return shown;
}
