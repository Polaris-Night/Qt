#ifndef MFILEMSGMANAGER_H
#define MFILEMSGMANAGER_H

#include <QObject>
#include <QVector>

/********************************文件基本信息类***************************************/
class FileBaseMsg
{
public:
    FileBaseMsg();
    FileBaseMsg(const QString &name, const qint64 &size);
    FileBaseMsg(const QString &fileName, const qint64 &size, const QString &path);
    FileBaseMsg(const FileBaseMsg &baseMsg);
    FileBaseMsg(FileBaseMsg *parent);
    virtual ~FileBaseMsg();

    bool isEmpty();
    FileBaseMsg& operator=(const FileBaseMsg &baseMsg);

    QString fileName;//文件名
    qint64 fileSize;//文件大小
    QString filePath;//文件路径
};
Q_DECLARE_METATYPE(FileBaseMsg)

/*********************************文件信息类****************************************/
class FileMsg : public FileBaseMsg
{
public:
    QVector<qint64> blockSize;//文件分块大小数组
};
Q_DECLARE_METATYPE(FileMsg)

/********************************分块文件信息类***************************************/
class BlockMsg : public FileBaseMsg
{
public:
    BlockMsg();
    BlockMsg(const QString &fileName, const qint64 &fileSize, const int &block, const qint64 &blockSize);
    BlockMsg(const QString &fileName, const qint64 &fileSize, const QString &filePath, const int &block, const qint64 &blockSize);
    BlockMsg(const BlockMsg &blockMsg, FileBaseMsg *parent = nullptr);
    ~BlockMsg();

    BlockMsg& operator=(const BlockMsg &blockMsg);

    int block;//文件分块编号
    qint64 blockSize;//文件分块大小
};
Q_DECLARE_METATYPE(BlockMsg)

/********************************文件信息管理类***************************************/
class MFileMsgManager : public QObject
{
    Q_OBJECT
public:
    explicit MFileMsgManager(QObject *parent = nullptr);

    MFileMsgManager(const int &fileCount, const int &blockCount, QObject *parent = nullptr);
    MFileMsgManager(const MFileMsgManager &msg, QObject *parent = nullptr);
    ~MFileMsgManager();

    QVector<BlockMsg> toBlockMsg();
    qint64 totalSize();
    MFileMsgManager& operator=(const MFileMsgManager &msg);
    FileMsg& operator[](int i);

    int fileCount;
    int blockCount;
    QVector<FileMsg> fileMsg;
};

#endif // MFILEMSGMANAGER_H
