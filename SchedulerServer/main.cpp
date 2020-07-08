#include <iostream>
#include <thread>
#include <cstdlib>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/thread.hpp>

#include <ahlib.h>

#include <fstream>

class tcp_connection
        : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new tcp_connection(io_service));
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        std::cout << "start" << std::endl;
        boost::asio::async_read(socket_, read_buffer,
                                boost::asio::transfer_exactly(4),
                                boost::bind(&tcp_connection::handle_size, shared_from_this(),
                                            boost::asio::placeholders::error));


    }

private:
    tcp_connection(boost::asio::io_service& io_service)
            : socket_(io_service)
    {
    }

    void handle_size(const boost::system::error_code& err){
        if (!err)
        {
            std::cout << "size" << std::endl;
            data_size = htonl(*((unsigned int*)boost::asio::buffer_cast<const void*>(read_buffer.data())));
            read_buffer.consume(4);
            boost::asio::async_read(socket_, read_buffer,
                                    boost::asio::transfer_exactly(data_size),
                                    boost::bind(&tcp_connection::handle_request, shared_from_this(),
                                                boost::asio::placeholders::error));
        }
        else if (err != boost::asio::error::eof)
        {
            std::cout << "Error: " << err << "\n";
        }
    }

    void handle_request(const boost::system::error_code& err){
        if (!err)
        {
            std::cout << "req" << std::endl;
            std::ofstream outfile;
            outfile.open("../instance.xml");
            if(!outfile.good()){
                std::cout << "Error: failed to open output file" << "\n";
            }
            outfile << boost::asio::buffer_cast<const char*>(read_buffer.data());
            outfile.close();

            /* TODO: Run solver */
            std::system("/home/joel/Documents/Work/Hercules/PREM/CP/XMLCPsolver/XMLCPsolver/build/XMLCPsolver ../instance.xml ../result.xml");
            /*
            FILE *in;
            char buff[512];
            in = popen("/home/joel/Documents/Work/Hercules/PREM/CP/XMLCPsolver/XMLCPsolver/build/XMLCPsolver ../instance.xml ../result.xml", "r");
            while (fgets(buff, sizeof(buff), in))
            {
                std::cout << buff;
            }
            */
            std::ifstream resultFile("../result.xml");
            if(!resultFile.good()){
                std::cout << "Error: failed to open input file" << "\n";
            }
            std::string resultString((std::istreambuf_iterator<char>(resultFile)),
                            std::istreambuf_iterator<char>());

            read_buffer.consume(data_size);

            data_size = htonl(resultString.size());
            std::string length((char *) &data_size, 4);

            message_ = length;
            message_ += resultString;

            boost::asio::async_write(socket_, boost::asio::buffer(message_, message_.size()),
                                     boost::bind(&tcp_connection::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
        }
        else if (err != boost::asio::error::eof)
        {
            std::cout << "Error: " << err << "\n";
        }
    }

    void handle_write(const boost::system::error_code& err,
                      size_t /*bytes_transferred*/)
    {
        if (!err)
        {
            boost::asio::async_read(socket_, read_buffer,
                                    boost::asio::transfer_exactly(4),
                                    boost::bind(&tcp_connection::handle_size, shared_from_this(),
                                                boost::asio::placeholders::error));
        }
        else if (err != boost::asio::error::eof)
        {
            std::cout << "Error: " << err << "\n";
        }
    }

    boost::asio::ip::tcp::socket socket_;
    std::string message_;
    boost::asio::streambuf read_buffer;
    unsigned int data_size;
};

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service)
            : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 4242))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        std::cout << "Accept" << std::endl;
        tcp_connection::pointer new_connection =
                tcp_connection::create(acceptor_.get_io_service());

        acceptor_.async_accept(new_connection->socket(),
                               boost::bind(&tcp_server::handle_accept, this, new_connection,
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

    boost::asio::ip::tcp::acceptor acceptor_;
};

void tcpServer(int port){
    try
    {
        boost::asio::io_service io_service;
        tcp_server server(io_service);
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Try to register in AH cloud" << std::endl;
    ArrowheadProvider provider("certificates.json", "provider.json");
    if(!provider.registerProvider()) std::cout << "Failed to register provider into AH cloud" << std::endl;
    provider.printProviderIds();
    std::thread serverThread(tcpServer, 4242);
    serverThread.join();
    return 0;
}
