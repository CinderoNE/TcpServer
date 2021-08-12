#include "HttpServer.h"

#include"HttpContext.h"
#include"HttpRequest.h"
#include"HttpResponse.h"
#include"../EventLoop.h"
#include"../Timer.h"


extern char favicon[555];

/*
HTTP请求报文

请求方法|空格|URL|空格|协议版本|CRLF          ----请求行
头部字段名|:|值|CRLF                          ----请求头
...                                           ...
CRLF
请求数据                                      ----请求数据

*/

/*
HTTP响应报文

协议版本|空格|状态码|空格|状态码描述符|CRLF   ----状态行
头部字段名|:|值|CRLF                          ----响应头
...                                           ...
CRLF
响应体                                        ----响应体

*/
void DefaultHttpCallback(const HttpRequest& req, HttpResponse* resp)
{
	/*std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;
	if (!benchmark)
	{
		const std::map<string, string>& headers = req.headers();
		for (const auto& header : headers)
		{
			std::cout << header.first << ": " << header.second << std::endl;
		}
	}*/
	if (req.path() == "/")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("text/html");
		resp->addHeader("Server", "Muduo");
		std::string now = Timestamp::now().ToFormattedString();
		resp->setBody("<html><head><title>This is title</title></head>"
			"<body><h1>Hello</h1>Now is " + now +
			"</body></html>");
	}
	else if (req.path() == "/favicon.ico")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("image/png");
		resp->setBody(std::string(favicon, sizeof favicon));
	}
	else if (req.path() == "/hello")
	{
		resp->setStatusCode(HttpResponse::k200Ok);
		resp->setStatusMessage("OK");
		resp->setContentType("text/plain");
		resp->addHeader("Server", "Muduo");
		resp->setBody("hello, world!\n");
	}
	else
	{
		resp->setStatusCode(HttpResponse::k404NotFound);
		resp->setStatusMessage("Not Found");
		resp->setCloseConnection(true);
	}
}


HttpServer::HttpServer(EventLoop* loop, const InetAddress& listen_addr, int thread_num)
	:tcp_server_(loop,listen_addr,thread_num),
	http_callback_(std::bind(DefaultHttpCallback, std::placeholders::_1, std::placeholders::_2))
{
	tcp_server_.set_connection_callback(std::bind(&HttpServer::OnConnection,this, 
		std::placeholders::_1));//SPTcpConnection
	tcp_server_.set_message_callback(std::bind(&HttpServer::OnMessage, this, 
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));//SPTcpConnection,Buffer*,Timestamp

}

HttpServer::~HttpServer()
{
}


void HttpServer::OnConnection(const TcpServer::SPTcpConnection& conn)
{
	if (conn->Connected()) {
		/*std::cout << "new connection : " << conn->name() << std::endl;*/
		conn->set_context(new HttpContext());
		
	}
	else {
		//std::cout << "disconnected : " << conn->name() << std::endl;
	}
	
}

void HttpServer::OnMessage(const TcpServer::SPTcpConnection& conn, Buffer* buf, Timestamp receive_time)
{
	HttpContext* http_context = conn->context();
	// 解析请求
	if (!http_context->ParseRequest(buf, receive_time))
	{
		conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->Shutdown(); // 关闭连接
	}

	// 请求消息解析完毕
	if (http_context->ParseFinish())  // state_ == ParseFinish
	{
		if(OnRequest(conn, http_context->request()))
			http_context->reset(); // 长连接才重置
	}
}

bool HttpServer::OnRequest(const TcpServer::SPTcpConnection& conn, const HttpRequest& http_request)
{
	std::string& connection = const_cast<std::string&>(http_request.getHeader("Connection"));
	for (size_t i = 0; i < connection.size(); ++i) {
		connection[i] = std::tolower(static_cast<unsigned char>(connection[i]));
	}
	bool close = connection == "close" ||
		(http_request.getVersion() == HttpRequest::kHttp10 && connection != "keep-alive");
	HttpResponse response(close); // 构造响应
	http_callback_(http_request, &response);  // 用户回调
	std::string buf;
	// 此时response已经构造好，将向客户发送Response添加到buffer中
	response.appendToBuffer(&buf);
	conn->Send(buf);
	// 如果非Keep-Alive则直接关掉
	if (response.closeConnection())
	{
		conn->Shutdown();
		return false; //close
	}
	else {
		if (!conn->context()->ShutdownTimerExpired()) {
			conn->context()->ResetShutdownTimer(5.0); // 重置主动关闭连接时间
		}
		else {
			conn->context()->set_shutdown_timer(
				conn->GetLoop()->RunAfter(5.0, std::bind(&TcpConnection::Shutdown, conn))); //5秒后没有收到消息关闭连接
		}	
		return true; //keep-alive
	}
}


