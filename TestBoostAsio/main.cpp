#include <ctime>
#include <iostream>
#include <string>
#include <array>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace {
    std::string make_daytime_string()
    {
        using namespace std; // For time_t, time and ctime;
        time_t now = time(0);
        return ctime(&now);
    }

    constexpr auto IP_ADDRESS = "localhost";
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    std::cout << argv[1] << std::endl;
    const std::string arg1 = argv[1];

    if (arg1 == "client")
    {
        try
        {
            boost::asio::io_context io_context;

            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = resolver.resolve(IP_ADDRESS, "daytime");

            tcp::socket socket(io_context);
            boost::asio::connect(socket, endpoints);

            for (;;)
            {
                std::array<char, 128> buf;
                boost::system::error_code error;

                const size_t len = socket.read_some(boost::asio::buffer(buf), error);

                if (error == boost::asio::error::eof)
                {
                    std::cerr << "Connection closed cleanly by peer." << std::endl;
                    break;
                }
                else if (error)
                {
                    throw boost::system::system_error(error); // Some other error.
                }

                std::cout.write(buf.data(), len);
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    else if (arg1 == "server")
    {
        try
        {
            boost::asio::io_context io_context;
            tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

            for (;;)
            {
                tcp::socket socket(io_context);
                acceptor.accept(socket);

                std::string message = make_daytime_string();

                boost::system::error_code ignored_error;
                std::cout << "will send: " << message << std::endl;
                boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
            }
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    std::cout << "Bye-bye!" << std::endl;
    return 0;
}