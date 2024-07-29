#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_WAITING_CHAIRS 3 // Number of chairs in the hallway

sem_t ta_sleeping;          // Semaphore to indicate TA is sleeping
sem_t student_waiting;      // Semaphore for students waiting
pthread_mutex_t mutex_lock; // Mutex for shared data access

int students_waiting = 0;   // Number of students waiting
int waiting_chairs[NUM_WAITING_CHAIRS]; // Array to keep track of waiting students
int current_student = -1; // Variable to keep track of the current student being helped

void *student_thread(void *param);
void *ta_thread(void *param);

int main() {
    int total_students = 5; // Total number of students
    pthread_t ta_thread_id;
    pthread_t student_thread_ids[total_students];
    
    // Initialize semaphores and mutex
    sem_init(&ta_sleeping, 0, 0);
    sem_init(&student_waiting, 0, 0);
    pthread_mutex_init(&mutex_lock, NULL);
    
    srand(time(NULL)); // Initialize random number generator
    
    // Create the TA thread
    pthread_create(&ta_thread_id, NULL, ta_thread, NULL);
    
    // Create student threads
    for(int i = 0; i < total_students; i++) {
        int *student_id = malloc(sizeof(int));
        *student_id = i;
        pthread_create(&student_thread_ids[i], NULL, student_thread, student_id);
    }
    
    // Wait for all student threads to finish
    for(int i = 0; i < total_students; i++) {
        pthread_join(student_thread_ids[i], NULL);
    }
    
    // Wait for the TA thread to finish
    pthread_join(ta_thread_id, NULL);
    
    // Destroy semaphores and mutex
    sem_destroy(&ta_sleeping);
    sem_destroy(&student_waiting);
    pthread_mutex_destroy(&mutex_lock);
    
    return 0;
}

void *student_thread(void *param) {
    int student_id = *(int *)param;
    free(param);
    
    while(1) {
        // Simulate programming time
        sleep(rand() % 5 + 1);
        
        pthread_mutex_lock(&mutex_lock);
        if (students_waiting < NUM_WAITING_CHAIRS) {
            if (students_waiting == 0) {
                // If this is the first student, wake up the TA
                printf("Student %d tries to wake the TA.\n", student_id);
                current_student = student_id;
                sem_post(&ta_sleeping);
            } else {
                // Sit in the waiting chair
                waiting_chairs[students_waiting - 1] = student_id;
                printf("Student %d is here waiting in the chair. Available chairs: %d\n", student_id, NUM_WAITING_CHAIRS - students_waiting - 1);
            }
            students_waiting++;
            pthread_mutex_unlock(&mutex_lock);
            
            // Wait for the TA to help
            sem_wait(&student_waiting);
        } else {
            // No available waiting chairs, go back to programming
            printf("Student %d is here, there are no chairs available, student %d is returning to programming.\n", student_id, student_id);
            pthread_mutex_unlock(&mutex_lock);
        }
    }
    
    pthread_exit(NULL);
}

void *ta_thread(void *param) {
    while(1) {
        // Wait for a student to wake up the TA
        printf("TA is sleeping.\n");
        sem_wait(&ta_sleeping);
        
        while(1) {
            pthread_mutex_lock(&mutex_lock);
            if (students_waiting > 0) {
                int student_being_helped = current_student;
                if (students_waiting > 1) {
                    current_student = waiting_chairs[0];
                    for (int i = 0; i < students_waiting - 1; i++) {
                        waiting_chairs[i] = waiting_chairs[i + 1];
                    }
                } else {
                    current_student = -1;
                }
                students_waiting--;
                printf("TA is helping student %d. Remaining waiting students: %d\n", student_being_helped, students_waiting);
                sem_post(&student_waiting);
                pthread_mutex_unlock(&mutex_lock);
                
                // Simulate help time
                sleep(rand() % 3 + 1);
            } else {
                // No students waiting, go back to sleep
                current_student = -1;
                pthread_mutex_unlock(&mutex_lock);
                printf("TA is going back to sleep.\n");
                break;
            }
        }
    }
    
    pthread_exit(NULL);
}

