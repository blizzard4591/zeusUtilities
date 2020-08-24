#include "ping.h"

#include <iostream>

#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"

Ping::Ping() : sendBuffer(), replyBuffer(sizeof(ICMP_ECHO_REPLY) + 32 + 1, '\0'), counter(0) {
    //
}

Ping::~Ping() {
    //
}


bool Ping::ping(QString const& ip, int timeout, PingResponse& pingResponse) {
    // We declare variables
    HANDLE hIcmpFile;                       // Handler
    unsigned long ipaddr = INADDR_NONE;     // Destination address
    DWORD dwRetVal = 0;                     // Number of replies

    sendBuffer = QString("Data Buffer:%1").arg(counter, 20, 10, QChar('0')).toLocal8Bit();
    ++counter;
    // Set the IP-address of the field qlineEdit
    ipaddr = inet_addr(ip.toStdString().c_str());
    hIcmpFile = IcmpCreateFile();   // create a handler

    // Call the ICMP echo request function
    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, sendBuffer.data(), sendBuffer.size(), NULL, replyBuffer.data(), replyBuffer.size(), 1000);

    // We create a row in which we write the response message
    QString strMessage = "";

    if (dwRetVal != 0) {
        pingResponse.hasError = false;
        pingResponse.errorCode = 0;
        // The structure of the echo response
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuffer.data();
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;

        strMessage += "Sent icmp message to " + ip + "\n";
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
    std::cout << strMessage.toStdString() << std::endl;

    return true;
}

void Ping::doPing(uint64_t pingId, QString const& targetIp, int timeout) {
    Ping::PingResponse result;
    ping(targetIp, timeout, result);

    emit pingDone(pingId, result);
}
