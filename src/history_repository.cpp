#include "history_repository.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr qint64 HistoryWindowSeconds = 24 * 60 * 60;
}

HistoryRepository::HistoryRepository(const QString &filePath)
    : m_filePath(filePath)
{
}

bool HistoryRepository::addSample(const QDateTime &timestamp, int soc)
{
    if (!timestamp.isValid() || soc < 0 || soc > 100) {
        return false;
    }

    QVector<HistorySample> history = retainRecent(load(), timestamp);
    history.append({timestamp.toUTC(), soc});
    return save(history);
}

QVector<HistorySample> HistoryRepository::samples(const QDateTime &from,
                                                  const QDateTime &to) const
{
    const QVector<HistorySample> history = retainRecent(load(), to);
    QVector<HistorySample> result;
    for (const HistorySample &sample : history) {
        if (sample.timestamp >= from && sample.timestamp <= to) {
            result.append(sample);
        }
    }
    return result;
}

QVector<HistorySample> HistoryRepository::load() const
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray()) {
        return {};
    }

    QVector<HistorySample> result;
    for (const QJsonValue &value : document.array()) {
        const QJsonObject object = value.toObject();
        const QDateTime timestamp = QDateTime::fromString(
            object.value("timestamp").toString(), Qt::ISODateWithMs);
        const int soc = object.value("soc").toInt(-1);
        if (timestamp.isValid() && soc >= 0 && soc <= 100) {
            result.append({timestamp.toUTC(), soc});
        }
    }
    return result;
}

bool HistoryRepository::save(const QVector<HistorySample> &samples) const
{
    QJsonArray array;
    for (const HistorySample &sample : samples) {
        array.append(QJsonObject{{"timestamp", sample.timestamp.toUTC().toString(Qt::ISODateWithMs)},
                                 {"soc", sample.soc}});
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(QJsonDocument(array).toJson(QJsonDocument::Compact)) >= 0;
}

QVector<HistorySample> HistoryRepository::retainRecent(QVector<HistorySample> samples,
                                                       const QDateTime &now)
{
    const QDateTime cutoff = now.toUTC().addSecs(-HistoryWindowSeconds);
    QVector<HistorySample> retained;
    for (const HistorySample &sample : samples) {
        if (sample.timestamp >= cutoff && sample.timestamp <= now.toUTC()) {
            retained.append(sample);
        }
    }
    return retained;
}
