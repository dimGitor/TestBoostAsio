#include <functional>
#include <iostream>
#include <boost/asio.hpp>

namespace
{
	void print(const boost::system::error_code& /*e*/)
	{
		std::cout << "Hello, world!" << std::endl;
	}
}

int main(int argc, char** argv)
{
	namespace asio = boost::asio;

	asio::io_context io;
	asio::steady_timer t(io, asio::chrono::seconds(5));
	t.async_wait(&print);
	io.run();
	return 0;
}
