#ifndef HISTORY_REPOSITORY_H
#define HISTORY_REPOSITORY_H

#include <QDateTime>
#include <QString>
#include <QVector>

struct HistorySample
{
    QDateTime timestamp;
    int soc = 0;
};

class HistoryRepository
{
public:
    explicit HistoryRepository(const QString &filePath);

    bool addSample(const QDateTime &timestamp, int soc);
    QVector<HistorySample> samples(const QDateTime &from,
                                   const QDateTime &to) const;

private:
    QString m_filePath;
    QVector<HistorySample> load() const;
    bool save(const QVector<HistorySample> &samples) const;
    static QVector<HistorySample> retainRecent(QVector<HistorySample> samples,
                                               const QDateTime &now);
};

#endif
