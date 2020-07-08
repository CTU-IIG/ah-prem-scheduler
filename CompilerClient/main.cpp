#include <iostream>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <fstream>

#include <ahlib.h>

using boost::asio::ip::tcp;

class tcp_client
{
public:
    tcp_client(
            boost::asio::io_service& io_service,
            const std::string& host,
            const std::string& port
    ) : io_service_(io_service), socket_(io_service, boost::asio::ip::tcp::tcp::endpoint(boost::asio::ip::tcp::tcp::v4(), 0))
    {
        boost::asio::ip::tcp::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::tcp::resolver::query query(boost::asio::ip::tcp::tcp::v4(), host, port);
        boost::asio::ip::tcp::tcp::resolver::iterator iter = resolver.resolve(query);
        endpoint_ = *iter;
        boost::asio::connect(socket_, iter);
    }

    ~tcp_client()
    {
        socket_.close();
    }

    void do_it()
    {
        send_request();
        receive_response();
    }

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::tcp::socket socket_;
    boost::asio::ip::tcp::tcp::endpoint endpoint_;
    boost::array<char, 2048> recv_buf;
    boost::asio::ip::tcp::endpoint sender_endpoint;
    boost::system::error_code error;
    std::string message_;

    boost::asio::streambuf read_buffer;

    void receive_response() {
        boost::asio::read(socket_, read_buffer, boost::asio::transfer_exactly(4), error);
        if (!error)
        {
            auto data_size = htonl(*((unsigned int*)boost::asio::buffer_cast<const void*>(read_buffer.data())));
            read_buffer.consume(4);
            boost::asio::read(socket_, read_buffer, boost::asio::transfer_exactly(data_size), error);
            std::ofstream outfile;
            outfile.open("../result.xml");
            if(!outfile.good()){
                std::cout << "Error: failed to open output file" << "\n";
            }
            outfile << boost::asio::buffer_cast<const char*>(read_buffer.data());
            outfile.close();
        }
        else if (error != boost::asio::error::eof)
        {
            std::cout << "Error: " << error << "\n";
        }
    }

    void send_request() {
        std::ifstream resultFile("instance.xml");
        if(!resultFile.good()){
            std::cout << "Error: failed to open input file" << "\n";
        }
        std::string resultString((std::istreambuf_iterator<char>(resultFile)),
                                 std::istreambuf_iterator<char>());

        auto data_size = htonl(resultString.size());
        std::string length((char *) &data_size, 4);

        message_ = length;
        message_ += resultString;

        boost::asio::write(socket_, boost::asio::buffer(message_, message_.size()), error);
        if (error == boost::asio::error::eof)
            std::cout << "Connection closed by peer." << std::endl;
        else if (error)
            throw boost::system::system_error(error); // Some other error.
    }

};

int main() {
    std::cout << "Trying to register consumer into AH" << std::endl;
    ArrowheadConsumer consumer("certificates.json", "consumer.json");
    ProviderInfo info = consumer.findProvider();
    std::cout << "We will try to connect the server: " << info.serverAddressIPV4 << ":" << std::to_string(info.serverPort) << std::endl;
    try
    {
        boost::asio::io_service io_service;
        tcp_client client(io_service, info.serverAddressIPV4, std::to_string(info.serverPort));
        client.do_it();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
