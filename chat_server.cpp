#include <boost/asio.hpp>
#include <optional>
#include <queue>
#include <unordered_set>
#include <iostream>
#include <random>
#include <iostream>
#include <memory>
#include <functional>

namespace io = boost::asio;
using tcp = io::ip::tcp;
using error_code = boost::system::error_code;

using message_handler = std::function<void (std::string)>;
using error_handler = std::function<void ()>;

class session : public std::enable_shared_from_this<session>
{
public:
    typedef std::shared_ptr<session> pointer;

    session(boost::asio::ip::tcp::socket&& socket)
    : socket(std::move(socket))
    {
    }

   void start(message_handler&& on_message, error_handler&& on_error)
  {
    this->on_message = std::move(on_message);
    this->on_error = std::move(on_error);
    async_read();
  }

  void post(std::string const& message)
{
    bool idle = outgoing.empty();
    outgoing.push(message);

    if(idle)
    {
        async_write();
    }
}

void async_read()
{
    io::async_read_until(socket, streambuf, "\n", [self = shared_from_this()] (error_code error, std::size_t bytes_transferred)
    {
        self->on_read(error, bytes_transferred);
    });
}

void async_write()
{
    io::async_write(socket, io::buffer(outgoing.front()), [self = shared_from_this()] (error_code error, std::size_t bytes_transferred)
    {
        self->on_write(error, bytes_transferred);
    });
}

void on_read(error_code error, std::size_t bytes_transferred)
{
    if(!error)
    {
        std::stringstream message;
        message << socket.remote_endpoint(error) << ": " << std::istream(&streambuf).rdbuf();
        streambuf.consume(bytes_transferred);
        on_message(message.str());
        async_read();
    }
    else
    {
        socket.close(error);
        on_error();
    }
}

void on_write(error_code error, std::size_t bytes_transferred)
{
    if(!error)
    {
        outgoing.pop();

        if(!outgoing.empty())
        {
            async_write();
        }
    }
    else
    {
        socket.close(error);
        on_error();
    }
}

private:
    tcp::socket socket; // Client's socket
    io::streambuf streambuf; // Streambuf for incoming data
    std::queue<std::string> outgoing; // The queue of outgoing messages
    message_handler on_message; // Message handler
    error_handler on_error; // Error handler
};

class server
{
public:

    server(boost::asio::io_context& io_context, std::uint16_t port)
    : io_context(io_context)
    , acceptor  (io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {
    }

  void post(std::string const& message)
  {
    for(auto& client : clients)
    {
        client->post(message);
    }
  }

   void async_accept()
{
    socket.emplace(io_context);

    acceptor.async_accept(*socket, [&] (error_code error)
    {
        auto client = std::make_shared<session>(std::move(*socket));
        client->post("Welcome to chat\n\r");
        post("We have a newcomer\n\r");
        clients.insert(client);
        client->start
        (
            std::bind(&server::post, this,  std::placeholders::_1 ),
            [&, weak = std::weak_ptr(client)]
            {
                if(auto shared = weak.lock(); shared && clients.erase(shared))
                {
                    post("We are one less\n\r");
                }
            }
        );

        async_accept();
    });
}

private:
  io::io_context& io_context;
  tcp::acceptor acceptor;
  std::optional<tcp::socket> socket;
  std::unordered_set<session::pointer> clients; // A set of connected clients

};

int main()
{
    boost::asio::io_context io_context;
    server srv(io_context, 15001);
    srv.async_accept();
    io_context.run();
    return 0;
}

