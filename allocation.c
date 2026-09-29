#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROCS 100
#define MEMORY_SIZE 1048576

typedef struct {
    int start;
    int end;
    int size;
    char process[10];
    bool allocated;

} MemoryBlock;

typedef struct {
    int start;
    int end;
    int size;

} Hole;

MemoryBlock memory[MAX_PROCS];
int total_blocks = 0;

void init_memory(int size) {
    total_blocks = 1;
    memory[0].start = 0;
    memory[0].end = MEMORY_SIZE - 1;
    memory[0].size = MEMORY_SIZE;
    strcpy(memory[0].process, "");
    memory[0].allocated = false;
}

void print_status(){
    int allocated_memory = 0;
    int free_memory = 0;

    //calculate total allocated memory
    for (int i = 0; i < total_blocks; i++) {
        if (memory[i].allocated) {
            allocated_memory += memory[i].size;
        }else{
            free_memory += memory[i].size;
        }
    }
    printf("Partitions [Allocated memory = %d]: \n", allocated_memory);

    //print the allocated partitions
    for (int i = 0; i < total_blocks; i++) {
        if (memory[i].allocated) {
            printf("Address [%d:%d] Process %s\n", memory[i].start, memory[i].end, memory[i].process);
        }
    }
    printf("\nHoles [Free memory = %d]:\n", free_memory);

    //print the free holes
    for (int i = 0; i < total_blocks; i++) {
        if (!memory[i].allocated){
            printf("Address [%d:%d] len = %d\n", memory[i].start, memory[i].end, memory[i].size);
        }
    }
}

void merge_holes() {
    int i = 0;
    while (i < total_blocks - 1) { //if current bloack and next block are both holes 
        if (!memory[i].allocated && !memory[i+1].allocated){
            memory[i].end = memory[i+1].end;
            memory[i].size += memory[i+1].size;
            
            //remove second hole by shifting all blocks
            for (int j = i + 1; j < total_blocks - 1; j++) {
                memory[j] = memory[j+1];
            }
            total_blocks--;
            //dont increment i here because we need to check if the merged hole can be merge with the next one
        }else{
            i++;
        }
    }
}
void release_memory(char *process_id) {
    bool found = false;

    //find and free block
    for (int i = 0; i < total_blocks; i++) {
        if (memory[i].allocated && strcmp(memory[i].process, process_id) == 0) {
            memory[i].allocated = false;
            strcpy(memory[i].process, "");
            found = true;
            printf("Successfully released memory for process %s\n", process_id);
            break;
        }
    }
    if (!found) {
        printf("Error: Process %s not found\n", process_id);
        return;
    }
    merge_holes();
}
int first_fit(int size) {
    for (int i = 0; i < total_blocks; i++) {
        if (!memory[i].allocated && memory[i].size >= size) {
            return i;
        }
    }
    return -1;
}
void allocate_memory(char *process, int size, char strategy) {
    int block_index = -1;

    //only using first fit..
    switch(strategy) {
        case 'F':
            block_index = first_fit(size);
            break;
        case 'B':
            printf("Sorry, only first fit is implemented\n");
            return;
        case 'W':
            printf("Sorry, only first fit is implemented\n");
        default:
            printf("Invalid allocation strategy\n");
    }
    if (block_index == -1) {
        printf("No hole of sufficient size\n");
        return;
    }
    //found suitable block
    if (memory[block_index].size == size) { //exact fit of memory
        memory[block_index].allocated = true;
        strcpy(memory[block_index].process, process);
    }else{
        //make space for the new block
        if (total_blocks >= MAX_PROCS) {
            printf("Error: Maximum number of processes reached\n");
            return;
        }
        for (int i = total_blocks; i > block_index + 1; i--) {
            memory[i] = memory[i-1];
        }
        //configure allocated block
        MemoryBlock *allocated = &memory[block_index];
        MemoryBlock *remaining = &memory[block_index + 1];

        //create the remaining free block
        remaining->start = allocated->start + size;
        remaining->end = allocated->end;
        remaining->size = allocated->size - size;
        remaining->allocated = false;
        strcpy(remaining->process, "");

        //update the allocated block
        allocated->end = remaining->start - 1;
        allocated->size = size;
        allocated->allocated = true;
        strcpy(allocated->process, process);

        total_blocks++;
    }
    printf("Successfully allocated %d to process %s\n", size, process);
}
    

void compact_memory() {
    if (total_blocks <= 1) {
        return; //if there are no blocks
    }
    //merge all free blocks
    merge_holes();
    //create temporary array to hold compacted memory
    MemoryBlock temp[MAX_PROCS];
    int temp_count = 0;

    //start with allocated blocks
    for (int i = 0; i < total_blocks; i++) {
        if (memory[i].allocated) {
            temp[temp_count++] = memory[i];
        }
    }
    //get total allocated size
    int allocated_size = 0;
    for (int i = 0; i < temp_count; i++) {
       allocated_size += temp[i].size;
    }
    //if there is free space, add a hole at the end
    if (allocated_size < MEMORY_SIZE) {
        MemoryBlock free_block;
        free_block.start = allocated_size;
        free_block.end = MEMORY_SIZE - 1;
        free_block.size = MEMORY_SIZE - allocated_size;
        free_block.allocated = false;
        strcpy(free_block.process, "");
        temp[temp_count++] = free_block;
    }

    //fix the start and end addresses of all the blocks
    int curr_address = 0;
    for (int i = 0; i < temp_count; i++) {
        temp[i].start = curr_address;
        temp[i].end = curr_address + temp[i].size - 1;
        curr_address = temp[i].end + 1;
    }

    //copy back to main mem array
    for (int i = 0; i < temp_count; i++) {
        memory[i] = temp[i];
    }
    total_blocks = temp_count;
    printf("Compacting process is successful\n");
}
int main(int argc, char *argv[]){
    if (argc != 2) {
        printf("Usage: %s <memory_size>\n", argv[0]);
        return 1;
    }
    int memory_size = atoi(argv[1]);
    if (memory_size <= 0 || memory_size > MEMORY_SIZE) {
        printf("Invalid memory size. Using default: %d\n", MEMORY_SIZE);
        memory_size = MEMORY_SIZE;
    }
    init_memory(memory_size);
    printf("Here, the First Fit approach has been implemented and the allocated %d bytes of memory.\n", memory_size);

    char command[10];
    char process_id[10];
    int size;
    char strategy;

    //menu
    while(1) {
        printf("command>");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            scanf("%s %d %c", process_id, &size, &strategy);
            allocate_memory(process_id, size, strategy);
        }
        else if (strcmp(command, "RL") == 0) {
            scanf("%s", process_id);
            release_memory(process_id);
        }
        else if (strcmp(command, "Status") == 0 || strcmp(command, "status") == 0) {
            print_status();
        }
        else if (strcmp (command, "C") == 0) {
            compact_memory();
        }
        else if (strcmp(command, "Exit") == 0 || strcmp(command, "exit") == 0) {
            printf("Exiting program\n");
            break;
        }else{
            printf("Invalid command, available commands are RQ, RL, Status, C and Exit\n");
        }  
    }
}
