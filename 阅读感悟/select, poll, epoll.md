#### Linux下用于I/O多路复用的三种机制

1. select  `<sys/select.h>`
    ```cpp
    // O(n) 等待一个或多个文件描述符的事件
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    ```
    ```cpp
    // fd_set 位数组（大小由 FD_SETSIZE 定义，通常为 1024）
    typedef long __fd_mask;
    #define __NFDBITS (8 * (int)sizeof(__fd_mask))
    typedef struct {
        __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
    } fd_set;
    ```
   - 轮询：调用 select 时，内核遍历 fd_set 检查事件状态，返回就绪集合

2. poll  `<poll.h>`
    ```cpp
    // O(n) 等待一个或多个文件描述符的事件
    int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    ```
    ```cpp
    // pollfd 结构体
    struct pollfd {
        int fd;         // 文件描述符
        short events;   // 关注的事件掩码
        short revents;  // 返回的事件掩码
    };

    // 事件掩码
    // 可读事件
    #define POLLIN      0x001   // 可读
    #define POLLPRI     0x002   // 紧急数据可读
    #define POLLRDNORM  0x040   // 普通数据可读
    #define POLLRDBAND  0x080   // 带外数据可读
    // 可写事件
    #define POLLOUT     0x004   // 可写
    #define POLLWRNORM  0x100   // 普通数据可写
    #define POLLWRBAND  0x200   // 带外数据可写
    // 错误/异常事件
    #define POLLERR     0x008   // 错误
    #define POLLHUP     0x010   // 挂起
    #define POLLNVAL    0x020   // 文件描述符无效
    ```
   - 轮询：调用 poll 时，内核遍历 pollfd 数组检查事件状态，返回就绪 fd



3. epoll  `<sys/epoll.h>`
   1. 用户态 `epoll_create()` -> 内核态 `sys_epoll_create()`
      ```cpp
      // O(1) 创建 epoll 实例
      int epoll_create(int size); // size 自 Linux 2.6.8 起仅为提示
      ```

   2. 增加，删除，修改 fd 及 event 到 epfd 中
      ```cpp
      // O(log n) 管理文件描述符和事件（基于红黑树）
      int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
      ```
      
   3. 监听套接字事件（阻塞/非阻塞）< 0：阻塞；= 0：立即返回；> 0：等待
      ```cpp
      // O(1) 等待事件（基于就绪链表）
      int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
      ```

    ```cpp
    // epoll_event 结构体
    struct epoll_event {
        uint32_t events;      // 事件掩码
        epoll_data_t data;    // 用户数据
    };

    // epoll_data 联合体
    typedef union epoll_data {
        void *ptr;
        int fd;
        uint32_t u32;
        uint64_t u64;
    } epoll_data_t;

    // 常用事件掩码
    // 可读事件
    #define EPOLLIN      0x001   // 可读
    #define EPOLLPRI     0x002   // 紧急数据可读
    // 可写事件
    #define EPOLLOUT     0x004   // 可写
    // 错误/异常事件
    #define EPOLLERR     0x008   // 错误
    #define EPOLLHUP     0x010   // 挂起
    #define EPOLLRDHUP   0x2000  // 对端关闭连接
    // 触发模式
    #define EPOLLET      1u << 31 // 边缘触发
    #define EPOLLONESHOT 1u << 30 // 一次性事件
    ```
   - 内核实现：红黑树管理文件描述符，就绪链表存储已触发事件。
   - 事件通知：内核为 epoll_ctr 添加的 fd 关联到内核的事件通知框架

```
                    +-------------------+
                    |      socket()     | --> 创建套接字
                    +-------------------+
                              |
                    +-------------------+
                    |      bind()       | --> 绑定到特定 IP 和端口
                    +-------------------+
                              |
                    +-------------------+
                    |      listen()     | --> 置于监听模式，等待客户端连接请求
                    +-------------------+
                              |
                    +-------------------+
                    |  epoll_create()   | --> 创建 epoll 实例，用于 I/O 多路复用
                    +-------------------+v
                              |
                    +-------------------+
                    |  EPOLL_CTL_ADD    | --> 添加监听套接字，设置监听事件
                    +-------------------+
                              |
                    +-------------------+
                    |  epoll_wait()     | --> 等待 epoll 事件发生
                    +-------------------+
                              |
          +-------------------+-------------------+
          |                   |                   |
+-------------------+ +-------------------+ +-------------------+
|   返回0（超时）    | |   返回-1（出错）   | |   返回>0          |
|                   | |                   | |                   |
|  - 打印超时信息    | |  - 打印错误信息    | |  - 遍历事件数组    |
|  - 可选择继续等待  | |  - 退出或重试      | |  - 处理事件        |
+-------------------+ +-------------------+ +-------------------+
          |                   |                   |
          +-------------------+-------------------+
                              |
                    +-------------------+
                    |    事件处理循环    |
                    +-------------------+

```
