#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "server.h"
#include "connection.h"
#include "fifo.h"

struct sockaddr_in clientAddr;
int connectionStatus;
pthread_t fifoWriteThreadId;

enum connection_status{
    WAITING_INIT,
    RECV_VIDEO
};

void run_server(int serverPort, cbuf_handle_t video_buf){
    init_server_socket(serverPort);
    
    pthread_create(&fifoWriteThreadId, NULL, &fifo_write_thread, video_buf);
    printf("Starting in server mode on port %d\n", serverPort);
    connectionStatus = WAITING_INIT;
	
	//Open physical memory 
	open_physical_memory_device();
    mmap_fpga_peripherals();
	
    while(1){
        switch(connectionStatus){
            case WAITING_INIT:
                wait_init(&clientAddr);
                break;
            
            case RECV_VIDEO:
                recv_video(video_buf);
                break;
        }
    }
	pthread_join(fifoWriteThreadId, NULL);
	//close physical memory
	munmap_fpga_peripherals();
    close_physical_memory_device();
}

void wait_init(struct sockaddr_in* client){
    char data[MAX_PACKET_SIZE];
    
    while(connectionStatus == WAITING_INIT){
        recv_data(client, data);
        if(data[0] == INIT){
            send_packet_type(client, INIT_ACK);
            connectionStatus = RECV_VIDEO;
            printf("INIT received\n");
        }
    }
}

void recv_video(cbuf_handle_t video_buffer){
    struct sockaddr_in recvAddr;
    char data[MAX_PACKET_SIZE];
    int dataLen = 0;
    int recvLen = 0;
    int fifoStatus = 0;
    
    while(1){
        recvLen = recv_data(&recvAddr, data);
        
        int type = data[0];

        if(type == TERMINATE){
            printf("received TERMINATE, waiting for new client\n");
            connectionStatus = WAITING_INIT;
            break;
        }

        if(recvLen < 3){
            continue;
        }
        

        unsigned short len = 0;
        memcpy(&len, &data[1], 2);
        unsigned int dataLen = (unsigned int) len;

        if(dataLen <= 0 || type != VIDEO_DATA){
            continue;
        }
        
        
        if(addrMatch(&recvAddr, &clientAddr)){
            int bufferSpace = get_space(video_buffer);
            if(bufferSpace >= dataLen){
                send_data_buffer(&data[3], dataLen, video_buffer);
            }
            else{/*
                printf("buffer is full\n");
                printf("space left: %d\n", bufferSpace);
                printf("space required: %d\n", dataLen);*/
            }
        }
        else{
            send_packet_type(&clientAddr, TERMINATE);
        }
    }
}

