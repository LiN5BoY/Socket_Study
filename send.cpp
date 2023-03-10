#include<iostream>
#include<Winsock2.h>
#include<windows.h>

//UTF-8转GB2312
char* U2G(const char* utf8){
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len+1];
    memset(wstr, 0, len+1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len+1];
    memset(str, 0, len+1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    if(wstr) delete[] wstr;
    return str;
}
//GB2312转UTF-8
char* G2U(const char* gb2312){
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len+1];
    memset(wstr, 0, len+1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len+1];
    memset(str, 0, len+1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    if(wstr) delete[] wstr;
    return str;
}
//将目标远程主机的IP和端口信息填入sockaddr：
sockaddr GetSockAddr(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint16_t inPort){
    sockaddr addr;
    sockaddr_in *addrin = reinterpret_cast<sockaddr_in*>(&addr);
    addrin->sin_family = AF_INET;
    addrin->sin_addr.S_un.S_un_b.s_b1 = b1;
    addrin->sin_addr.S_un.S_un_b.s_b2 = b2;
    addrin->sin_addr.S_un.S_un_b.s_b3 = b3;
    addrin->sin_addr.S_un.S_un_b.s_b4 = b4;
    addrin->sin_port = htons(inPort);

    return addr;
}

int main(){
    //先进行激活
    ////WWSAStartup函数：第一个参数代表使用的版本号，该函数会向第二个参数填入被激活的socket库的信息
    WSADATA wsd;
    if(WSAStartup(MAKEWORD(2, 2), &wsd)){
        std::cout << "WSAStartup Error" << std::endl;
        exit(-1);
    }
    //创建socket，使用ipv4协议，报文为离散报文，默认协议（udp）
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    const size_t BufMaxSize = 1000;
    char buf[BufMaxSize] = {};
        sockaddr toAddr = GetSockAddr(192, 168, 0, 182, 50002); 
    int toAddrLen = sizeof(sockaddr);
    std::cout << ">>> ";
    while(true){
        if(std::cin >> buf){
            char *buf_UTF8 = G2U(buf);

            //sendTo函数的参数如下：
            //int sendto(SOCKET s,const char buf,int len,int flags,const struct sockaddr to,int tolen);
            //s:数据包应该使用的socket，如果没有绑定，socket库将自动绑定一个可用的端口。
            //buf:带发送数据的起始地址的指针。可以是任何能够转为char*的数据类型
            //len:待发送数据的大小，尽量避免发送数据大于1300字节的数据包
            //flags:对控制发送的标志进行按位或者运算的结果，该值通常取0即可
            //to:目标接收者的sockaddr。注意to的地址族必须和用于创建socket的地址族一致
            //tolen:to的sockaddr的大小。对于IPV4,传入sizeof(sockaddr_in)即可。
            //sendto操作成功返回等待发送的数据长度（说明成功进入发送队列），否则返回-1。
            sendto(udpSocket, buf_UTF8, strlen(buf_UTF8), 0, &toAddr, toAddrLen);
            std::cout << "已发送!" <<std::endl;
            std::cout << ">>> ";
            memset(buf, 0, sizeof(buf));
        }
    }

    //shutdown(udpSocket, SB_BOTH);
    closesocket(udpSocket);

    WSACleanup();

    return 0;
}