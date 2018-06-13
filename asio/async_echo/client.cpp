#include<boost/asio.hpp>
#include<thread>
#include<memory>
#include<iostream>
using namespace std;
using namespace boost::asio;
io_service service;
ip::tcp::endpoint ed(ip::address::from_string("127.0.0.1"), 8001);

class Client :public enable_shared_from_this<Client>
{
public:
	Client(const string& msg) :sock_(service), msg_(msg + "\n") { }

	void start()
	{
		auto self = shared_from_this();
		sock_.async_connect(ed, [self,this](const boost::system::error_code& err) {
			if (!err)
				do_write();
		});
	}

private:

	void do_write()
	{
		auto self = shared_from_this();
		std::copy(msg_.begin(), msg_.end(), buf);
		async_write(sock_, buffer(buf), [this,self](const boost::system::error_code& err,size_t bytes) {
			if (!err)
				do_read();
		});
	}

	void do_read()
	{
		auto self = shared_from_this();
		async_read(sock_, buffer(buf), [this, self](const boost::system::error_code& err, size_t bytes)->size_t {
			if (err)
				return 0;
			bool found = std::find(buf, buf + bytes, '\n') < buf + bytes;
			return found ? 0 : 1;
		}, 
		[this, self](const boost::system::error_code& err,size_t bytes) {
			printf("%s\n", string(buf, bytes - 1).c_str());
			sock_.close();
		});
	}

	ip::tcp::socket sock_;
	string msg_;
	char buf[1024];
};

int main()
{
	std::thread threads[4];
	char* messages[] = { "join says hi","so does James", "Lucy just go home","Asio is fun"};
	for (int i = 0; i < 4; ++i)
	{
		auto client = std::make_shared<Client>(messages[i]);
		threads[i] = thread([client] { client->start(); });
	}

	service.run();
	for (int i = 0; i < 4; ++i)
	{
		threads[i].join();
	}
	return 0;
}