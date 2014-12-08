import sysv_ipc

class IPCMonitor():

	def __init__(self, id):
		print("IPC monitor created for id: " + str(id) + "\n")
		self.id = id
		self.k_sem_lock = sysv_ipc.ftok("/sketch/k_lock.txt", self.id, True)
		self.k_shm = sysv_ipc.ftok("/sketch/k_shm.txt", self.id, True)
		self.k_sem_data = sysv_ipc.ftok("/sketch/k_data.txt", self.id, True)
		self.k_sem_space = sysv_ipc.ftok("/sketch/k_space.txt", self.id, True)

	def open(self):
		self.sem_lock = sysv_ipc.Semaphore(self.k_sem_lock)
		self.mem = sysv_ipc.SharedMemory(self.k_shm)
		self.sem_data = sysv_ipc.Semaphore(self.k_sem_data) #initially 0
		self.sem_space = sysv_ipc.Semaphore(self.k_sem_space) #initially 1

	def lock_value(self):
		return self.sem_lock.value

	def data_value(self):
		return self.sem_data.value

	def space_value(self):
		return self.sem_space.value
