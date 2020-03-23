#define soc_cv_av
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>


#include "hps_soc_system.h"
#include "fifo.h"
#include "hps_linux.h"

#define FIFO_FRAMING_FULL		  ((*(fifo_framing_status_ptr+1))& 1 ) 
#define FIFO_FRAMING_EMPTY	  ((*(fifo_framing_status_ptr+1))& 2 ) 

#define alt_write_word(dest, src)       (*ALT_CAST(volatile uint32_t *, (dest)) = (src))

volatile uint16_t * fifo_framing_transmit_ptr = NULL ;
volatile unsigned int * fifo_framing_status_ptr = NULL ;
volatile unsigned int * fpga_leds = NULL;
volatile unsigned int * fpga_switches = NULL;


void open_physical_memory_device() {
    // We need to access the system's physical memory so we can map it to user
    // space. We will use the /dev/mem file to do this. /dev/mem is a character
    // device file that is an image of the main memory of the computer. Byte
    // addresses in /dev/mem are interpreted as physical memory addresses.
    // Remember that you need to execute this program as ROOT in order to have
    // access to /dev/mem.

    fd_dev_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if(fd_dev_mem  == -1) {
        printf("ERROR: could not open \"/dev/mem\".\n");
        printf("    errno = %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
void close_physical_memory_device() {
    close(fd_dev_mem);
}


void mmap_fpga_peripherals() {
    h2f_lw_axi_master = mmap(NULL, h2f_lw_axi_master_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, h2f_lw_axi_master_ofst);
    if (h2f_lw_axi_master == MAP_FAILED) {
        printf("Error: h2f_lw_axi_master mmap() failed.\n");
        printf("    errno = %s\n", strerror(errno));
        close(fd_dev_mem);
        exit(EXIT_FAILURE);
    }
	h2f_axi_master = mmap(NULL, h2f_axi_master_span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd_dev_mem, h2f_axi_master_ofst);
    if (h2f_axi_master == MAP_FAILED) {
        printf("Error: h2f_lw_axi_master mmap() failed.\n");
        printf("    errno = %s\n", strerror(errno));
        close(fd_dev_mem);
        exit(EXIT_FAILURE);
    }
	fifo_framing_transmit_ptr = (unsigned int *) (h2f_axi_master + FIFO_TX_VIDEO_IN_BASE);
	fifo_framing_status_ptr = (unsigned int *)(h2f_lw_axi_master +  FIFO_TX_VIDEO_IN_CSR_BASE);
	fpga_leds =   (unsigned int *) (h2f_lw_axi_master +  HPS_FPGA_LEDS_BASE);
	fpga_switches = h2f_lw_axi_master + HPS_FPGA_SWITCHES_BASE;
}


void munmap_fpga_peripherals() {
    if (munmap(h2f_lw_axi_master, h2f_lw_axi_master_span) != 0) {
        printf("Error: h2f_lw_axi_master munmap() failed\n");
        printf("    errno = %s\n", strerror(errno));
        close(fd_dev_mem);
        exit(EXIT_FAILURE);
    }
	if (munmap(h2f_axi_master, h2f_axi_master_span) != 0) {
        printf("Error: h2f_lw_axi_master munmap() failed\n");
        printf("    errno = %s\n", strerror(errno));
        close(fd_dev_mem);
        exit(EXIT_FAILURE);
    }
    h2f_lw_axi_master = NULL;
	h2f_axi_master    = NULL;
    fpga_leds         = NULL;
	fpga_switches	  = NULL;
	fifo_framing_transmit_ptr = NULL ;
	fifo_framing_status_ptr = NULL ;

}


int send_data_fifo(char* data, int dataLen){

	printf("penis");
    if (!FIFO_FRAMING_FULL) {
		*fifo_framing_transmit_ptr=(int)data;
		printf("FIFO to framing block Empty value %d \n", FIFO_FRAMING_EMPTY);
		printf("FIFO to framing block Full value %d \n", FIFO_FRAMING_FULL);
		printf("FIFO to framing block fill level %d \n", *fifo_framing_status_ptr);
		
		
		return 1;
	}
	return 0;
	
	
}