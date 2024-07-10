#include "rawdataparser.h"
#include <QDebug>
#include <QThread>

/**
 * @brief 处理数据包
 * @remarks
 * 1. 每个数据包是512个字节，前8个字节不要，后面504字节是2路视频数据
 * 2. 全是0xfade/0xdead则是空帧
 */
static QByteArray processByteArray(const QByteArray& input)
{
    if (input.size() % 512 != 0)
        return QByteArray();

    QByteArray ret;
    for (int i = 0; i < input.size(); i += 512) {
        QByteArray packet = input.mid(i, 512);
        packet.remove(0, 8);

        bool allFade = true;
        for (int i = 0; i < 504; i += 2) {
            if (!(packet[i] == char(0xfa) && packet[i + 1] == char(0xde))
                && !(packet[i] == char(0xde) && packet[i + 1] == char(0xad))) {
                allFade = false;
                break;
            }
        }
        if (allFade)
            continue;
        QByteArray processed;
        for (int j = 0; j < 504; j += 4) {
            processed.append(packet[j + 1]);
            processed.append(packet[j]);
        }
        ret.append(processed);
    }
    return ret;
}

RawDataParser::RawDataParser(const QString &fileName, QObject *parent)
    : QObject{parent}, m_fileName(fileName)
{

}

void RawDataParser::setFile1(const QSharedPointer<File1> &file1)
{
    m_file1 = file1;
}

bool RawDataParser::start()
{
    if (!QFile::exists(m_fileName))
        return false;
    m_file.setFileName(m_fileName);
    if (!m_file.open(QIODevice::ReadOnly))
        return false;
    doParse();
    return true;
}

bool RawDataParser::stop()
{
    if (m_file.isOpen())
        m_file.close();
    return true;
}

void RawDataParser::doParse()
{
    QByteArray buffer;
    while (true) {
        buffer.resize(64 * 1024);
        auto bytesRead = m_file.read(buffer.data(), buffer.size());
        if (bytesRead <= 0)
            break;
        if (bytesRead < buffer.size())
            buffer.resize(bytesRead);

        auto after = processByteArray(buffer);
        if (!after.isEmpty()) {
            m_file1->appendData(after);
        }
    }
}
