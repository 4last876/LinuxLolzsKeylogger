#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <cstring>
#include <array>
#include <fstream>
#include <future>

using namespace boost::asio;
ip::tcp::endpoint im(ip::tcp::v4(),8083);

class session : public std::enable_shared_from_this<session> {

struct fileInfo{
char filename[30];
int filesize;
};

template<typename T> void clearVec(std::vector<T>& vec){
  vec.clear();
  vec.resize(64000);
}

  bool readiness = false;
  int totalWrite;
  ip::tcp::socket sock;
  std::vector<char> bufferRd;
  std::string bufferWr;
  fileInfo fileinfo;
  public:
  session(ip::tcp::socket&& sock_) : sock(std::move(sock_)), bufferRd(64000),bufferWr(""),totalWrite(0){
  }
  
   template<typename T> void send(T&& str){
    bufferWr = std::move(str);
    sock.async_write_some(buffer(bufferWr.c_str(),bufferWr.length()),std::bind(&session::handlerSend,shared_from_this(),std::placeholders::_1,std::placeholders::_2));
   }
 
    void handlerSend( const boost::system::error_code& err,std::size_t bytes_transferred){
    if(err || (bytes_transferred < 0)){
       std::cout << err.what() << std::endl;
   }
}

  void do_read(){
sock.async_receive(buffer(bufferRd),std::bind(&session::on_read,shared_from_this(),std::placeholders::_1,std::placeholders::_2));
  }

    void handlerprocess(){
     std::string str("fileInfo");
      std::vector<char> bstr(str.begin(),str.end());

      auto iter = std::search(bufferRd.begin(),bufferRd.end(),bstr.begin(),bstr.end());
      if(iter != bufferRd.end()){
        char label[10];
        sscanf(bufferRd.data(),"%s %s %d",label,fileinfo.filename,&fileinfo.filesize);
      }
        readiness = true;
        clearVec(bufferRd);
        std::ofstream ost(fileinfo.filename, std::ios::binary | std::ios::ate);
        ost.close();
        send(std::move(std::string("OK")));
        do_read();
    }

  void handlecreatefile(std::size_t bytes_transferre){
    totalWrite += bytes_transferre;
    std::ofstream ost;
    ost.open(fileinfo.filename, std::ios::binary | std::ios::ate);
    ost.write(bufferRd.data(),bytes_transferre);
    ost.close();
    if(totalWrite == fileinfo.filesize){
    std::cout << "успешно" << std::endl;
    send(std::move(std::string("EOF")));
    }else{
    do_read();
    }
  }

   void on_read( const boost::system::error_code& err,std::size_t bytes_transferred){
    if(err || (bytes_transferred < 0)){
       std::cout << err.what() << std::endl;
   }
   if(readiness){
    handlecreatefile(bytes_transferred);
   }else{
    handlerprocess();
   }
     
}
};

void do_accept(const boost::system::error_code& error,
    boost::asio::ip::tcp::socket peer, ip::tcp::acceptor& accep){
  std::cout << "новое подключени" << std::endl;
  std::shared_ptr<session> ses =  std::make_shared<session>(std::move(peer));
  ses->do_read();
   accep.async_accept(std::bind(do_accept,std::placeholders::_1,std::placeholders::_2,std::ref(accep)));
}


int main()
{
  io_context io;
  ip::tcp::acceptor accep(io,im);
  accep.async_accept(std::bind(do_accept,std::placeholders::_1,std::placeholders::_2,std::ref(accep)));
  io.run();

  return 0;
}