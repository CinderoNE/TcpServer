#include "TcpConnection.h"

#include"EventLoop.h"
#include"Socket.h"
#include"Channel.h"

#include<unistd.h>
#include<cstring>
#include <cassert>

TcpConnection::TcpConnection(EventLoop* loop, int client_fd, const InetAddress& client_addr, const std::string& conn_name):
	loop_(loop),
	channel_(std::make_unique<Channel>(client_fd)),
	socket_(std::make_unique<Socket>(client_fd)),
	client_addr_(client_addr),
	name_(conn_name),
	state_(kConneting)
{
	channel_->EnableRead();
	channel_->set_read_callback(std::bind(&TcpConnection::HandleRead,this,std::placeholders::_1));
	channel_->set_write_callback(std::bind(&TcpConnection::HandleWrite, this));
	channel_->set_close_callback(std::bind(&TcpConnection::HandleClose, this));
	channel_->set_error_callback(std::bind(&TcpConnection::HandleError, this));
	
}

TcpConnection::~TcpConnection()
{
	//std::cout << "TcpConnection dtor" << std::endl;
}

void TcpConnection::Send(const std::string& msg)
{
	if (loop_->IsInLoopThread()) {
		SendInLoop(msg);
	}
	else {
		loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, msg));
	}
}

void TcpConnection::SendInLoop(const std::string& msg)
{
	loop_->AssertInLoopThread();
	//先尝试直接发送数据，如果一次发不完就放进write_buffer,
	//通过handle_write发送后续数据
	ssize_t write_n = 0;
	//如果write_buffer已经有数据，则不能直接发送，因为会乱序
	if (write_buffer_.ReadableBytes() == 0) {
		write_n = write(channel_->GetFd(), msg.data(), msg.size());
		if (write_n >= 0) {
			if (static_cast<size_t>(write_n) < msg.size()) {
				//std::cout << "send not finish" << std::endl;
			}
			else {
				//std::cout << "SendInLoop() send finish" << std::endl;
				if (send_complete_callback_) {
					send_complete_callback_(shared_from_this());
				}
					
			}
		}
		else {
			write_n = 0;
			if (errno != EAGAIN) {
				//std::cerr << "SendInLoop error errno : " << errno << std::endl;
			}
		}
	}

	if (static_cast<size_t>(write_n) < msg.size()) {
		write_buffer_.Append(msg.data() + write_n, msg.size() - write_n);
		channel_->EnableWrite();
		loop_->EpollUpdateChannel(channel_.get());
	}

}

void TcpConnection::Shutdown()
{
	if (state_ == kConnected) {
		set_state(kDisconnecting);
		if (loop_->IsInLoopThread()) {
			ShutdownInLoop();
		}
		else {
			loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, shared_from_this()));
		}
	}
	
}


void TcpConnection::ShutdownInLoop()
{
	loop_->AssertInLoopThread();
	//如果没有数据未写出去，则直接关闭写端(发送FIN给对端)，
	// 之后通过HandleRead读到0关闭连接

	//如果还有数据未写出去，则在HandleWrite中重新调用ShutdownInLoop
	if (!channel_->IsWriting()) {
		socket_->ShutdownWrite();
	}

}

void TcpConnection::SetTcpNodelay(bool on)
{
	socket_->SetNoDelay(on);
}


void TcpConnection::ConnectionEstablished()
{
	//std::cout << socket_->sock_fd() <<" : ConnectionEstablished  " << std::endl;
	set_state(kConnected);
	loop_->EpollAddChannel(channel_.get());
	connection_callback_(shared_from_this());
	
}

void TcpConnection::ConnectionDestoryed()
{
	//std::cout <<socket_->sock_fd()<< "ConnectionDestoryed" << std::endl;
	set_state(kDisconnected);
	channel_->DisableAll();
	connection_callback_(shared_from_this());
	loop_->EpollRemoveChannel(channel_.get());
}

