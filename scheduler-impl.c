//
//  main.c
//  MLFQ Scheduler
//
//  Created by Steve Tamang on 7/11/2020.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

typedef struct remainingTime {
    int rt;
    int pid;
    int slice;
}remainingTime;

void oneQueue(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period);

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);

int atIndex(Process p, int proc_num, remainingTime rt[]);


void s(Process *xp, Process *yp);
void SortP(LinkedQueue *queue, int proc_num);

void scheduler(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period){
    printf("Process number: %d\n", proc_num);
    
    
    
    // remaining time array
    remainingTime *rt = (remainingTime *)malloc(proc_num * sizeof(remainingTime));
    
    // slice offset, if the remaining time is less than the slice itself
    int* sliceOffset = (int*)malloc(proc_num * sizeof(int));
    
    for (int i = 0;i < proc_num; i++) {
        printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);
        rt[i].rt = 0;
        rt[i].slice = 0;
        rt[i].pid = proc[i].process_id;
        sliceOffset[i] = 0;
        
    }

    printf("\nQueue number: %d\n", queue_num);
    printf("Period: %d\n", period);
    
    
    typedef struct timeSlice {
        int ts;
        int allotment_time;
    }timeSlice;
    
    // time slice array
    timeSlice* ts = (timeSlice*)malloc(queue_num * sizeof(timeSlice));
    
    for (int i = 0;i < queue_num; i++){
        printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
        ts[i].ts = ProcessQueue[i]->time_slice;
        ts[i].allotment_time = ProcessQueue[i]->allotment_time;
    }
    
    /*
       Test outprint function, it will output "Time_slot:1-2, pid:3, arrival-time:4, remaining_time:5" to output.loc file.
    */
    
    /*
     
     Process number: 5
     0: 36 10 10
     1: 1023 10 160
     2: 123 60 90
     3: 13 70 100
     4: 12 80 30

     Queue number: 3
     Period: 300
     0 60 120
     1 40 80
     2 10 30
     
     */
    int proc_completed = 0;
    int time = 1;
    bool hold_slice = false;
    int hold_by = 0;
    int q;
    
    do {
  
        if (hold_slice == false) {
            
            if(IsEmptyQueue(ProcessQueue[2]) && !IsEmptyQueue(ProcessQueue[1])){
                q = 1;
                hold_by = ts[1].ts;
                hold_slice = true;

            }
            else if(IsEmptyQueue(ProcessQueue[2]) && IsEmptyQueue(ProcessQueue[1]) && !IsEmptyQueue(ProcessQueue[0])){
                q = 0;
                hold_by = ts[0].ts;
                hold_slice = true;
            }
            else {
                hold_by = ts[2].ts;
                hold_slice = true;
                q = 2;
            }
            //int count = 0;
            
//            for(int i = queue_num - 1; i >= 0; i--){
//                if (IsEmptyQueue(ProcessQueue[i])) {
//
//                }else{
//                    q = i - 1;
//                    hold_by = ts[q].ts;
//                    hold_slice = true;
//                    break;
//                }
//            }
        }
        
       
        // don't deduct by time slice, deduct by time
        // if the time remaining is ==
        
//        printf("Time = %d\n", time);
//
//
//        printf("Q3: ");
//        QueuePrint(ProcessQueue[2]);
//        printf("\nQ2: ");
//        QueuePrint(ProcessQueue[1]);
//        printf("\nQ1: ");
//        QueuePrint(ProcessQueue[0]);
//        printf("\n");

        
        for(int i = 0; i < proc_num; i++){
            if(time == proc[i].arrival_time){
                ProcessQueue[2] = EnQueue(ProcessQueue[2], proc[i]);
                rt[i].rt = proc[i].execution_time;
                sliceOffset[i] = time % ts[2].ts;
            }
        }
        
        Process p = FrontQueue(ProcessQueue[q]);
        
        
        if(hold_slice == true){
            hold_by -= 1;
        }
        
        if(hold_by == 0){
            hold_slice = false;
        }
        
        // execute time slice
        if(time > 0 /*&& time % ts[q].ts == sliceOffset[h]*/ && hold_slice == false){
            if(time == ts[q].ts){
                continue;
            }
            int toExecute = 0;
            
            Process temp = FrontQueue(ProcessQueue[q]);
            // determine which rt to execute
            for(int i = 0; i < proc_num; i++){
                if(temp.process_id == rt[i].pid){ toExecute = i; break;}
            }
            
        
            rt[toExecute].rt -= ts[q].ts;
            rt[toExecute].slice += 1;
            
            // dequeue
            Process dequeue;
            if(!IsEmptyQueue(ProcessQueue[q])){
                dequeue = DeQueue(ProcessQueue[q]);
            }
            
            
            // remaining time for process != 0 and number of sliced used x slice is not equal to the allotment_time
            if(rt[toExecute].rt > 0){
                // slice used up
                if(rt[toExecute].slice * ts[q].ts == ts[q].allotment_time){
                    //printf("go to lower queue at queue: %d\n", q - 1);
                    QueuePrint(ProcessQueue[q]);
                    
                    ProcessQueue[q - 1] = EnQueue(ProcessQueue[q - 1], dequeue);
                    
                    QueuePrint(ProcessQueue[q - 1]);
                    rt[toExecute].slice = 0;
                    sliceOffset[toExecute] = (time) % ts[q - 1].ts;
                    outprint(time - ts[q].ts, time, dequeue.process_id, dequeue.arrival_time, rt[toExecute].rt);
                    
                }else{
                    // slice not used up, round robin
                    ProcessQueue[q] = EnQueue(ProcessQueue[q], dequeue);
                    sliceOffset[toExecute] = (time) % ts[q].ts;
                    outprint(time - ts[q].ts, time, dequeue.process_id, dequeue.arrival_time, rt[toExecute].rt);
                    
                }
            }else{
                printf("\nProcess dequeue = %d\n", dequeue.process_id);
                outprint(time - ts[q].ts, time, dequeue.process_id, dequeue.arrival_time, rt[toExecute].rt);
                proc_completed += 1;
            }
        }
      
        if(time == 300){
            // dequeue from all queue to a temp queue and sort by arrival time and set it as queue 2
            
            for(int i = 0; i < queue_num - 1; i++){
                while (!IsEmptyQueue(ProcessQueue[i])) {
                    ProcessQueue[queue_num - 1] = EnQueue(ProcessQueue[queue_num - 1], DeQueue(ProcessQueue[i]));
                }
            }
            
            for(int i = 0; i < proc_num; i++){
                rt[i].slice = 0;
            }
            
            QueuePrint(ProcessQueue[queue_num - 1]);
            SortP(ProcessQueue[queue_num - 1], proc_num - proc_completed);
            printf("\n--- After sort ---");
            printf("Queue 2: ");
            QueuePrint(ProcessQueue[queue_num - 1]);
            printf("\nQueue 1:");
            QueuePrint(ProcessQueue[queue_num - 2]);
            printf("\nQueue 0:");
            QueuePrint(ProcessQueue[queue_num - 3]);
            
            
        }
        
        if(proc_completed == proc_num){
            break;
        }
        
    }while(++time);
   
}

int atIndex(Process p, int proc_num, remainingTime rt[]){
    int h = -1;
    for(int i = 0; i < proc_num; i++){
        if(p.process_id == rt[i].pid){ h = i; break;}
    }
    return h;
}

void s(Process *xp, Process *yp){
    Process temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void SortP(LinkedQueue *queue, int proc_num){
    Process *a = (Process *)malloc(sizeof(Process) * proc_num);
    //int *a = (int *)malloc(sizeof(int) * proc_num);
    for(int i = 0; i < proc_num; i++){
        a[i] = DeQueue(queue);
    }

    for(int i = 0; i < proc_num - 1; i++){
        for(int j = 0; j < proc_num - i - 1; j++){
            if (a[j].process_id > a[j + 1].process_id) {s(&a[j], &a[j + 1]);}
        }
    }
    for(int i = 0; i < proc_num; i++){
        queue = EnQueue(queue, a[i]);
    }
    
}
