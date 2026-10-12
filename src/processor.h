#pragma once

#include <QObject>
#include <QThreadPool>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

#include <atomic>
#include <memory>

// Runs the watermark pipeline over dropped files on a thread pool and reports
// each result back to QML.
class Processor : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int total READ total NOTIFY progressChanged)
    Q_PROPERTY(int completed READ completed NOTIFY progressChanged)
    Q_PROPERTY(int failed READ failed NOTIFY progressChanged)
    Q_PROPERTY(QString outDir READ outDir NOTIFY outDirChanged)
    Q_PROPERTY(QStringList formats READ formats CONSTANT)

public:
    explicit Processor(QObject *parent = nullptr);
    ~Processor() override;

    bool busy() const { return m_busy; }
    int total() const { return m_total; }
    int completed() const { return m_completed; }
    int failed() const { return m_failed; }
    QString outDir() const { return m_outDir; }
    QStringList formats() const;

    // Starts processing. An empty customOutDir means "<folder of first item>/out".
    // Returns the number of photos queued, 0 when nothing supported was found,
    // or -1 when the output folder can't be created (problem() is emitted).
    Q_INVOKABLE int process(const QList<QUrl> &urls, bool recurse, double opacity,
                            const QString &customOutDir);
    Q_INVOKABLE void cancel();

    Q_INVOKABLE void reveal(const QString &path) const;
    Q_INVOKABLE void openUrl(const QUrl &url) const;
    Q_INVOKABLE QString pathFromUrl(const QUrl &url) const;
    Q_INVOKABLE QString displayName(const QString &path) const;

signals:
    void busyChanged();
    void progressChanged();
    void outDirChanged();
    void fileFinished(const QString &name, const QString &output, const QUrl &outputUrl,
                      qint64 bytes, const QString &error);
    void finished(bool cancelled);
    void problem(const QString &message);

private:
    void onFileDone(int generation, const QString &input, const QString &error, bool skipped);
    void setBusy(bool busy);

    QThreadPool m_pool;
    std::shared_ptr<std::atomic_bool> m_cancelled;
    int m_generation = 0;
    bool m_busy = false;
    int m_total = 0;
    int m_completed = 0;
    int m_failed = 0;
    int m_reported = 0;
    QString m_outDir;
};
