#include <array>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

namespace {
    std::string make_daytime_string()
    {
        std::ostringstream oss;
        std::time_t now = std::time(nullptr);
        std::tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif

        oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S"); // Format: YYYY-MM-DD HH:MM:SS
        return oss.str();
    }

    constexpr auto IP_ADDRESS = "localhost";
} // namespace

class tcp_connection
  : public std::enable_shared_from_this<tcp_connection>
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

  void start()
  {
        message_ = make_daytime_string();

        boost::asio::async_write(socket_, boost::asio::buffer(message_),
            std::bind(&tcp_connection::handle_write, shared_from_this()));
  }

private:
    tcp_connection(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }

    void handle_write()
    {
    }

    tcp::socket socket_;
    std::string message_;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_context& io_context)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        start_accept();
    }

    private:
    void start_accept()
    {
        tcp_connection::pointer new_connection =
        tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            std::bind(&tcp_server::handle_accept, this, new_connection,
            boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection,
        const boost::system::error_code& error)
    {
        if (!error)
        {
        new_connection->start();
        }

        start_accept();
    }

    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

class udp_server
{
public:
    udp_server(boost::asio::io_context& io_context)
        : socket_(io_context, udp::endpoint(udp::v4(), 13))
    {
        start_receive();
    }

private:
    void start_receive()
    {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            std::bind(&udp_server::handle_receive, this,
            boost::asio::placeholders::error));
    }

    void handle_receive(const boost::system::error_code& error)
    {
        if (!error)
        {
        std::shared_ptr<std::string> message(
            new std::string(make_daytime_string()));

        socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
            std::bind(&udp_server::handle_send, this, message));

        start_receive();
        }
    }

    void handle_send(std::shared_ptr<std::string> /*message*/)
    {
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1> recv_buffer_;
};

void clientWorker()
{
    try
    {
        boost::asio::io_context io_context;

        udp::resolver resolver(io_context);
        udp::endpoint receiver_endpoint =
        *resolver.resolve(udp::v4(), IP_ADDRESS, "daytime").begin();

        udp::socket socket(io_context);
        socket.open(udp::v4());

        std::array<char, 1> send_buf  = {{ 0 }};
        socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);

        std::array<char, 128> recv_buf;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

        std::cout.write(recv_buf.data(), len);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void serverWorker()
{
    try
    {
        boost::asio::io_context io_context;
        tcp_server server1(io_context);
        udp_server server2(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: client <host>" << std::endl;
        return 1;
    }

    const std::string arg1 = argv[1];
    std::cout << "Argument: " << arg1 << std::endl;

    if (arg1 == "client")
    {
        clientWorker();
    }
    else if (arg1 == "server")
    {
        serverWorker();
    }
    else
    {
        std::cerr << "Unknown mode selected" << std::endl;
        return 1;
    }

    return 0;
}