ADD_EXECUTABLE(server 
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    Epoll.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    InetAddress.cpp
    main.cpp
    Socket.cpp
    TcpConnection.cpp
    TcpServer.cpp
    Timer.cpp
    TimerManager.cpp
    Timestamp.cpp
    http/HttpContext.cpp
    http/HttpRequest.cpp
    http/HttpResponse.cpp
    http/HttpServer.cpp
)
TARGET_LINK_LIBRARIES(server pthread)
  
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
    
    
    
    
    
    
    