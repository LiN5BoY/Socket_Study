#include<iostream>
#include<cstring>
#ifdef _WIN32

#include<winsock2.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
typedef int SOCKET;

#endif

int main(){
#ifdef _WIN32
    WSADATA wsd;
    //WWSAStartup函数：第一个参数代表使用的版本号，该函数会向第二个参数填入被激活的socket库的信息
    if(WSAStartup(MAKEWORD(2, 2), &wsd)){
        std::cout << "WSAStartup Error" << std::endl;
        exit(-1);
    }   
#endif
    //socket函数：(int af,int type,int protocol)
    //af：使用协议族，如AF_INET为Ipv4,AF_INET6表示IPV6等
    //type：指明通过socket发送和接受分组的形式。如SOCK_STREAM表示有序，可靠的数据流分段；SOCK_DGRAM表示离散的报文；SOCK_RAW表示数据头部可以由应用层自定义
    //protocol：指明发送数据使用什么协议。IPPROTO_UDP；IPPOTO_TCP；IPPROTO_IP;0表示根据socket类型选择默认协议。
    
    //像这里，那么就是选择了ipv4协议，离散的报文，默认协议（udp）
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0); 

    //下面属于bind函数的范围，那么参数为：
    //(SOCKET sock,const sockaddr *address,int address_len)
    //sock:绑定socket
    //address:是指发送数据包的源地址！而不是发送目的地地址
    //address_len:存储address的sockaddr结构体大小。bind成功时返回0，出现错误时返回-1
    //给端口号赋值0，并且告诉socket库找一个未被使用的端口并绑定
    //如果一个进程试图使用一个未绑定的socket发送数据，网络库将自动为socket绑定一个可用的端口号
    //对于服务器来说手动调用bind绑定是必须的，而对于客户端来说通常是没有必要的。
    sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_addr.s_addr = htonl(INADDR_ANY);
    sain.sin_port = htons(atoi("50002"));
    if(bind(udpSocket, (sockaddr *)&sain, sizeof(sockaddr)) == -1){
        std::cout << "绑定失败" << std::endl;
    }   
    const size_t BufMaxSize = 1000;
    char buf[BufMaxSize] = {}; 
    sockaddr fromAddr;
#ifndef _WIN32
    unsigned
#endif
    //recfrom函数：(SOCKET s,char buf,int len,int flags,struct sockaddr from,int *fromlen)
    //s：查询数据的socket
    //buf：接收的数据包的缓冲区
    //len：buf可以存储的最大字节数。到达的数据包的剩余字节将被丢弃。
    //flags：同sendto flags
    //fromlen：from所指向的sockaddr的大小
    int fromAddrLen = sizeof(sockaddr);
    std::cout << "等待接收..." << std::endl;
    while(true){
        if(recvfrom(udpSocket, buf, BufMaxSize, 0, &fromAddr, &fromAddrLen) != -1){
            std::cout << "接收到数据：" << buf << std::endl;
            memset(buf, 0, sizeof(buf));
        }   
        else{
            std::cout << "接收失败或发生错误！" << std::endl;
            return -1; 
        }   
    }   
    //shutdown(udpSocket, SB_BOTH);
    //关闭工作
#ifdef _WIN32
    WSACleanup();
    closesocket(udpSocket);
#else
    close(udpSocket);
#endif

    return 0;
}