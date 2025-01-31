#include <functional>
#include <iostream>
#include <boost/asio.hpp>

namespace
{
	void print(const boost::system::error_code& /*e*/, boost::asio::steady_timer* t, int* count)
	{
		if (*count < 5)
		{
			std::cout << *count << std::endl;
			++(*count);

			t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
			t->async_wait(std::bind(print,	boost::asio::placeholders::error, t, count));
		}
	}
}

int main(int argc, char** argv)
{
	namespace asio = boost::asio;

	asio::io_context io;
	int count = 0;
	asio::steady_timer t(io, asio::chrono::seconds(1));
	t.async_wait(std::bind(print, asio::placeholders::error, &t, &count));
	io.run();
	std::cout << "Final count is " << count << std::endl;
	return 0;
}
