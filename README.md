# 基于Linux的Socket编程学习
## 网络中进程之间如何通信？

首先需要解决的问题是——如何唯一标识一个进程。

在本地可以通过进程PID来唯一标识一个进程，但在网络是**行不通的**。

网络层的“**ip地址**”可以唯一标识网络中的主机，而**传输层的“协议+端口”**可以唯一标识主机中的应用程序（进程）。这样，利用三元组（ip地址，协议，端口）就可以标识网络中的进城了，网络中的进程通信就可以利用这个标志与其他进程进行交互。

使用TCP/IP协议的应用程序通常采用应用编程接口：UNIX BSD的套接字（socket）。

## socket的基本操作

socket的实现过程是——“open-write/read-close”模式的一种实现

以TCP为例，介绍几个基本的socket接口函数

### socket()函数

```C++
int socket(int domain, int type, int protocol);
```

socket函数对应于普通文件的打开操作，普通文件的打开操作返回一个文件描述字，而**socket()**用于创建一个socket描述符（socket descriptor），它标识唯一的一个socket。

创建socket的时候，也可以指定不同的参数创建不同的socket描述符，socket函数的三个参数分别为:



#### domain

即协议域，又称为协议族。常用的协议族有：AF_INET、AF_INET6、AF_LOCAL(或者为AF_UNIX,Unix域socket)、AF_ROUTE等等。

协议族决定的socket的地址类型，在通信中必须采用对应的地址，如AF_INET决定了要用ipv4地址（32位）与端口号为16位的组合、AF_UNIX决定了要用一个绝对路径名作为地址



#### type

指定socket类型。常用的socket类型有，SOCK_STREAM、SOCK_DGRAM、SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET等等



#### protocol

指定协议。。常用的协议有，IPPROTO_TCP、IPPTOTO_UDP、IPPROTO_SCTP、IPPROTO_TIPC等，它们分别对应TCP传输协议、UDP传输协议、STCP传输协议、TIPC传输协议

上面的type与protocol并不是刻意随意组合的，如SOCK_STREAM不可以跟IPPROTO_UDP组合。当protocol为o时，会自动选择type类型对应的默认协议。

当我们调用socket创建一个socket时，返回的socket描述子它存在于协议族（address family，AF_XXX）空间中，但没有一个具体的地址。如果想要给它赋值一个地址，就必须调用**bind()**函数，否则就当调用**connect()**,**listen()**时系统会自动随机分配一个端口。



### bind()函数

bind()函数把一个地址族中的特定地址赋给socket，例如对应AF_INET,AF_INET6就是把一个ipv4或者ipv6地址和端口号组合赋给socket。

```c++
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

函数的三个参数分别为：

#### sockfd

即socket描述字，它是通过socket()函数创建了一个唯一标识，即一个socket。

bind()函数就是给这个描述字绑定一个名字

#### addr

一个const struct sockaddr*指针，指向要绑定给sockfd的协议地址。结构地址根据地址创建socket的地址协议族的不同而不同，如ipv4对应的是：

```c++
struct sockaddr_in {
    sa_family_t    sin_family; /* address family: AF_INET */
    in_port_t      sin_port;   /* port in network byte order */
    struct in_addr sin_addr;   /* internet address */
};

/* Internet address. */
struct in_addr {
    uint32_t       s_addr;     /* address in network byte order */
};
```

ipv6对应的是：

```c++
struct sockaddr_in6 { 
    sa_family_t     sin6_family;   /* AF_INET6 */ 
    in_port_t       sin6_port;     /* port number */ 
    uint32_t        sin6_flowinfo; /* IPv6 flow information */ 
    struct in6_addr sin6_addr;     /* IPv6 address */ 
    uint32_t        sin6_scope_id; /* Scope ID (new in 2.4) */ 
};

struct in6_addr { 
    unsigned char   s6_addr[16];   /* IPv6 address */ 
};
```

Unix域对应的是：

```c++
#define UNIX_PATH_MAX    108

struct sockaddr_un { 
    sa_family_t sun_family;               /* AF_UNIX */ 
    char        sun_path[UNIX_PATH_MAX];  /* pathname */ 
};
```

#### addrlen

对应的是地址的长度。

通常服务器在启动的时候都会绑定一个众所周知的地址（如ip地址+端口号），用于提供服务，客户就可以通过它来连接服务器；而客户端就不用指定，有系统自动分配一个端口号和自身的ip地址组合，这就是为什么通常服务器端在listen之前会调用bind()，而客户端就不会调用，而是在connect()时由系统随机生成一个。

### listen()、connect()函数

如果作为一个服务器，在调用socket()、bind()之后就会调用listen()来监听这个socket，如果客户端这时调用connect()发出连接请求，服务器端就会接收到这个请求。

```c++
int listen(int sockfd, int backlog);
```

listen函数的第一个参数为要监听的socket描述字

第二个参数为相应socket可以排队的最大连接个数。

socket()函数创建的socket默认是一个主动类型的，listen函数将socket变为被动类型的，等待客户的连接请求。

```c++
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

