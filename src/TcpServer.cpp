#include "TcpServer.h"

#include"EventLoop.h"
#include"Acceptor.h"
#include"EventLoopThreadPool.h"

#include<sstream>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr, int thread_num /*=0*/) :
	loop_(loop),
	name_(listen_addr.ToString()),
	acceptor_(std::make_unique<Acceptor>(loop,listen_addr)),
	thread_pool_(std::make_unique<EventLoopThreadPool>(loop,thread_num)),
	started_(false),
	conn_count_(0)
{
	acceptor_->set_new_connection_callback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::Start()
{
	if (!started_) {
		started_ = true;
		thread_pool_->Start();
	}
	loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
}

void TcpServer::NewConnection(int clinet_fd, const InetAddress& clinet_addr)
{
	loop_->AssertInLoopThread();

	++conn_count_;
	std::stringstream ss;
	ss << name_ << " connection:" << clinet_fd;
	std::string conn_name = ss.str();
	//选择一个IO线程
	EventLoop* io_loop = thread_pool_->GetNextLoop();
	SPTcpConnection conn = std::make_shared<TcpConnection>(io_loop, clinet_fd, clinet_addr, conn_name);

	conn_map_[conn_name] = conn;
	conn->SetTcpNodelay(true);
	conn->set_connection_callback(connection_callback_);
	conn->set_message_callback(message_callback_);
	conn->set_close_callback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
	conn->set_send_complete_callback(send_complete_callback_);
	//连接已建立,在IO线程中运行
	io_loop->RunInLoop(std::bind(&TcpConnection::ConnectionEstablished, conn));
}

void TcpServer::RemoveConnection(const SPTcpConnection& conn)
{
	//将IO线程中的RemoveConnection移动到baseloop中，避免锁操作
	loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const SPTcpConnection& conn)
{
	//std::cout << "server [" << name_ << "] RemoveConnection " << conn->name() << std::endl;
	//erase 之后如果用户没有持有SPTcpConnection，则此时引用计数为1
	conn_map_.erase(conn->name());
	--conn_count_;

	//std::cout << "RemoveConnectionInLoop conn use count : "<<conn.use_count() << std::endl;

	//QueueInLoop()是因为channel还处在HandleEvent()中，如果直接调用ConnectionDestoryed()
	//调用完成后TcpConnection进入析构，同时销毁channel
	//也就是说HandleEvent()还没结束，channel本身就销毁了

	//使用bind延长TcpConnection的生命，避免RemoveConnection()结束后TcpConnection析构

	//moduo
	//再将ConnectionDestoryed移动到io线程中执行
	//是为了保证TcpConnection的ConnectionCallback始 终在其ioLoop回调，方便客户端代码的编写
	EventLoop* io_loop = conn->GetLoop();
	io_loop->QueueInLoop(std::bind(&TcpConnection::ConnectionDestoryed, conn));

}
