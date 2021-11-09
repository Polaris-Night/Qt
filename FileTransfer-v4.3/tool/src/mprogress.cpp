#include "mprogress.h"

MProgress::MProgress(QObject *parent)
    : QObject(parent), totalSize(0), currentSize(0)
{
}

MProgress::MProgress(const qint64 &size, QObject *parent)
    : QObject(parent), totalSize(size), currentSize(0)
{
}

MProgress::MProgress(const MProgress &prog, QObject *parent)
    : QObject(parent), totalSize(prog.totalSize), currentSize(prog.currentSize)
{
}

MProgress::~MProgress()
{
}

void MProgress::setTotalSize(const qint64 &size)
{
    totalSize = size;
}

void MProgress::setCurrentSize(const qint64 &size)
{
    currentSize = size;
}

void MProgress::addSize(const qint64 &size)
{
    currentSize += size;
}

int MProgress::getProgress() const
{
    return (static_cast<double>(currentSize) / static_cast<double>(totalSize) * 100);
}

qint64 MProgress::getTotalSize() const
{
    return totalSize;
}

qint64 MProgress::getCurrentSize() const
{
    return currentSize;
}

void MProgress::clear()
{
    totalSize = currentSize = 0;
}
