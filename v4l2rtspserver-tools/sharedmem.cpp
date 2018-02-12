#include <cstdlib>
#include "sharedmem.h"

SharedMem::SharedMem() {
    currentConfig.nightmode = 0;
    currentConfig.flip = 0;

    key_image_mem = ftok("/usr/include", '1');
    key_image_semaphore = ftok("/usr/include", '2');
    key_config_mem = ftok("/usr/include", '3');
    key_config_semaphore = ftok("/usr/include", '4');

    semaphore_lock[0].sem_flg = 0;
    semaphore_lock[0].sem_num = (unsigned short) -1;
    semaphore_lock[0].sem_op = SEM_UNDO;


    semaphore_unlock[0].sem_flg = 0;
    semaphore_unlock[0].sem_num = (unsigned short) 1;
    semaphore_unlock[0].sem_op = SEM_UNDO;


}

SharedMem::~SharedMem() {

}

int SharedMem::getImageSize() {
    return this->getMemorySize(key_image_mem);
}

void *SharedMem::getImageBuffer() {
    int memlen = this->getImageSize();
    void *memory = malloc(memlen);

    this->lockSemaphore(key_image_semaphore);
    this->readMemory(key_image_mem, memory, memlen);
    this->unlockSemaphore(key_image_semaphore);


    return memory;
}

void SharedMem::copyImage(void *imageMemory, int imageSize) {
    this->lockSemaphore(key_image_semaphore);
    this->writeMemory(key_image_mem, imageMemory, imageSize);
    this->unlockSemaphore(key_image_semaphore);
}

shared_conf *SharedMem::getConfig() {
    return &currentConfig;
}

void SharedMem::readConfig(){
    this->lockSemaphore(key_config_semaphore);
    this->readMemory(key_config_mem, &currentConfig, sizeof(shared_conf));
    this->unlockSemaphore(key_config_semaphore);
}

void SharedMem::setConfig() {
    this->lockSemaphore(key_config_semaphore);
    this->writeMemory(key_config_mem, &currentConfig, sizeof(shared_conf));
    this->unlockSemaphore(key_config_semaphore);
}

void SharedMem::lockSemaphore(key_t key) {
    semop(key, &semaphore_lock[0], 1);
}

void SharedMem::unlockSemaphore(key_t key) {
    semop(key, &semaphore_unlock[0], 1);


}

void SharedMem::readMemory(key_t key, void *memory, int memorylenght) {
    void *shared_mem;
    int shm_id = shmget(key, 0, 0);
    shared_mem = shmat(shm_id, NULL, 0);
    memcpy(memory, shared_mem, (size_t) memorylenght);
    shmdt(shared_mem);
}

int SharedMem::getMemorySize(key_t key) {
    int shm_id = shmget(key, 0, 0);
    struct shmid_ds buf;
    shmctl(shm_id, IPC_STAT, &buf);
    int memlen = buf.shm_segsz;
    return memlen;
}

void SharedMem::writeMemory(key_t key, void *memory, int memorylenght) {

    int shm_id;

    shm_id = shmget(key, 0, 0);
    if (shm_id != -1) {
        int memlen = this->getMemorySize(key);
        if (memlen != memorylenght) {
            shmctl(shm_id, IPC_RMID, NULL);
        }
    }


    shm_id = shmget(key, memorylenght, IPC_CREAT);
    if (shm_id != -1) {
        void *shared_mem;
        shared_mem = shmat(shm_id, NULL, 0);
        memcpy(shared_mem, memory, memorylenght);
        shmdt(shared_mem);
    } else {

    }


}

void *SharedMem::getImage() {
    int memlen = getImageSize();
    void *memory = malloc((size_t) memlen);
    this->lockSemaphore(key_image_semaphore);
    this->readMemory(key_image_mem, memory, memlen);
    this->unlockSemaphore(key_image_semaphore);
    return memory;

}