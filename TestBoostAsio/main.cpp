#include <functional>
#include <iostream>
#include <boost/asio.hpp>

namespace
{
    class Printer
    {
    public:
        Printer(boost::asio::io_context& io)
            : timer_(io, boost::asio::chrono::seconds(1)),
            count_(0)
        {
            timer_.async_wait(std::bind(&Printer::print, this));
        }

        ~Printer()
        {
            std::cout << "Final count is " << count_ << std::endl;
        }

        void print()
        {
            if (count_ < 5)
            {
                std::cout << "Printer: " << count_ << std::endl;
                ++count_;

                timer_.expires_after(boost::asio::chrono::seconds(1));
                timer_.async_wait(std::bind(&Printer::print, this));
                std::cout << "Another code..." << std::endl;
            }
            else
            {
                std::cout << "Canceling timer..." << std::endl;
                timer_.cancel();
                static_cast<boost::asio::io_context&>(timer_.get_executor().context()).stop();
            }
        }

    private:
        boost::asio::steady_timer timer_;
        int count_;
    };
}

int main(int argc, char** argv)
{
	namespace asio = boost::asio;

    asio::io_context io;
    Printer p(io);
    io.run();
    std::cout << "Bye" << std::endl;
	return 0;
}
