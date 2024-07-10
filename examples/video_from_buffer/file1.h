#ifndef FILE1_H
#define FILE1_H

#include <QIODevice>
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class File1 : public QIODevice
{
    Q_OBJECT
public:
    explicit File1(QObject *parent = nullptr);
    ~File1();

    void appendData(const QByteArray &data);
    void start();
    void stop();
protected:
    qint64 readData(char *data, qint64 len) override;
    qint64 writeData(const char *data, qint64 maxSize) override { return 0; }
    qint64 size() const override { return 0; }
    qint64 bytesAvailable() const override { return std::numeric_limits<qint64>::max(); }
    bool isSequential() const override { return false; }
    bool atEnd() const override { return false; }
private:
    QByteArray m_buffer;
    QMutex m_mutex;
    QWaitCondition m_cond;
    const int m_bufferSize = 64 * 1024;
    bool m_quit = false;
};

#endif // FILE1_H
