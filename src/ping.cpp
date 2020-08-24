#include "ping.h"

#include <iostream>

#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"

#include <QHostInfo>
#include <QRegularExpression>
#include <QThread>

Ping::Ping(QString const& target, int timeout, QObject* parent) : QObject(parent), mTarget(target), mTargetIp(resolveHostname(target)), mTimeout(timeout), mSendBuffer(), mReplyBuffer(sizeof(ICMP_ECHO_REPLY) + 32 + 1, '\0'), mCounter(0) {
    //
}

Ping::~Ping() {
    //
}

QString Ping::resolveHostname(QString const& hostname) {
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])$");
    QRegularExpressionMatch match = re.match(hostname);
    if (match.hasMatch()) {
        return hostname;
    }

    QHostInfo const hostInfo = QHostInfo::fromName(hostname);
    if ((hostInfo.error() != QHostInfo::NoError) || (hostInfo.addresses().size() == 0)) {
        std::cerr << "Failed to look up IP for hostname: " << hostname.toStdString() << std::endl;
        throw;
    }

    return hostInfo.addresses().at(0).toString();
}

bool Ping::ping(PingResponse& pingResponse) {
    // We declare variables
    HANDLE hIcmpFile;                       // Handler
    unsigned long ipaddr = INADDR_NONE;     // Destination address
    DWORD dwRetVal = 0;                     // Number of replies

    mSendBuffer = QString("Data Buffer:%1").arg(mCounter, 20, 10, QChar('0')).toLocal8Bit();
    ++mCounter;
    // Set the IP-address of the field qlineEdit
    ipaddr = inet_addr(mTargetIp.toStdString().c_str());
    hIcmpFile = IcmpCreateFile();   // create a handler

    // Call the ICMP echo request function
    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, mSendBuffer.data(), mSendBuffer.size(), NULL, mReplyBuffer.data(), mReplyBuffer.size(), mTimeout);

    // We create a row in which we write the response message
    QString strMessage = "";

    pingResponse.target = mTarget;
    if (dwRetVal != 0) {
        pingResponse.hasError = false;
        pingResponse.errorCode = 0;
        // The structure of the echo response
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)mReplyBuffer.data();
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;

        strMessage += "Sent icmp message to " + mTargetIp + "\n";
        if (dwRetVal > 1) {
            strMessage += "Received " + QString::number(dwRetVal) + " icmp message responses \n";
            strMessage += "Information from the first response: ";
        } else {
            strMessage += "Received " + QString::number(dwRetVal) + " icmp message response \n";
            strMessage += "Information from the first response: ";
        }
        strMessage += "Received from ";
        strMessage += inet_ntoa(ReplyAddr);
        strMessage += "\n";
        strMessage += "Status = " + pEchoReply->Status;
        strMessage += "Roundtrip time = " + QString::number(pEchoReply->RoundTripTime) + " milliseconds \n";
        pingResponse.roundTripTime = pEchoReply->RoundTripTime;
    } else {
        pingResponse.hasError = true;
        pingResponse.errorCode = GetLastError();
        strMessage += "Call to IcmpSendEcho failed.\n";
        strMessage += "IcmpSendEcho returned error: ";
        strMessage += QString::number(GetLastError());
    }

    // Display information about the received data
    //std::cout << strMessage.toStdString() << std::endl;

    return true;
}

void Ping::doPing(quint64 pingId) {
    Ping::PingResponse result;
    ping(result);

    emit pingDone(pingId, result);
}
