#include"Timer.h"
#include"TimerManager.h"
#include"EventLoop.h"
#include"EventLoopThread.h"
#include"Acceptor.h"
#include"InetAddress.h"
#include"TcpServer.h"
#include"Buffer.h"
#include"http/HttpServer.h"

#include<unistd.h>
#include<unordered_map>
#include<chrono>
#include<sstream>
#include<iomanip>
#include <arpa/inet.h>
#include <csignal>


using std::cout;
using std::endl;
using std::string;


//namespace __test_EventLoop_ {
//
//    EventLoop* p_loop;
//    void timeout() {
//        cout << "到点了！" << endl;
//        p_loop->Quit();
//    }
//
//    void test1() {
//        EventLoop loop;
//        p_loop = &loop;
//        loop.RunAfter(3, timeout);
//        loop.Loop();
//    }
//
//    void run4() {
//        cout << "run4 , tid:" << std::this_thread::get_id() << endl;
//        p_loop->Quit();
//    }
//
//    void run3() {
//        cout << "run3 , tid:" << std::this_thread::get_id() << endl;
//        p_loop->RunAfter(3,run4);
//    }
//
//    void run2() {
//        cout << "run2 , tid:" << std::this_thread::get_id() << endl;
//        p_loop->QueueInLoop(run3);
//    }
//
//    void run1() {
//        cout << "run1 , tid:" << std::this_thread::get_id() << endl;
//        p_loop->RunInLoop(run2);
//    }
//
//    void thread_fun2() {
//        cout << "thread_fun2 , tid:" << std::this_thread::get_id() << endl;
//        p_loop->RunAfter(3, run1);
//    }
//
//    void test2() {
//        EventLoop loop;
//        p_loop = &loop;
//        loop.RunAfter(3, run1);
//        //other thrad
//        std::thread t(thread_fun2);
//        t.detach();
//        loop.Loop();
//    }
//
//    void thread_fun3_1() {
//        cout << "thread_fun3_1 , tid:" << std::this_thread::get_id() << endl;
//    }
//
//    void thread_fun3_2(EventLoop* loop) {
//        cout << "thread_fun3_2 after 3 seconds quit , tid:" << std::this_thread::get_id() << endl;
//        loop->RunAfter(3, std::bind(&EventLoop::Quit, loop));
//    }
//
//    void test3() {
//
//        EventLoopThread loop_thread;
//        EventLoop* loop = loop_thread.StartLoop();
//        loop->RunInLoop(thread_fun3_1);
//        sleep(1);
//        loop->RunAfter(1, thread_fun3_1);
//        std::thread t(thread_fun3_2,loop);
//        t.detach();
//        sleep(4);
//    }
//
//    void TestMain() {
//        test3();
//    }
//
//}
//
//namespace __test_Acceptor_ {
//
//    EventLoop* p_loop;
//    void NewConnection1(int client_fd,const InetAddress& client_addr) {
//        cout << "accept new connection from " << client_addr.ToString() <<
//            "  client_fd : " << client_fd<< endl;
//        write(client_fd, "hello ,you got connection with me!\n", 36);
//
//        std::stringstream ss;
//        ss << "server_time : " << Timestamp::now().ToFormattedString() << '\n';
//        std::string msg = ss.str();
//        write(client_fd, msg.c_str(), msg.size());
//
//        write(client_fd, "server will close connection after 3 sconds!\n", 46);
//        p_loop->RunAfter(3, std::bind(Socket::Close,client_fd));
//    }
//
//    void NewConnection2(int client_fd, const InetAddress& client_addr) {
//        cout << "accept new connection from " << client_addr.ToString() <<
//            "  client_fd : " << client_fd << endl;
//        write(client_fd, "hello ,you got connection with me!\n", 36);
//
//
//        auto local_sockaddr = Socket::GetLocalAddr(client_fd);
//        uint16_t port = ntohs(local_sockaddr.sin_port);
//        char ip[INET_ADDRSTRLEN];
//        inet_ntop(AF_INET, &local_sockaddr.sin_addr, ip, sizeof ip);
//        std::stringstream ss;
//        ss << "server address :" << ip << ":port" << port << '\n';
//        std::string msg = ss.str();
//        write(client_fd, msg.c_str(), msg.size());
//
//        write(client_fd, "server will close connection after 3 sconds!\n", 46);
//        p_loop->RunAfter(3, std::bind(Socket::Close, client_fd));
//    }
//
//    void test1() {
//        EventLoop loop;
//        p_loop = &loop;
//        InetAddress server_addr(8080);
//        Acceptor acceptor(&loop, server_addr);
//        acceptor.set_new_connection_callback(NewConnection1);
//        acceptor.Listen();
//        loop.Loop();
//    }
//
//    void test2() {
//        EventLoop loop;
//        p_loop = &loop;
//        InetAddress server_addr1(8080);
//        InetAddress server_addr2(8090);
//        Acceptor acceptor1(&loop, server_addr1);
//        Acceptor acceptor2(&loop, server_addr2);
//        acceptor1.set_new_connection_callback(NewConnection1);
//        acceptor2.set_new_connection_callback(NewConnection2);
//        acceptor1.Listen();
//        acceptor2.Listen();
//        loop.Loop();
//    }
//
//    void TestMain() {
//        test2();
//    }
//}
//
//namespace __test_TcpServer_ {
//
//    void OnConnection(const TcpServer::SPTcpConnection& conn) {
//        if (conn->Connected()) {
//            cout << "New Connection : [" << conn->name() << "] from " << conn->client_addr().ToString() << endl;
//        }
//        else {
//            cout << "Connection : [" << conn->name() << "] " << conn->client_addr().ToString() << "disconnected" << endl;
//        }
//        
//    }
//
//    void OnMessage(const TcpServer::SPTcpConnection& conn,Buffer* buf,Timestamp receive_time) {
//        cout << receive_time.ToFormattedString() 
//            << "receive " << buf->ReadableBytes() << " bytes from "
//            << conn->client_addr().ToString() << " : " << buf->RetrieveAllAsStr();
//    }
//
//    void test1() {
//        InetAddress server_addr(8080);
//        EventLoop loop;
//        TcpServer server(&loop, server_addr);
//        server.set_connection_callback(OnConnection);
//        server.set_message_callback(OnMessage);
//        server.Start();
//
//        loop.Loop();
//    }
//
//    void TestMain() {
//        test1();
//    }
//}
//
//namespace __test_TcpConnection_ {
//
//    std::string msg1;
//    std::string msg2;
//
//    std::string msg3;
//    void OnConnection(const TcpServer::SPTcpConnection& conn) {
//        if (conn->Connected()) {
//            cout << "New Connection : [" << conn->name() << "] from " << conn->client_addr().ToString() << endl;
//            conn->Send(msg1);
//            conn->Send(msg2);
//            conn->Shutdown();
//        }
//        else {
//            cout << "Connection : [" << conn->name() << "] " << conn->client_addr().ToString() << "disconnected" << endl;
//        }
//        
//    }
//
//    void OnMessage(const TcpServer::SPTcpConnection& conn, Buffer* buf, Timestamp receive_time) {
//        cout << receive_time.ToFormattedString()
//            << "receive " << buf->ReadableBytes() << " bytes from "
//            << conn->client_addr().ToString() << " : " << buf->RetrieveAllAsStr();
//    }
//
//    void OnConnection2(const TcpServer::SPTcpConnection& conn) {
//        if (conn->Connected()) {
//            cout << "New Connection : [" << conn->name() << "] from " << conn->client_addr().ToString() << endl;
//            conn->Send(msg3);
//        }
//        else {
//            cout << "Connection : [" << conn->name() << "] " << conn->client_addr().ToString() << " disconnected" << endl;
//        }
//       
//    }
//
//    void OnSendComplete(const TcpServer::SPTcpConnection& conn) {
//        conn->Send(msg3);
//    }
//
//    void test1() {
//        InetAddress server_addr(8080);
//        EventLoop loop;
//
//        msg1.assign(100, 'A');
//        msg2.assign(200, 'B');
//
//        TcpServer server(&loop, server_addr);
//        server.set_connection_callback(OnConnection);
//        server.set_message_callback(OnMessage);
//        server.Start();
//
//        loop.Loop();
//    }
//
//    void test2() {
//        InetAddress server_addr(8080);
//        EventLoop loop;
//
//        std::string line;
//        for (int i = 33; i < 127; ++i)
//        {
//            line.push_back(char(i));
//        }
//        line += line;
//
//        for (size_t i = 0; i < 127 - 33; ++i)
//        {
//            msg3 += line.substr(i, 72) + '\n';
//        }
//
//        TcpServer server(&loop, server_addr);
//        server.set_connection_callback(OnConnection2);
//        server.set_message_callback(OnMessage);
//        server.set_send_complete_callback(OnSendComplete);
//        server.Start();
//
//        loop.Loop();
//    }
//
//    void TestMain() {
//        test2();
//    }
//}
//
//namespace __test_multi_thread_TcpServer_ {
//
//    std::string msg;
//
//    void OnMessage(const TcpServer::SPTcpConnection& conn, Buffer* buf, Timestamp receive_time) {
//        cout << receive_time.ToFormattedString()
//            << "receive " << buf->ReadableBytes() << " bytes from "
//            << conn->client_addr().ToString() << " : " << buf->RetrieveAllAsStr();
//    }
//
//    void OnConnection(const TcpServer::SPTcpConnection& conn) {
//        if (conn->Connected()) {
//            cout << "New Connection : [" << conn->name() << "] from " << conn->client_addr().ToString() << endl;
//            conn->Send("test msg!\n");
//        }
//        else {
//            cout << "Connection : [" << conn->name() << "] " << conn->client_addr().ToString() << " disconnected" << endl;
//        }
//       
//    }
//
//    void OnSendComplete(const TcpServer::SPTcpConnection& conn) {
//        //conn->Send(msg);
//    }
//
//    void test1() {
//        InetAddress server_addr(8080);
//        EventLoop loop;
//
//        std::string line;
//        for (int i = 33; i < 127; ++i)
//        {
//            line.push_back(char(i));
//        }
//        line += line;
//
//        for (size_t i = 0; i < 127 - 33; ++i)
//        {
//            msg += line.substr(i, 72) + '\n';
//        }
//
//        TcpServer server(&loop, server_addr,4);
//        server.set_connection_callback(OnConnection);
//        server.set_message_callback(OnMessage);
//        server.set_send_complete_callback(OnSendComplete);
//        server.Start();
//
//        loop.Loop();
//    }
//
//    void TestMain() {
//        test1();
//    }
//}


//全局对象，忽略SIGPIPE信号（对方返回RST，本端继续write就会出现）,SIGPIPE信号会使程序终止
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        std::cout << "ignore SIGPIPE" << std::endl;
        signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe initObj;


int main(int argc,char** argv)
{

    //__test_event_loop_::TestMain();

    //__test_Acceptor_::TestMain();

    //__test_TcpServer_::TestMain();

    //__test_TcpConnection_::TestMain();

    //__test_multi_thread_TcpServer_::TestMain();

    uint16_t port = 8080;
    int io_thread_num = 4;

    if (argc == 3) {
        port = atoi(argv[1]);
        io_thread_num = atoi(argv[2]);
    }
   
    InetAddress server_addr(port);
    EventLoop loop;
    HttpServer server(&loop, server_addr, io_thread_num);
    server.Start();
    loop.Loop();




    

}