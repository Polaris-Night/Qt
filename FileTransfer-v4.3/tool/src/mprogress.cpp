#include "mprogress.h"

MProgress::MProgress(QObject *parent)
: QObject(parent)
, m_totalSize(0)
, m_currentSize(0) { }

MProgress::MProgress(const qint64 &size, QObject *parent)
: QObject(parent)
, m_totalSize(size)
, m_currentSize(0) { }

MProgress::MProgress(const MProgress &prog, QObject *parent)
: QObject(parent)
, m_totalSize(prog.m_totalSize)
, m_currentSize(prog.m_currentSize) { }

MProgress::~MProgress() { }

void MProgress::setTotalSize(const qint64 &size) { m_totalSize = size; }

void MProgress::addTotalSize(const qint64 &size) { m_totalSize += size; }

void MProgress::setCurrentSize(const qint64 &size) { m_currentSize = size; }

void MProgress::addSize(const qint64 &size) { m_currentSize += size; }

int MProgress::getProgress() const {
    return (static_cast<double>(m_currentSize) / static_cast<double>(m_totalSize) * 100);
}

qint64 MProgress::getTotalSize() const { return m_totalSize; }

qint64 MProgress::getCurrentSize() const { return m_currentSize; }

void MProgress::clear() { m_totalSize = m_currentSize = 0; }
