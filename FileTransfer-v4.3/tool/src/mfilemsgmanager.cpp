#include "mfilemsgmanager.h"

/********************************文件信息管理类***************************************/
MFileMsgManager::MFileMsgManager(QObject *parent)
    : QObject(parent), fileCount(0), blockCount(0)
{
}

MFileMsgManager::MFileMsgManager(const int &fileCount, const int &blockCount, QObject *parent)
    : QObject(parent), fileMsg(fileCount)
{
    this->fileCount = fileCount;
    this->blockCount = blockCount;
    for (int i = 0; i < fileCount; i++)
        fileMsg[i].blockSize = QVector<qint64>(blockCount, 0);
}

MFileMsgManager::MFileMsgManager(const MFileMsgManager &msg, QObject *parent)
    : QObject(parent), fileCount(msg.fileCount), blockCount(msg.blockCount), fileMsg(msg.fileMsg)
{
}

MFileMsgManager::~MFileMsgManager()
{
}

QVector<BlockMsg> MFileMsgManager::toBlockMsg()
{
    QVector<BlockMsg> blockVector;
    for (int i = 0; i < fileCount; i++) {
        for (int j = 0; j < blockCount; j++) {
            BlockMsg blockMsg;
            blockMsg.fileName = fileMsg[i].fileName;
            blockMsg.fileSize = fileMsg[i].fileSize;
            blockMsg.filePath = fileMsg[i].filePath;
            blockMsg.block = j;
            blockMsg.blockSize = fileMsg[i].blockSize[j];
            blockVector.push_back(blockMsg);
        }
    }
    return blockVector;
}

qint64 MFileMsgManager::totalSize()
{
    qint64 size = 0;
    for (int i = 0; i < fileCount; i++)
        size += fileMsg[i].fileSize;
    return size;
}

MFileMsgManager& MFileMsgManager::operator=(const MFileMsgManager &msg)
{
    fileCount = msg.fileCount;
    blockCount = msg.blockCount;
    fileMsg = msg.fileMsg;
    return *this;
}

FileMsg& MFileMsgManager::operator[](int i)
{
    return fileMsg[i];
}

/********************************文件基本信息类***************************************/
FileBaseMsg::FileBaseMsg()
    : fileSize(0)
{
    qRegisterMetaType<FileBaseMsg>("FileBaseMsg");
}

FileBaseMsg::FileBaseMsg(const QString &name, const qint64 &size)
    : fileName(name), fileSize(size)
{
    qRegisterMetaType<FileBaseMsg>("FileBaseMsg");
}

FileBaseMsg::FileBaseMsg(const QString &name, const qint64 &size, const QString &path)
    : fileName(name), fileSize(size), filePath(path)
{
    qRegisterMetaType<FileBaseMsg>("FileBaseMsg");
}

FileBaseMsg::FileBaseMsg(const FileBaseMsg &baseMsg)
    : fileName(baseMsg.fileName), fileSize(baseMsg.fileSize), filePath(baseMsg.filePath)
{
    qRegisterMetaType<FileBaseMsg>("FileBaseMsg");
}

FileBaseMsg::FileBaseMsg(FileBaseMsg *parent)
{
    qRegisterMetaType<FileBaseMsg>("FileBaseMsg");
    if (parent == nullptr)
        return;

    fileName = parent->fileName;
    fileSize = parent->fileSize;
    filePath = parent->filePath;
}

FileBaseMsg::~FileBaseMsg()
{
}

bool FileBaseMsg::isEmpty()
{
    return fileName.isEmpty();
}

FileBaseMsg& FileBaseMsg::operator=(const FileBaseMsg &baseMsg)
{
    fileName = baseMsg.fileName;
    fileSize = baseMsg.fileSize;
    filePath = baseMsg.filePath;
    return *this;
}

/********************************分块文件信息类***************************************/
BlockMsg::BlockMsg()
{
    block = -1;
    blockSize = 0;
}

BlockMsg::BlockMsg(const QString &fileName, const qint64 &fileSize, const int &block, const qint64 &blockSize)
    : FileBaseMsg(fileName, fileSize)
{
    qRegisterMetaType<BlockMsg>("BlockMsg");
    this->block = block;
    this->blockSize = blockSize;
}

BlockMsg::BlockMsg(const QString &fileName, const qint64 &fileSize, const QString &filePath, const int &block, const qint64 &blockSize)
    : FileBaseMsg(fileName, fileSize, filePath)
{
    qRegisterMetaType<BlockMsg>("BlockMsg");
    this->block = block;
    this->blockSize = blockSize;
}

BlockMsg::BlockMsg(const BlockMsg &blockMsg, FileBaseMsg *parent)
    : FileBaseMsg(parent), block(blockMsg.block), blockSize(blockMsg.blockSize)
{
    qRegisterMetaType<BlockMsg>("BlockMsg");
    fileName = blockMsg.fileName;
    fileSize = blockMsg.fileSize;
    filePath = blockMsg.filePath;
}

BlockMsg::~BlockMsg()
{
}

BlockMsg& BlockMsg::operator=(const BlockMsg &blockMsg)
{
    fileName = blockMsg.fileName;
    fileSize = blockMsg.fileSize;
    filePath = blockMsg.filePath;
    block = blockMsg.block;
    blockSize = blockMsg.blockSize;
    return *this;
}
