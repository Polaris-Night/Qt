#ifndef FILESPLITER_H
#define FILESPLITER_H

#include <QString>
#include <QVector>
#include <QJsonObject>

#include "message.h"

struct FileBlock : public SerializeApi {
    QString fileName;
    qint64 fileSize{};
    qint32 blockCount{};        // 分块总数
    qint32 blockIndex{};        // 当前块编号
    qint64 blockStartAddress{}; // 当前块偏移地址
    qint64 blockSize{};         // 当前块大小

    void toJson(QJsonObject &j) const override;
    void fromJson(const QJsonObject &j) override;

    QByteArray toJson() const override;
    void fromJson(const QByteArray &j) override;;
};

class FileSpliter {
public:
    static QVector<FileBlock> split(const QString &fileName, int blockCount);
};

#endif // FILESPLITER_H
