/*
 *    Copyright (c) 2019 - 2020, Thales DIS Singapore, Inc
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

#include "LSerial.h"
#include <cstdio>
#include <cstring>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>

//#define SERIAL_DEBUG

LSerial::LSerial(void) {
	m_uart = -1;
}

LSerial::~LSerial(void) {
}


bool LSerial::start(const char *modem_port) {
#ifdef SERIAL_DEBUG
	printf("Opening serial port...");
#endif

	const char* uart = (const char*) modem_port; //"/dev/ttyACM0";
	int port;
	struct termios serial;
	memset(&serial, 0, sizeof(serial));

	if((m_uart = open(uart, O_RDWR | O_NOCTTY | O_NONBLOCK)) >= 0) {
		tcgetattr(m_uart, &serial);

		serial.c_cflag = CS8 | HUPCL | CREAD | CLOCAL;
		serial.c_iflag = IGNPAR;
		serial.c_oflag = 0;
		serial.c_lflag = 0;
		serial.c_cc[VMIN] = 0;
		serial.c_cc[VTIME] = 0;
		// Set baud, e.g.:
		cfsetispeed(&serial, B115200);
		cfsetospeed(&serial, B115200);
		
		tcsetattr(m_uart, TCSANOW, &serial); // Apply configuration
		tcflush(m_uart, TCIOFLUSH);
		ioctl(m_uart, TIOCEXCL);

#ifdef SERIAL_DEBUG
		printf("Found serial %s %d\r\n", uart, m_uart);
#endif

		return true;
	}
	
	return false;
}

bool LSerial::send(char* data, unsigned long int toWrite, unsigned long  int* size) {
	unsigned long int i;
	int w;
	
	if(m_uart < 0) {
		return false;
	}
	
	for(i=0; i<toWrite;) {
		w = write(m_uart, &data[i], (toWrite - i));
		if(w == -1) {
			return false;
		}
		else if(w) {
			i += w;
		}
		
	}

	*size = toWrite;

	
	#ifdef SERIAL_DEBUG
	if(*size) {
		unsigned long int i;
		printf("> ");
		for(i=0; i<*size; i++) {
			if((data[i] != '\r') && (data[i] != '\n')) {
				printf("%c", data[i]);
			}
		}
		printf("\n");

	}
	#endif
	
	return true;
}

bool LSerial::recv(char* data, unsigned long int toRead, unsigned long int* size, int ms_timeout) {
	unsigned long int i = 0;
    int r;
    fd_set rfds;
    struct timeval tv;
    int retval;

    while (i == 0) {
        FD_ZERO(&rfds);
        FD_SET(m_uart, &rfds);
        // Set timeout
        tv.tv_sec = ms_timeout / 1000;
        tv.tv_usec = (ms_timeout % 1000) * 1000;
        retval = select(m_uart + 1, &rfds, NULL, NULL, &tv);
        if (retval == -1) {
            printf("select() error");
            return false;  
        } 
		else if (retval == 0) {
            printf("recv timeout\n");
            return false;
        }
        // Data is available
		if (FD_ISSET(m_uart, &rfds))
		{ 
			r = read(m_uart, &data[i], toRead);
			if (r == 0)
			{
				printf("read() 0 try again, FD_ISSET = %d\n", FD_ISSET(m_uart, &rfds));
				usleep(1000);
			}
			else if ((r == -1) && (errno == EAGAIN))
			{
				printf("read() error with code %d\n", errno);
				usleep(1000);
			}
			else
			{
				i += r;
			}		
		}
		else
		{
			printf("nothing to read");
		}
        
    }
    *size = i;
#ifdef SERIAL_DEBUG
	if(*size) {
		unsigned long int i;
		printf("< ");
		for(i=0; i<*size; i++) {
			if((data[i] != '\r') && (data[i] != '\n')) {
				printf("%c", data[i]);
			}
		}
		printf("\n");
	}
	#endif
	
	return true;
}

bool LSerial::stop(void) {
#ifdef SERIAL_DEBUG
	printf("Closing serial port...");
	if(m_uart >= 0) {
        printf("%d ", m_uart);
		close(m_uart);
	}
	printf("OK\n");
#else
	if(m_uart >= 0)
		close(m_uart);
#endif
	return true;
}
