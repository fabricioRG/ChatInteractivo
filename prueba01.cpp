#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <thread>         // std::thread
#include <mutex>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdlib>
#include <list>


// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

using namespace std;

// Create new termios struc, we call it 'tty' for convention
struct termios tty;

static int serial_port = 0;
static int num_bytes = 0;
static string SEPARATOR = "--------------";

static list<string> messagesRemoto;
static list<string> messagesLocal;

int listen (void);
int writeInformation (void);
int printMessages (void);

int listen (){

  string messageRemoto = "";

  // Allocate memory for read buffer, set size according to your needs
  char read_buf [256];

  // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.

do {

  // Normally you wouldn't do this memset() call, but since we will just receive
  // ASCII data for this example, we'll set everything to 0 so we can
  // call printf() easily.
  memset(&read_buf, '\0', sizeof(read_buf));

  // Read bytes. The behaviour of read() (e.g. does it block?,
  // how long does it block for?) depends on the configuration
  // settings above, specifically VMIN and VTIME

  num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

  if (num_bytes < 0) {

      printf("Error de lectura: %s. Saliendo...\n", strerror(errno));
      exit(0);
      //return 1;


  } else if (num_bytes > 0){

    messageRemoto = read_buf;

    if(messagesRemoto.size() == 50){
      messagesRemoto.pop_front();
    }

    messagesRemoto.push_back(messageRemoto);

    printMessages();

  }

} while (num_bytes >= 0);

  /*if (num_bytes > 0){
    messageRemoto = read_buf;
    messagesRemoto.push_back(messageRemoto);

    // Here we assume we received ASCII data, but you might be sending raw bytes (in that case, don't try and
    // print it to the screen like this!)
    //printf("Read %i bytes. Received message: %s\n", num_bytes, read_buf);
    list<string>::iterator it = messagesRemoto.begin();
    while( it != messagesRemoto.end() ){
        cout << "Message: " << *it++ << endl;
    }
  }*/
}

int writeInformation (){

  while(true){

    string messageLocal = "";
    getline(cin, messageLocal);
    //cin >> messageLocal;
    int sizeMessageLocal = messageLocal.length();
    //cout << "size: " <<  sizeMessageLocal << ", Message: " << messageLocal << endl;
    //cout << "num_bytes: " << num_bytes << endl;

    write(serial_port, messageLocal.c_str(), sizeMessageLocal);

    if(messagesLocal.size() == 50){
      messagesLocal.pop_front();
    }

    messagesLocal.push_back(messageLocal);
    printMessages();

  }
}

int printMessages(){

  system("clear");
  /*Remoto Messages*/
  cout << "+" << SEPARATOR << "+" << SEPARATOR << "+" << endl;
  cout << "| " << "Remoto (" << messagesRemoto.size() << "): " << "|" << endl;
  cout << "+" << SEPARATOR << "+" << endl;

  list<string>::iterator it = messagesRemoto.begin();
  while( it != messagesRemoto.end() ){
      cout << "|" << " > " << *it++ << endl;
  }

  cout << "+" << SEPARATOR << "+" << SEPARATOR << "+" << endl;

  /*Local messages*/
  cout << "| " << "Local (" << messagesLocal.size() << "): " << " |" << endl;

  cout << "+" << SEPARATOR << "+" << endl;

  list<string>::iterator it2 = messagesLocal.begin();
  while( it2 != messagesLocal.end() ){
      cout << "| " << "> " << *it2++ << endl;
  }

  cout << "+" << SEPARATOR << SEPARATOR << "-+" << endl;

}

int main() {
  // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  serial_port = open("/dev/ttyUSB0", O_RDWR);

  // Read in existing settings, and handle any error
  if (tcgetattr( serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  std::thread first (listen);
  std::thread second (writeInformation);

  first.join();
  second.join();


  close(serial_port);
  return 0; // success
}
