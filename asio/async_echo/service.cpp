#include<boost/asio.hpp>
#include<iostream>
#include<memory>
using namespace std;
using namespace boost::asio;

class Session :public enable_shared_from_this<Session>
{
public:
	Session(io_service& service):sock_(service){}

	void start()
	{
		do_read();
	}

	ip::tcp::socket& get_sock() { return sock_; }
private:

	void do_read()
	{
		auto self(shared_from_this());
		sock_.async_read_some(buffer(buf), [this,self](const boost::system::error_code& err,size_t bytes){
		
			if (!err)
				do_write(bytes);
		});
	}

	void do_write(size_t bytes)
	{
		auto self(shared_from_this());
		sock_.async_write_some(buffer(buf, bytes), [this,self](const boost::system::error_code& err,size_t bytes) {
			if (!err)
				do_read();
		});
	}

	ip::tcp::socket sock_;
	char buf[1024];
};

class Server
{
public:
	Server(io_service& service):service_(service),
		acc(service,ip::tcp::endpoint(ip::tcp::v4(),8001))
	{
		do_accept();
	}
private:
	void do_accept()
	{
		auto session = std::make_shared<Session>(service_);
		acc.async_accept(session->get_sock(),[this,session](const boost::system::error_code& err) {
			if(!err)
				session->start();
			do_accept();
		});
	}

	io_service& service_;
	ip::tcp::acceptor acc;
};

int main()
{
	io_service service;
	Server s(service);
	service.run();
	return 0;
}