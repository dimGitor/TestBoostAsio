#include <iostream>
#include <boost/asio.hpp>

#include "client.h"
#include "server.h"

namespace {
	constexpr auto ServerApplication = "server";
	constexpr auto ClientApplication = "client";
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Choose client or server!" << std::endl;
		return 1;
	}

	using asio = boost::asio;

	asio::io_context io;
	asio::steady_timer t(io, asio::chrono::seconds(5));
	t.wait();
	return 0;
}
