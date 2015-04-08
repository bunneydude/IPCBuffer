#include <stdint.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <iostream>
#include <errno.h>
#include "IPCBuffer.h"

#ifdef EDISON
#include <stdlib.h>
#else
#include "Arduino.h"
#endif


IPCBuffer::IPCBuffer(uint8_t id){
	_id = id;
	system("touch /sketch/k_lock.txt");
	system("touch /sketch/k_shm.txt");
	system("touch /sketch/k_data.txt");
	system("touch /sketch/k_space.txt");	
}

uint8_t IPCBuffer::startMem(uint32_t* shmid, key_t key, uint8_t** mem, size_t size){
	// Setup shared memory
	if ((*shmid = shmget(key, size, IPC_CREAT | 0666)) < 0){
        std::cout << "First SHM call failed" << std::endl;
		return 1;
	}
	// Attached shared memory
	if ((*mem = (uint8_t *)shmat(*shmid, NULL, 0)) == (uint8_t *) -1){
        std::cout << "Second SHM call failed: " << errno << std::endl;
        std::cout << strerror(errno) << std::endl;
		return 2;
	}
	return 0;
}


uint8_t IPCBuffer::startSem(uint32_t* sid, key_t key, uint32_t val){
	// Setup semaphore
	if ((*sid = semget(key, 1, IPC_CREAT | 0660)) < 0){
		return 1;
	}
	semopts.val = val;
	// Initialize count
	if ((semctl(*sid, 0, SETVAL, semopts)) < 0){
		return 2;
	}
	return 0;
}


uint8_t IPCBuffer::open(size_t length, size_t width){

	_width = width;
	_length = length;

	key_t key_s_lock = ftok("/sketch/k_lock.txt", _id);
	key_t key_shm_queue = ftok("/sketch/k_shm.txt", _id);
	key_t key_s_data = ftok("/sketch/k_data.txt", _id);
	key_t key_s_space = ftok("/sketch/k_space.txt", _id);

	uint8_t temp;

	temp = startMem(&shmid_queue, key_shm_queue, &shared_memory, length*width+sizeof(size_t)*4);
    std::cout << "StartMem returned " << temp << std::endl;

	//set up shared memory header before exposing semaphores
	//python scripts will fail if they try to run at this point because
	//the semaphores don't exist yet.
	//this elminates a race condition where you start the sketch and a python
	//script at nearly the same time
	*(shared_memory + 0*sizeof(size_t)) = 0; //head
	*(shared_memory + 1*sizeof(size_t)) = 0; //tail
	*(shared_memory + 2*sizeof(size_t)) = width;	
	*(shared_memory + 3*sizeof(size_t)) = length;

	temp = startSem(&sid_lock, key_s_lock, 1);	
    std::cout << "lock startSem returned " << temp << std::endl;
	
    temp = startSem(&sid_data, key_s_data, 0);
    std::cout << "data startSem returned " << temp << std::endl;
	
    temp = startSem(&sid_space, key_s_space, length-1);
    std::cout << "space startSem returned " << temp << std::endl;

	return 0;
}


uint8_t IPCBuffer::write(void* source){
	//block if no space
	//only allowed to write 1 _width's worth of bytes per call
	//otherwise interpretation of sem_space would be broken
	//sem_space protects against writing over un-read data

	semop(sid_space, &sem_down,1); //make sure room for data
	semop(sid_lock, &sem_down,1); //protect against other readers/writers

	//enter critical section
	size_t head = *(shared_memory+0*sizeof(size_t)); //get head value
	memcpy(shared_memory + head*_width + 4*sizeof(size_t), source, _width); 
	*(shared_memory+0*sizeof(size_t)) = (head+1) % _length;
	//exit critical section

	semop(sid_lock, &sem_up,1); //allow other readers/writers to make progress
	semop(sid_data, &sem_up,1); //indicate new data in buffer

	return 0;
}

uint8_t IPCBuffer::read(void* dest){

	semop(sid_data, &sem_down,1); //make sure room for data
	semop(sid_lock, &sem_down,1); //protect against other readers/writers

	//enter critical section
	size_t tail = *(shared_memory+1*sizeof(size_t)); 
	memcpy(dest, shared_memory + tail*_width + 4*sizeof(size_t), _width); 
	*(shared_memory+1*sizeof(size_t)) = (tail+1) % _length;
	//exit critical section

	semop(sid_lock, &sem_up,1); //allow other readers/writers to make progress
	semop(sid_space, &sem_up,1); //indicate new data in buffer

	return 0;
}

uint8_t IPCBuffer::available(){
   //read from tail, write at head
   //empty if head == tail
   uint8_t count = 0;
   semop(sid_lock, &sem_down,1); //protect against other readers/writers
   // enter critical section

   size_t head = *(shared_memory+0*sizeof(size_t));   
   size_t tail = *(shared_memory+1*sizeof(size_t)); 

   if(head >= tail){
      count = head - tail;
   }else{
      count = head + _length - tail;
   }

   // exit critical section
   semop(sid_lock, &sem_up,1); //allow other readers/writers to make progress
   return count;
}


uint8_t IPCBuffer::close(){

	shmdt((const void *)shmid_queue);
	shmctl(shmid_queue, IPC_RMID, NULL);
	semctl(sid_lock, 0, IPC_RMID, NULL);
	semctl(sid_data, 0, IPC_RMID, NULL);
	semctl(sid_space, 0, IPC_RMID, NULL);

	return 0;
}

uint16_t IPCBuffer::getSemLock(){
	return sid_lock;
}

uint16_t IPCBuffer::getShm(){
	return shmid_queue;
}

uint16_t IPCBuffer::getSemSpace(){
	return sid_space;
}

uint16_t IPCBuffer::getSemData(){
	return sid_data;
}
