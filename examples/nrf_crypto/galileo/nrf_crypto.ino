#include "nrf_crypto.h"
#include <SPI.h>
#include <stdint.h>

#include <nrf24.h>
#include <nRF24L01.h>
#include <IPCBuffer.h>


// state
uint8_t hbt_output = 0;
//counters
volatile uint16_t task_hbt = TASK_HBT_PERIOD;
//-----------------------------------
// Function Prototypes
//-----------------------------------

void nrf24_ce_digitalWrite(uint8_t state){
      digitalWrite(RADIO1_CE,state);
}

void nrf24_csn_digitalWrite(uint8_t state){
      digitalWrite(RADIO1_CSN,state);
}

uint8_t rx_data_array[16];
uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};


uint8_t spi_transfer(uint8_t tx)
{
    uint8_t rx = 0;    
    rx = SPI.transfer(tx);
    return rx;
}
    
IPCBuffer myBuffer(1); //create shared memory buffer  
    
void setup() {                
  
  system("telnetd -l /bin/sh"); //set up Telnet
  system("ifconfig eth0 169.254.1.1 netmask 255.255.0.0 up");

  //initialize pins
  pinMode(HBT_LED, OUTPUT);   
  pinMode(RADIO1_CE, OUTPUT);
  digitalWrite(RADIO1_CE, LOW);     
  pinMode(RADIO1_CSN, OUTPUT);
  digitalWrite(RADIO1_CSN, HIGH);   
  
  Serial.begin(SERIAL_BAUD);

  SPI.begin();
  nrf24_init();
  nrf24_config(2,16);
  nrf24_tx_address(rx_address); //backwards looking but is fine
  nrf24_rx_address(tx_address);    

  uint8_t temp = 0;
  temp = myBuffer.open(QUEUE_LENGTH, QUEUE_WIDTH);
}


void loop() {
  
  if(1 == task_hbt){
    task_hbt = TASK_HBT_PERIOD;
    digitalWrite(HBT_LED, hbt_output^=0x01);     
  }

  if(nrf24_dataReady()){      
      nrf24_getData(rx_data_array);         
      
      uint8_t temp;     
      temp = myBuffer.write(rx_data_array);
  }

  delay(COUNTER_RATE);

  if(1 < task_hbt){task_hbt--;}
  //tasks w/ counters of 0 are not scheduled
  //tasks are set to run when their counter reaches 1
}
