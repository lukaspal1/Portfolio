#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> //for forking and piping
#include <sys/types.h> //for pid_t
#include <sys/wait.h> //for waitinf functions

#define NUM_STUDENTS 10 //the max amount of students is 10 per ta
#define ASSIGN_PER_CHAPTER 2
int grades[NUM_STUDENTS][6]; // 10 students, 6 columns (3 chapters 2 assignemtsn)

void TA_proc(int chapter, int assign, int pipe_fd) { //ta process, finds the average of 1 assignment and sends it to the parent (gradTA)
    int sum = 0; //init sum
    for (int i = 0; i < NUM_STUDENTS; i++) { //find the sum for the assignment in each chapter
        sum += grades[i][chapter * ASSIGN_PER_CHAPTER + assign];

    }
    float avg = (float)sum / NUM_STUDENTS; //find the average
    write(pipe_fd, &avg, sizeof(float)); // send to the gta proc through a pipe
    close(pipe_fd); //close 
    exit(0);
}   

void GTA_proc(int chapter, int pipe_fd){  //grad ta process, parent of ta and child of teacher processes
    int pipes[ASSIGN_PER_CHAPTER][2]; //pipes used for communicating with the TAs
    float averages[ASSIGN_PER_CHAPTER];

    for (int i = 0; i < ASSIGN_PER_CHAPTER; i++) {
        pipe(pipes[i]); //create a new pipe for comminication
        
        pid_t pid = fork(); //creat a new ta process (child)
        if (pid == 0) {
            close(pipes[i][0]); //close the reading as the ta is only writing
            TA_proc(chapter, i, pipes[i][1]); //call ta function to calculate the average
            exit(0);
        }else {
            close(pipes[i][1]); //close the writing as the gta only reads
        }
    }
    for (int i = 0; i < ASSIGN_PER_CHAPTER; i++) { //gta waits for the ta processes to finish before reading the result
        wait(NULL);
        float avg;
        read(pipes[i][0], &avg, sizeof(float)); //read average
        averages[i] = avg; //store the data
        close(pipes[i][0]); //close the reading of the pipe
    }
    write(pipe_fd, averages, sizeof(averages));
    close(pipe_fd);
    exit(0);

}
void TEACH_proc(int num_chapters) {
    int pipes[num_chapters][2];
    float averages[num_chapters][ASSIGN_PER_CHAPTER];

    for (int i = 0; i < num_chapters; i++){
        pipe(pipes[i]); // create a new pipe before forking
        pid_t pid = fork(); // create new gta processes for each chapter
        if (pid == 0) {
        close(pipes[i][0]); // close reading end
        
        GTA_proc(i, pipes[i][1]); //call gta function
        exit(0);
        }else{
            close(pipes[i][1]);
        }
        
    }
    for (int i = 0; i < num_chapters; i++) {
        wait(NULL); //wait for all the gta processes to finish for each chapter
        read(pipes[i][0], &averages[i], sizeof(float) * ASSIGN_PER_CHAPTER);
        close(pipes[i][0]);
    }
    for (int i = 0; i < num_chapters; i++) {
        for (int j = 0; j < ASSIGN_PER_CHAPTER; j++) {
            printf("Assignment %d - Average = %.6f\n", i * ASSIGN_PER_CHAPTER + j + 1, averages[i][j]); //print out the stored data here so its in order
        }
    }
}

void read_grades(const char *filename, int *num_chapters) {
    FILE *file = filename ? fopen(filename, "r") : stdin; // Open file or use stdin
    if (!file) { //if opening file fails... but it wont B)
        perror("File open failed");
        exit(1);
    }
    for (int i = 0; i < NUM_STUDENTS; i++) {  //read the grades into an array
        for (int j = 0; j < 6; j++) { //3 chapters, 2 assignments each so 6 columns
            fscanf(file, "%d", &grades[i][j]); 
        }
    }
    fclose(file); //close the file
    *num_chapters = 6 / ASSIGN_PER_CHAPTER; //just 3

}

int main(int argc, char *argv[]) {

    if (argc != 2) { //making sure you use this and the file
        fprintf(stderr, "Usage: %s <grades_files>\n", argv[0]); //how to use
        return 1;
    }
    int num_chapters;
    read_grades(argv[1], &num_chapters); //read grades frin the file, then start the teacher process, which will do the rest
    TEACH_proc(num_chapters);
    return 0;
}
