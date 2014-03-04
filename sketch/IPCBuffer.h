#ifndef IPCBUFFER_H
#define IPCBUFFER_H
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
union semun{ 
	uint32_t val;
	struct semid_ds *buf;
	ushort *array;
	struct seminfo *__buf;
	void *__pad;
};

class IPCBuffer{

	public:
		IPCBuffer(uint8_t id);
		uint8_t open(size_t length, size_t width);
		uint8_t write(void* source);
		uint8_t read(void* dest);		
		uint8_t close();
		uint16_t getSemLock();
		uint16_t getShm();
		uint16_t getSemSpace();
		uint16_t getSemData();
	private:
		uint32_t _id;
		size_t _length;
		size_t _width;
		union semun semopts; 
		uint32_t sid_lock;
		uint32_t shmid_queue; 
		uint32_t sid_data; 
		uint32_t sid_space; 
		uint8_t* shared_memory;	
		struct sembuf sem_down = {0, -1, 0};
		struct sembuf sem_up = {0, 1, 0};
		uint8_t startMem(uint32_t* shmid, key_t key, uint8_t** shared_memory, size_t size);
		uint8_t startSem(uint32_t* sid, key_t key, uint32_t initial_value);
};

#endif
