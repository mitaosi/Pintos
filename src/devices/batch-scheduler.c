/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h" //generate random numbers

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1

/*
 *	initialize task with direction and priority
 *	call o
 * */
typedef struct {
	int direction;
	int priority;
} task_t;


struct semaphore bus_slot; // keeps track of how many slots are occupied
struct semaphore available;
struct semaphore send_normal_prio;
struct semaphore receive_normal_prio;
static int current_dir;
static int nr_priority_send;
static int nr_priority_receive;


void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);


void oneTask(task_t task);/*Task requires to use the bus and executes methods below*/
void getSlot(task_t task); /* task tries to use slot on the bus */
void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
void leaveSlot(task_t task); /* task release the slot */



/* initializes semaphores */ 
void init_bus(void){ 
 
    random_init((unsigned int)123456789);

<<<<<<< HEAD
    occupied_slots = 0;
    current_direction = -1;

    lock_init(&lock);

    cond_init(&var[SENDER][NORMAL]);
    cond_init(&var[SENDER][HIGH]);
    cond_init(&var[RECEIVER][NORMAL]);
    cond_init(&var[RECEIVER][HIGH]);    
=======
    sema_init(&bus_slot, BUS_CAPACITY);
    sema_init(&send_normal_prio, 1);
    sema_down(&send_normal_prio);
    sema_init(&receive_normal_prio, 1);
    sema_down(&receive_normal_prio);
    sema_init(&available, 1);

    current_dir = -1;
    nr_priority_send = 0;
    nr_priority_receive = 0;
>>>>>>> 0c805181dc032d96d2658d48eaca76a6134dae99
}

/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread. 
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive)
{
<<<<<<< HEAD
=======
     nr_priority_send = num_priority_send;
     nr_priority_receive = num_priority_receive;

     if(nr_priority_send) { sema_up(&send_normal_prio); }
     if(nr_priority_receive) { sema_up(&receive_normal_prio); }

>>>>>>> 0c805181dc032d96d2658d48eaca76a6134dae99
     unsigned int i;
     for(i = 0; i < num_tasks_send; i++)
         thread_create("normal_send", PRI_DEFAULT, &senderTask, NULL);
     for(i = 0; i < num_priority_send; i++)
         thread_create("high_send", PRI_DEFAULT, &senderPriorityTask, NULL);
     for(i = 0; i < num_task_receive; i++)
<<<<<<< HEAD
         thread_create("normal_receive", PRI_DEFAULT, &receiverTask, NULL);
=======
         thread_create("normal_recieve", PRI_DEFAULT, &receiverTask, NULL);
>>>>>>> 0c805181dc032d96d2658d48eaca76a6134dae99
     for(i = 0; i < num_priority_receive; i++)
         thread_create("high_recieve", PRI_DEFAULT, &receiverPriorityTask, NULL);
}

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED){
        task_t task = {SENDER, NORMAL};
        oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED){
        task_t task = {SENDER, HIGH};
        oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED){
        task_t task = {RECEIVER, NORMAL};
        oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED){
        task_t task = {RECEIVER, HIGH};
        oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task) {

  getSlot(task);
  transferData(task);
  leaveSlot(task);

}


/* task tries to get slot on the bus subsystem */
void getSlot(task_t task) 
<<<<<<< HEAD
{
    lock_acquire(&lock);
    
    while(occupied_slots == BUS_CAPACITY || (occupied_slots > 0 && task.direction != current_direction))
    {
        waiters[task.direction][task.priority]++;
        cond_wait(&var[task.direction][task.priority], &lock);
        waiters[task.direction][task.priority]--;
    }

    if(current_direction != task.direction && occupied_slots == 0)
    {
        current_direction = task.direction;
        if(waiters[current_direction][HIGH])
            cond_broadcast(&var[current_direction][HIGH], &lock);
        else if(!waiters[!current_direction][HIGH])
            cond_broadcast(&var[current_direction][NORMAL], &lock);
    }

    occupied_slots++;

    lock_release(&lock);
=======
{     
     // if the direction isn't the same and the bus isn't empty, then wait
     if(current_dir != task.direction && bus_slot.value < BUS_CAPACITY) 
     {
         sema_down(&available);
     }

     current_dir = task.direction;
     
     if(task.direction == SENDER)
     {
         if(task.priority == HIGH)
         {
              sema_down(&bus_slot); // get slot
              nr_priority_send--;
              if(nr_priority_send == 0) sema_up(&send_normal_prio); // no more high prio left
         }
         else if(task.priority == NORMAL)
         {
              sema_down(&send_normal_prio); // wait for high prio
              sema_down(&bus_slot); // get slot
              sema_up(&send_normal_prio);
         }
     }
     else if(task.direction == RECEIVER)
     {
         if(task.priority == HIGH)
         {
              sema_down(&bus_slot); // get slot
              nr_priority_receive--;
              if(nr_priority_receive == 0) sema_up(&send_normal_prio); // no more high prio left
         }
         else if(task.priority == NORMAL)
         {
              sema_down(&receive_normal_prio); // wait for high prio
              sema_down(&bus_slot); // get slot
              sema_up(&receive_normal_prio);
         }
     }
>>>>>>> 0c805181dc032d96d2658d48eaca76a6134dae99
}

/* task processes data on the bus send/receive */
void transferData(task_t task)
{
    timer_sleep(random_ulong() % 10);
}

/* task releases the slot */
void leaveSlot(task_t task)
{
<<<<<<< HEAD
     lock_acquire(&lock);

     occupied_slots--;
     
     if(occupied_slots == 0)
     {
         if(waiters[current_direction][HIGH]) 
             cond_broadcast(&var[current_direction][HIGH], &lock);
         else if(waiters[!current_direction][HIGH])
             cond_broadcast(&var[!current_direction][HIGH], &lock);
         else if(waiters[current_direction][NORMAL])
             cond_broadcast(&var[current_direction][NORMAL], &lock);
         else if(waiters[!current_direction][NORMAL])
             cond_broadcast(&var[!current_direction][NORMAL], &lock);
     }
     else
     {
         if(waiters[current_direction][HIGH]) 
             cond_signal(&var[current_direction][HIGH], &lock); 
         else if(!waiters[!current_direction][HIGH]) 
             cond_signal(&var[current_direction][NORMAL], &lock); 
     }

     lock_release(&lock); 
=======
    sema_up(&bus_slot); // leaving the slot
    if(bus_slot.value == BUS_CAPACITY) sema_up(&available); 
>>>>>>> 0c805181dc032d96d2658d48eaca76a6134dae99
}