char favicon[555] = {
  '\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA',
  '\x0', '\x0', '\x0', '\xD', 'I', 'H', 'D', 'R',
  '\x0', '\x0', '\x0', '\x10', '\x0', '\x0', '\x0', '\x10',
  '\x8', '\x6', '\x0', '\x0', '\x0', '\x1F', '\xF3', '\xFF',
  'a', '\x0', '\x0', '\x0', '\x19', 't', 'E', 'X',
  't', 'S', 'o', 'f', 't', 'w', 'a', 'r',
  'e', '\x0', 'A', 'd', 'o', 'b', 'e', '\x20',
  'I', 'm', 'a', 'g', 'e', 'R', 'e', 'a',
  'd', 'y', 'q', '\xC9', 'e', '\x3C', '\x0', '\x0',
  '\x1', '\xCD', 'I', 'D', 'A', 'T', 'x', '\xDA',
  '\x94', '\x93', '9', 'H', '\x3', 'A', '\x14', '\x86',
  '\xFF', '\x5D', 'b', '\xA7', '\x4', 'R', '\xC4', 'm',
  '\x22', '\x1E', '\xA0', 'F', '\x24', '\x8', '\x16', '\x16',
  'v', '\xA', '6', '\xBA', 'J', '\x9A', '\x80', '\x8',
  'A', '\xB4', 'q', '\x85', 'X', '\x89', 'G', '\xB0',
  'I', '\xA9', 'Q', '\x24', '\xCD', '\xA6', '\x8', '\xA4',
  'H', 'c', '\x91', 'B', '\xB', '\xAF', 'V', '\xC1',
  'F', '\xB4', '\x15', '\xCF', '\x22', 'X', '\x98', '\xB',
  'T', 'H', '\x8A', 'd', '\x93', '\x8D', '\xFB', 'F',
  'g', '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f', 'v',
  'f', '\xDF', '\x7C', '\xEF', '\xE7', 'g', 'F', '\xA8',
  '\xD5', 'j', 'H', '\x24', '\x12', '\x2A', '\x0', '\x5',
  '\xBF', 'G', '\xD4', '\xEF', '\xF7', '\x2F', '6', '\xEC',
  '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA',
  '\xAF', 'I', '5', 'F', '\xAA', 'T', '\x5F', '\x9F',
  '\x22', 'A', '\x2A', '\x95', '\xA', '\x83', '\xE5', 'r',
  '9', 'd', '\xB3', 'Y', '\x96', '\x99', 'L', '\x6',
  '\xE9', 't', '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',
  '\xA7', '\xC4', 'b', '1', '\xB5', '\x5E', '\x0', '\x3',
  'h', '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X', 'R',
  '\x14', 'E', '6', 'S', '\x94', '\xCB', 'e', 'x',
  '\xBD', '\x5E', '\xAA', 'U', 'T', '\x23', 'L', '\xC0',
  '\xE0', '\xE2', '\xC1', '\x8F', '\x0', '\x9E', '\xBC', '\x9',
  'A', '\x7C', '\x3E', '\x1F', '\x83', 'D', '\x22', '\x11',
  '\xD5', 'T', '\x40', '\x3F', '8', '\x80', 'w', '\xE5',
  '3', '\x7', '\xB8', '\x5C', '\x2E', 'H', '\x92', '\x4',
  '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g', '\x98',
  '\xE9', '6', '\x1A', '\xA6', 'g', '\x15', '\x4', '\xE3',
  '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i', '\xB7', 'C',
  '\xAB', '\xEA', 'x', '\x2F', 'j', 'X', '\x92', '\xBB',
  '\x18', '\x20', '\x9F', '\xCF', '3', '\xC3', '\xB8', '\xE9',
  'N', '\xA7', '\xD3', 'l', 'J', '\x0', 'i', '6',
  '\x7C', '\x8E', '\xE1', '\xFE', 'V', '\x84', '\xE7', '\x3C',
  '\x9F', 'r', '\x2B', '\x3A', 'B', '\x7B', '7', 'f',
  'w', '\xAE', '\x8E', '\xE', '\xF3', '\xBD', 'R', '\xA9',
  'd', '\x2', 'B', '\xAF', '\x85', '2', 'f', 'F',
  '\xBA', '\xC', '\xD9', '\x9F', '\x1D', '\x9A', 'l', '\x22',
  '\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15',
  '\x90', '\x7', '\x93', '\xA2', '\x28', '\xA0', 'S', 'j',
  '\xB1', '\xB8', '\xDF', '\x29', '5', 'C', '\xE', '\x3F',
  'X', '\xFC', '\x98', '\xDA', 'y', 'j', 'P', '\x40',
  '\x0', '\x87', '\xAE', '\x1B', '\x17', 'B', '\xB4', '\x3A',
  '\x3F', '\xBE', 'y', '\xC7', '\xA', '\x26', '\xB6', '\xEE',
  '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
  '\xA', '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X', '\x0',
  '\x27', '\xEB', 'n', 'V', 'p', '\xBC', '\xD6', '\xCB',
  '\xD6', 'G', '\xAB', '\x3D', 'l', '\x7D', '\xB8', '\xD2',
  '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4',
  '\xEA', '\xCC', '\x2', 'N', '\xAE', '\x5E', 'p', '\x1A',
  '\xEC', '\xB3', '\x40', '9', '\xAC', '\xFE', '\xF2', '\x91',
  '\x89', 'g', '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7',
  'X', '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N', 'N',
  'b', 't', '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
  '\xEC', '\x86', '\x2', 'H', '\x26', '\x93', '\xD0', 'u',
  '\x1D', '\x7F', '\x9', '2', '\x95', '\xBF', '\x1F', '\xDB',
  '\xD7', 'c', '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF',
  '\x22', 'J', '\xC3', '\x87', '\x0', '\x3', '\x0', 'K',
  '\xBB', '\xF8', '\xD6', '\x2A', 'v', '\x98', 'I', '\x0',
  '\x0', '\x0', '\x0', 'I', 'E', 'N', 'D', '\xAE',
  'B', '\x60', '\x82',
};