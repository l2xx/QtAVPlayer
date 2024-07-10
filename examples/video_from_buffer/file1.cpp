#include "file1.h"
#include <QByteArray>
#include <QDebug>
#include <QThread>
#include <QMutexLocker>

File1::File1(QObject *parent)
    : QIODevice(parent)
{
    open(QIODevice::ReadOnly);
}

File1::~File1()
{
    stop();
}

void File1::appendData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_buffer.append(data);
    m_cond.wakeAll();
}

void File1::start()
{
    {
        QMutexLocker locker(&m_mutex);
        m_quit = false;
    }
    m_cond.wakeAll();
}

void File1::stop()
{
    {
        QMutexLocker locker(&m_mutex);
        m_quit = true;
    }
    m_cond.wakeAll();
}

qint64 File1::readData(char *data, qint64 len)
{
    if (!len)
        return 0;
    QMutexLocker locker(&m_mutex);
    qint64 bytesWritten = 0;
    while (len) {
        if (m_buffer.isEmpty()) {
            m_cond.wait(&m_mutex);
            if (m_quit || m_buffer.isEmpty())
                break;
        }
        qint64 toWrite = qMin(len, (qint64)m_buffer.size());
        QByteArray ret = m_buffer.left(toWrite);
        m_buffer.remove(0, toWrite);
        memcpy(data, ret.data(), ret.size());
        bytesWritten += toWrite;
        data += toWrite;
        len -= toWrite;
    }
    if (m_quit) {
        memset(data, 0, static_cast<size_t>(len));
        bytesWritten = len;
    }
    return bytesWritten;
}