void TcpConnection::HandleRead(Timestamp receive_time)
{
	int save_err = 0;
	ssize_t n = read_buffer_.ReadFd(channel_->GetFd(), save_err);
	if (n > 0) {
		message_callback_(shared_from_this(), &read_buffer_, receive_time);
	}
	else if (n == 0) {
		if (state_ == kDisconnecting) {
			//std::cout << "主动shutdown 后 client返回的FIN" << std::endl;
		}
		else {
			//std::cout << "对方主动关闭连接的fin" << std::endl;
			set_state(kPeerFIN);
		}
		HandleClose();
	}
	else {
		std::cerr << "HandleRead error,errorno" << save_err << std::endl;
		HandleError();
	}
	
}

void TcpConnection::HandleWrite()
{
	//是否还有数据未写出去
	//触发了close事件后，会取消所有注册事件(DisableAll)，所以不会进入
	if (channel_->IsWriting()) {
		//这里没有循环调用send
		//如果一次send没有发送完全部数据，
		//第二次肯定会返回EAGAIN，因此这样可以节省一次系统调用
		ssize_t write_n = write(channel_->GetFd(),
			write_buffer_.ReadBegin(),
			write_buffer_.ReadableBytes());

		if (write_n > 0) {
			write_buffer_.Retrieve(write_n);
			//数据已经发送完
			if (write_buffer_.ReadableBytes() == 0) {
				//std::cout << "HandleWrite() send finish" << std::endl;

				channel_->DisalbeWrite();
				loop_->EpollUpdateChannel(channel_.get());
				if (send_complete_callback_) {
					send_complete_callback_(shared_from_this());
				}
				//对方主动关闭连接，发送完未发送数据后直接close
				if (state_ == kPeerFIN) {
					ForceClose();
					return;
				}
				
				//己方主动关闭连接，发送完未发送数据后，关闭write端(主动发送FIN)，
				//之后HandleRead读到0（对方FIN），再关闭连接
				if (state_ == kDisconnecting) {
					ShutdownInLoop();
				}
			}
			else {
				std::cout << "need send more data !" << std::endl;
			}
		}
		else {
			//socket写缓冲已满，等待下一次epoll返回
			if (errno == EAGAIN) {
				std::cout << "need send more data !" << std::endl;
			}
			else 
			{
				//如果不是对方主动关闭连接，一旦出现错误，HandleRead()会读到0字节，继而关闭连接
				//std::cout << "HandleWrite error,errno : " << errno << std::endl;
				if (state_ == kPeerFIN) {
					//强制关闭是因为，如果发生错误且write_buffer_中还有数据，HandleClose()不会关闭连接
					ForceClose();
				}
				
			}
		}
	}
	else {
		//std::cout << "connection down no more write" << std::endl;
	}
}

void TcpConnection::HandleClose()
{
	//std::cout << "handleclose" << std::endl;
	//主动关闭连接
	if (state_ == kDisconnecting || state_ == kConnected) {
		ForceClose();
	}
	//对方主动关闭连接
	else if(state_ == kPeerFIN){
		//还有数据未发送
		if (write_buffer_.ReadableBytes() != 0) {
			
		}
		else {
			ForceClose();
		}
	}
}

void TcpConnection::ForceClose()
{
	//std::cout << "forceclose" << std::endl;
	channel_->DisableAll();
	loop_->EpollUpdateChannel(channel_.get());
	close_callback_(shared_from_this());
}

void TcpConnection::HandleError()
{
	int error = Socket::GetSockError(socket_->sock_fd());
	
	
	char buf[32];
	bzero(buf, sizeof buf);
	char* error_msg = strerror_r(error, buf, sizeof buf);
	/*std::cerr << "TcpConnection::handleError [" << name_
		<< "] - SO_ERROR = " << error << " " << error_msg << std::endl;*/
	
	

	

}




