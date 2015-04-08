#include <iostream>
#include "IPCBuffer.h"

IPCBuffer myBuffer(3);
#define BUFFER_LEN 255
#define DATA_WIDTH 1

int data = 1;

int main(){
    std::cout << "Open buffer" << std::endl;
	data = myBuffer.open(BUFFER_LEN, DATA_WIDTH);
    std::cout << "Return code " << data << std::endl;
    data = 1;
    
    while(data != 0){
        myBuffer.read(&data);
    	std::cout << "Got data: " << data << std::endl;
    }
    myBuffer.close();
}
