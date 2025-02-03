#include <array>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <hiredis/hiredis.h>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

namespace {
    constexpr auto REDIS_HOST = "172.24.206.9";
    constexpr auto REDIS_PORT = 6379;
    constexpr auto REDIS_KEY  = "daytime";
    constexpr auto SERVER_PORT = 5000;

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
        oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    bool store_in_redis(const std::string& value)
    {
        std::cout << "Connecting to Redis..." << std::endl;
        redisContext* context = redisConnect(REDIS_HOST, REDIS_PORT);
        if (!context || context->err)
        {
            std::cerr << "Redis connection error: " << (context ? context->errstr : "Unknown") << std::endl;
            return false;
        }
        std::cout << "Connected to Redis, storing value: " << value << std::endl;
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", REDIS_KEY, value.c_str());
        bool success = (reply != nullptr);
        if (reply) 
            freeReplyObject(reply);
        redisFree(context);
        return success;
    }

    std::string fetch_from_redis()
    {
        std::cout << "Fetching from Redis..." << std::endl;
        redisContext* context = redisConnect(REDIS_HOST, REDIS_PORT);
        if (!context || context->err)
        {
            std::cerr << "Redis connection error: " << (context ? context->errstr : "Unknown") << std::endl;
            return "Redis Error";
        }

        redisReply* reply = (redisReply*)redisCommand(context, "GET %s", REDIS_KEY);
        std::string value = (reply && reply->type == REDIS_REPLY_STRING) ? reply->str : "No Data";
        std::cout << "Fetched value: " << value << std::endl;
        if (reply) 
            freeReplyObject(reply);
        redisFree(context);
        return value;
    }

    class udp_server
    {
    public:
        udp_server(boost::asio::io_context& io_context)
            : socket_(io_context, udp::endpoint(boost::asio::ip::address_v4::any(), SERVER_PORT))
        {
            std::cout << "UDP Server listening on port " << SERVER_PORT << " on all interfaces" << std::endl;
            start_receive();
        }

    private:
        void start_receive()
        {
            socket_.async_receive_from(
                boost::asio::buffer(recv_buffer_), remote_endpoint_,
                std::bind(&udp_server::handle_receive, this, std::placeholders::_1));
        }

        void handle_receive(const boost::system::error_code& error)
        {
            if (!error)
            {
                std::cout << "[UDP Server] Received request from: " 
                        << remote_endpoint_.address().to_string() << ":" 
                        << remote_endpoint_.port() << std::endl;

                std::string redis_value = fetch_from_redis();
                std::cout << "[Redis] Retrieved value: " << redis_value << std::endl;

                std::shared_ptr<std::string> message(new std::string(redis_value));
                socket_.async_send_to(boost::asio::buffer(*message), remote_endpoint_,
                    std::bind(&udp_server::handle_send, this, message));
                std::cout << "[UDP Server] Sent response to client." << std::endl;

                start_receive();
            }
            else
            {
                std::cerr << "[UDP Server] Receive error: " << error.message() << std::endl;
            }
        }

        void handle_send(std::shared_ptr<std::string>) {}

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
            udp::endpoint receiver_endpoint = *resolver.resolve(udp::v4(), "127.0.0.1", std::to_string(SERVER_PORT)).begin();

            udp::socket socket(io_context);
            socket.open(udp::v4());

            std::array<char, 1> send_buf = {{ 0 }};
            std::cout << "[Client] Sending request to server..." << std::endl;
            socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);
            
            std::array<char, 128> recv_buf;
            udp::endpoint sender_endpoint;
            size_t len = socket.receive_from(boost::asio::buffer(recv_buf), sender_endpoint);

            std::cout << "[Client] Received response from server: ";
            std::cout.write(recv_buf.data(), len);
            std::cout << std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << "[Client] Error: " << e.what() << std::endl;
        }
    }

    void serverWorker()
    {
        try
        {
            if (!store_in_redis(make_daytime_string())) 
            {
                std::cerr << "Failed to store data in Redis." << std::endl;
                return;
            }

            boost::asio::io_context io_context;
            udp_server server(io_context);
            std::cout << "Server is running, waiting for connections..." << std::endl;
            io_context.run();
        }
        catch (std::exception& e)
        {
            std::cerr << "Server error: " << e.what() << std::endl;
        }
    }
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: program <client|server>" << std::endl;
        return 1;
    }

    const std::string mode = argv[1];

    if (mode == "server")
    {
        serverWorker();
    }
    else if (mode == "client")
    {
        clientWorker();
    }
    else
    {
        std::cerr << "Unknown mode!" << std::endl;
        return 1;
    }

    return 0;
}
