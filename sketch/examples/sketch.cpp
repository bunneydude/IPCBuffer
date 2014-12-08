#include <iostream>
#include "IPCBuffer.h"

IPCBuffer myBuffer(1);
#define BUFFER_LEN 5
#define DATA_WIDTH 1

int main(){
	myBuffer = myBuffer.open(BUFFER_LEN, DATA_WIDTH);

	std::cout << "Hello world" << std::endl;
}
