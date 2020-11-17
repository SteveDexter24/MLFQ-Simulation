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

typedef struct timeSlice {
    int ts;
    int allotment_time;
}timeSlice;

void oneQueue(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period);

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);

int atIndex(Process p, int proc_num, remainingTime rt[]);


void s(Process *xp, Process *yp);
void SortP(LinkedQueue *queue, int proc_num);

void scheduler(Process* proc, LinkedQueue** ProcessQueue, int proc_num, int queue_num, int period){
//    printf("Process number: %d\n\n", proc_num);
    
    // remaining time array
    remainingTime *rt = (remainingTime *)malloc(proc_num * sizeof(remainingTime));
    
    // slice offset, if the remaining time is less than the slice itself
    //int* sliceOffset = (int*)malloc((proc_num + 1) * sizeof(int));
    
    for (int i = 0;i < proc_num; i++) {
        //printf("%d %d %d\n", proc[i].process_id, proc[i].arrival_time, proc[i].execution_time);
        rt[i].rt = 0;
        rt[i].slice = 0;
        rt[i].pid = proc[i].process_id;
        //sliceOffset[i] = 0;
    }
    
    // time slice array
    timeSlice *ts = (timeSlice*)malloc((queue_num + 1) * sizeof(timeSlice));
    
    for (int i = 0; i < queue_num; i++){
//        printf("%d %d %d\n", i, ProcessQueue[i]->time_slice, ProcessQueue[i]->allotment_time);
        ts[i].ts = ProcessQueue[i]->time_slice;
        ts[i].allotment_time = ProcessQueue[i]->allotment_time;
    }
    
    int proc_completed = 0, p_time = 0, period_at = 1;
    int time = 1;
    bool hold_slice = false;
    int hold_by = 0;
    int q = queue_num - 1;
    int process_entered = 0;
    
    do {
        
        //printf("time = %d", time);
        
        for(int i = 0; i < proc_num; i++){
            if(time == proc[i].arrival_time){
//                printf("Enqueue at time = %d\n", time);
                ProcessQueue[queue_num - 1] = EnQueue(ProcessQueue[queue_num - 1], proc[i]);
                rt[i].rt = proc[i].execution_time;
                process_entered += 1;
            }
        }
        
        int slice_corr = 0;
  
        if (hold_slice == false) {
            
            int count = 0;
            
            q = queue_num - 1;
            for(int i = 0; i < queue_num; i++){
                if(!IsEmptyQueue(ProcessQueue[i])){
                    q = i;
                }else{
                    count++;
                }
            }
                
            Process front;
            
            //if(!IsEmptyQueue(ProcessQueue[q])){
                front = FrontQueue(ProcessQueue[q]);

                int index = atIndex(front, proc_num, rt);
                
                if(rt[index].rt < ts[q].ts && period_at * period >= ts[q].ts + time){
                    hold_by = rt[index].rt;
                }else if(period * period_at < ts[q].ts + time){
                    hold_by = period - time + 1;
                    p_time = hold_by;
                }else{
                    hold_by = ts[q].ts;
                }
                hold_slice = true;
           // }
        }
        
        if(hold_slice == true){
            hold_by -= 1;
        }
        
        if(hold_by == 0){
            hold_slice = false;
        }
        
        // execute time slice
        if(time > 0 && hold_slice == false){
            if(time == ts[q].ts){
                continue;
            }
            int toExecute = 0;
            
            Process temp = FrontQueue(ProcessQueue[q]);
            // determine which rt to execute
            for(int i = 0; i < proc_num; i++){
                if(temp.process_id == rt[i].pid){ toExecute = i; break;}
            }
            
            if(time == period * period_at){
                slice_corr = ts[q].ts - p_time;
                rt[toExecute].rt -= p_time;
            }else if(rt[toExecute].rt < ts[q].ts && time != period * period_at){
                slice_corr = ts[q].ts - rt[toExecute].rt;
                rt[toExecute].rt = 0;
            }
            else{ rt[toExecute].rt -= ts[q].ts;}
            
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
                    //sliceOffset[toExecute] = (time) % ts[q - 1].ts;
                    outprint(time - ts[q].ts + slice_corr, time, dequeue.process_id, dequeue.arrival_time, rt[toExecute].rt);
                    
                }else{
                    // slice not used up, round robin
                    ProcessQueue[q] = EnQueue(ProcessQueue[q], dequeue);
                    //sliceOffset[toExecute] = (time) % ts[q].ts;
                    outprint(time - ts[q].ts + slice_corr, time, dequeue.process_id, dequeue.arrival_time, rt[toExecute].rt);
                    
                }
            }else{
//                printf("\nProcess dequeue = %d\n", dequeue.process_id);
                outprint(time - ts[q].ts + slice_corr, time, dequeue.process_id, dequeue.arrival_time, rt[toExecute].rt);
                proc_completed += 1;
            }
        }
      
        if(time == period){
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
//            printf("\n\n proc_completed = %d \n\n", proc_completed);
            SortP(ProcessQueue[queue_num - 1], process_entered - proc_completed);
            
//            printf("\nQueues after sort\n");
//            for(int i = 0; i < queue_num; i++){
//                QueuePrint(ProcessQueue[queue_num - 1 - i]);
//            }
//            printf("\n");
            period_at += 1;
        }
        
        if(proc_completed == proc_num){
            break;
        }
        
    }while(++time);
   
}

int atIndex(Process p, int proc_num, remainingTime rt[]){
    int i;
    for(i = 0; i < proc_num; i++){
        if(p.process_id == rt[i].pid){break;}
    }
    return i;
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
        if(!IsEmptyQueue(queue))
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