connect函数的第一个参数即为客户端的socket描述字

第二参数为服务器的socket地址

第三个参数为socket地址的长度。

客户端通过调用connect函数来建立与TCP服务器的连接

### accept()函数

TCP服务器端依次调用socket(),bind(),listen()之后，就会监听指定的socket地址了。TCP客户端依次调用socket(),connect()之后就像TCP服务器发送了一个连接请求。TCP服务器监听到这个请求之后，就会调用accept()函数去接收请求，这样连接就建立好了，就可以进行网络I/O操作了。

```c++
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

accept函数的第一个参数为服务器的socket描述字，

第二个参数为指向struct sockaddr*的指针，用于返回客户端的协议地址。

第三个参数为协议地址的长度。如果accept成功，那么其返回值是由内核自动生成的一个全新的描述字，代表与返回客户的TCP连接。

**注意**：accept的第一个参数为服务器的socket描述字，是服务器开始调用socket()函数生成的，称为监听socket描述字；而accept函数返回的是已连接的socket描述字。一个服务器通常仅仅只创建一个监听socket描述字，它在该服务器的生命周期内一直存在。内核为每个由服务器进程接受的客户连接创建了一个已连接socket描述字，当服务器完成了对某个客户的服务，相应的已连接socket描述字就被关闭。

### read()、write()函数

网络I/O操作有下面几组：

- read()/write()
- recv()/send()
- readv()/writev()
- recvmsg()/sendmsg()
- recvfrom()/sendto()

最常用的I/O函数为recvmsg()与sendmsg()，声明如下：

```c++
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
const struct sockaddr *dest_addr, socklen_t addrlen);
       
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
```

read函数是负责从fd中读取内容，读取成功时，read返回实际所读的字节数，如果返回的值是o表示已经读到文件的结束了。
小于o表示出现了错误。如果错误为EINTR说明读是由终端引起的，如果是ECONNREST表示网络连接出了问题。

write函数将buf中的nbytes字节内容写入文件描述符fd。
成功时返回写的字节数。失败时返回-1，并设置errno变量。
在网络程序中，当我们向套接字文件描述符写时可能有两种可能:)write的返回值大于0，表示写了部分或者是全部的数据。2)返回的值小于0，此时出现了错误。我们要根据错误类型来处理。如果错误为EINTR表示在写的时候出现了中断错误。如果为EPIPE表示网络连接出现了问题(对方已经关闭了连接)。
下面的例子使用到send/recv。



### close()函数

进行完读写操作就需要进行关闭操作。

```c++
#include <unistd.h>
int close(int fd);
```

close一个TCP socket，代表着把该socket标记为已关闭，然后立即返回到调用进程。该描述字不能再由调用进程使用，也就是说不能再作为read或write的第一个参数。

**注意**：close操作只是使相应socket描述字的引用计数-1，只有当引用计数为0的时候，才会触发TCP客户端向服务器发送终止连接请求。



## socket中的TCP三次握手建立连接

tcp建立连接要进行的“三次握手”，即交换三个分组，大致流程如下：

- 客户端向服务器发送一个SYN J
- 服务器向客户端响应一个SYN K，并对SYN J进行确认ACK J+1
- 客户端再想服务器发一个确认ACK K+1

![image](https://images.cnblogs.com/cnblogs_com/skynet/201012/201012122157476286.png)

**总结**：客户端的connect在三次握手的第二次返回，而服务器端的accept在三次握手的第三次返回。



## socket中TCP的四次握手释放连接

### ![image](https://images.cnblogs.com/cnblogs_com/skynet/201012/201012122157494693.png)

如上图所示，图示过程如下：

- 某个应用进程首先调用**close**主动关闭连接，这时TCP发送一个FIN M；
- 另一端接收到FIN M之后，执行被动关闭，对这个FIN进行确认。它的接收也作为文件结束符传递给应用进程，因为FIN的接收意味着应用进程在相应的连接上再也接收不到额外数据；
- 一段时间之后，接收到文件结束符的应用进程调用close关闭它的socket。这导致它的TCP也发送一个FIN N；
- 接收到这个FIN的源发送端TCP对它进行确认。
