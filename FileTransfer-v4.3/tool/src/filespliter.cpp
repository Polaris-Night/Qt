#include "filespliter.h"
#include <QFileInfo>

void FileBlock::toJson(QJsonObject &j) const {
    j = QJsonObject{
        {"fileName", fileName},
        {"fileSize", fileSize},
        {"blockCount", blockCount},
        {"blockIndex", blockIndex},
        {"blockStartAddress", blockStartAddress},
        {"blockSize", blockSize},
    };
}

void FileBlock::fromJson(const QJsonObject &j) {
    fileName = j.value("fileName").toString();
    fileSize = j.value("fileSize").toVariant().toLongLong();
    blockCount = j.value("blockCount").toInt();
    blockIndex = j.value("blockIndex").toInt();
    blockStartAddress = j.value("blockStartAddress").toVariant().toLongLong();
    blockSize = j.value("blockSize").toVariant().toLongLong();
}

QByteArray FileBlock::toJson() const {
    QJsonObject obj;
    toJson(obj);
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void FileBlock::fromJson(const QByteArray &j) {
    QJsonParseError e;
    auto doc = QJsonDocument::fromJson(j, &e);
    if (e.error != QJsonParseError::NoError || doc.isNull()) {
        return;
    }
    auto obj = doc.object();
    fromJson(obj);
}

QVector<FileBlock> FileSpliter::split(const QString &fileName, int blockCount) {
    QFileInfo info(fileName);
    if (!info.exists() || !info.isFile()) {
        return {};
    }
    QVector<FileBlock> result;
    qint64 sizeAvg = info.size() / blockCount;
    for (int i = 0; i < blockCount; i++) {
        FileBlock block;
        block.fileName = fileName;
        block.fileSize = info.size();
        block.blockCount = blockCount;
        block.blockIndex = i;
        block.blockStartAddress = i * sizeAvg;
        block.blockSize = sizeAvg;
        // fileSize可能无法整除blockCount，因此最后一块需重新计算
        if (i == blockCount - 1) {
            block.blockSize = info.size() - sizeAvg * (blockCount - 1);
            block.blockStartAddress = info.size() - block.blockSize;
        }
        result.append(std::move(block));
    }
    return result;
}
