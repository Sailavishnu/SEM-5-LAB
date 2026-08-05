#include <stdio.h>
#include <limits.h>
struct Process{
    int pid;
    int at;       
    int bt;       
    int wt;       
    int tat;      
    int priority;
    int rt;       
};
struct Process p[20];
int n;
void printCompressedGantt(int *ganttPid, int *ganttStart, int *ganttEnd, int gc){
    int i;
    printf("\nGantt Chart:\n|");
    for(i=0;i<gc;i++)
        printf(" P%d |",ganttPid[i]);
    printf("\n%d",ganttStart[0]);
    for(i=0;i<gc;i++)
        printf("    %d",ganttEnd[i]);
    printf("\n");
}
void fcfs(){
    struct Process s[20], temp;
    int i,j,time=0;
    float avgwt=0,avgtat=0;
    int ganttStart[20], ganttEnd[20];
    for(i=0;i<n;i++)
        s[i]=p[i];
    for(i=0;i<n-1;i++)
        for(j=0;j<n-i-1;j++)
            if(s[j].at>s[j+1].at)
            {
                temp=s[j];
                s[j]=s[j+1];
                s[j+1]=temp;
            }
    for(i=0;i<n;i++)
    {
        if(time<s[i].at)
            time=s[i].at;
        ganttStart[i]=time;
        s[i].wt=time-s[i].at;          
        time+=s[i].bt;
        ganttEnd[i]=time;
        s[i].tat=time-s[i].at;         
    }
    printf("\nGantt Chart:\n|");
    for(i=0;i<n;i++)
        printf(" P%d |",s[i].pid);
    printf("\n%d",ganttStart[0]);
    for(i=0;i<n;i++)
        printf("   %d",ganttEnd[i]);
    printf("\n\nProcess\tAT\tBT\tWT\tTAT\n");
    for(i=0;i<n;i++)
    {
        avgwt+=s[i].wt;
        avgtat+=s[i].tat;
        printf("P%d\t%d\t%d\t%d\t%d\n",
        s[i].pid,s[i].at,s[i].bt,s[i].wt,s[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f",avgwt/n);
    printf("\nAverage Turnaround Time = %.2f\n",avgtat/n);
}
void sjf(){
    struct Process s[20];
    int done[20]={0};
    int i,count=0,time=0,idx;
    float avgwt=0,avgtat=0;
    int ganttPid[20], ganttStart[20], ganttEnd[20], gc=0;
    for(i=0;i<n;i++)
        s[i]=p[i];
    while(count<n)
    {
        idx=-1;
        int minbt=INT_MAX;
        for(i=0;i<n;i++)
        {
            if(!done[i] && s[i].at<=time && s[i].bt<minbt)
            {
                minbt=s[i].bt;
                idx=i;
            }
        }
        if(idx==-1)
        {
            int nextAt=INT_MAX;
            for(i=0;i<n;i++)
                if(!done[i] && s[i].at<nextAt)
                    nextAt=s[i].at;
            time=nextAt;
            continue;
        }
        ganttPid[gc]=s[idx].pid;
        ganttStart[gc]=time;
        s[idx].wt=time-s[idx].at;
        time+=s[idx].bt;
        s[idx].tat=time-s[idx].at;
        ganttEnd[gc]=time;
        gc++;
        done[idx]=1;
        count++;
    }
    printCompressedGantt(ganttPid,ganttStart,ganttEnd,gc);
    printf("\nProcess\tAT\tBT\tWT\tTAT\n");
    for(i=0;i<n;i++)
    {
        avgwt+=s[i].wt;
        avgtat+=s[i].tat;
        printf("P%d\t%d\t%d\t%d\t%d\n",
        s[i].pid,s[i].at,s[i].bt,s[i].wt,s[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f",avgwt/n);
    printf("\nAverage Turnaround Time = %.2f\n",avgtat/n);
}
void srtf(){
    struct Process s[20];
    int i;
    int time=0,completed=0;
    float avgwt=0,avgtat=0;
    int rawPid[2000], rawCount=0;

    for(i=0;i<n;i++)
    {
        s[i]=p[i];
        s[i].rt=s[i].bt;
    }
    int minAt=s[0].at;
    for(i=1;i<n;i++)
        if(s[i].at<minAt) minAt=s[i].at;
    time=minAt;
    while(completed<n)
    {
        int idx=-1,minrt=INT_MAX;
        for(i=0;i<n;i++){
            if(s[i].at<=time && s[i].rt>0 && s[i].rt<minrt){
                minrt=s[i].rt;
                idx=i;
            }
        }
        if(idx==-1){
            time++;
            continue;
        }
        rawPid[rawCount++]=s[idx].pid;
        s[idx].rt--;
        time++;
        if(s[idx].rt==0){
            completed++;
            s[idx].tat=time-s[idx].at;
            s[idx].wt=s[idx].tat-s[idx].bt;
        }
    }
    int ganttPid[2000], ganttStart[2000], ganttEnd[2000], gc=0;
    int t=minAt;
    for(i=0;i<rawCount;){
        int curPid=rawPid[i];
        int start=t;
        while(i<rawCount && rawPid[i]==curPid){
            t++;
            i++;
        }
        ganttPid[gc]=curPid;
        ganttStart[gc]=start;
        ganttEnd[gc]=t;
        gc++;
    }
    printCompressedGantt(ganttPid,ganttStart,ganttEnd,gc);
    printf("\nProcess\tAT\tBT\tWT\tTAT\n");
    for(i=0;i<n;i++){
        avgwt+=s[i].wt;
        avgtat+=s[i].tat;
        printf("P%d\t%d\t%d\t%d\t%d\n",
        s[i].pid,s[i].at,s[i].bt,s[i].wt,s[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f",avgwt/n);
    printf("\nAverage Turnaround Time = %.2f\n",avgtat/n);
}
void priorityScheduling(){
    struct Process s[20];
    int done[20]={0};
    int i,count=0,time=0,idx;
    float avgwt=0,avgtat=0;
    int ganttPid[20], ganttStart[20], ganttEnd[20], gc=0;
    for(i=0;i<n;i++)
        s[i]=p[i];
    while(count<n){
        idx=-1;
        int bestPriority=INT_MAX;
        for(i=0;i<n;i++){
            if(!done[i] && s[i].at<=time && s[i].priority<bestPriority){
                bestPriority=s[i].priority;
                idx=i;
            }
        }
        if(idx==-1){
            int nextAt=INT_MAX;
            for(i=0;i<n;i++)
                if(!done[i] && s[i].at<nextAt)
                    nextAt=s[i].at;
            time=nextAt;
            continue;
        }
        ganttPid[gc]=s[idx].pid;
        ganttStart[gc]=time;
        s[idx].wt=time-s[idx].at;
        time+=s[idx].bt;
        s[idx].tat=time-s[idx].at;
        ganttEnd[gc]=time;
        gc++;
        done[idx]=1;
        count++;
    }
    printCompressedGantt(ganttPid,ganttStart,ganttEnd,gc);
    printf("\nProcess\tAT\tBT\tPriority\tWT\tTAT\n");
    for(i=0;i<n;i++){
        avgwt+=s[i].wt;
        avgtat+=s[i].tat;
        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\n",
        s[i].pid,s[i].at,s[i].bt,s[i].priority,
        s[i].wt,s[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f",avgwt/n);
    printf("\nAverage Turnaround Time = %.2f\n",avgtat/n);
}
void preemptivePriority(){
    struct Process s[20];
    int i;
    int time=0,completed=0;
    float avgwt=0,avgtat=0;
    int rawPid[2000], rawCount=0;
    for(i=0;i<n;i++){
        s[i]=p[i];
        s[i].rt=s[i].bt;
    }
    int minAt=s[0].at;
    for(i=1;i<n;i++)
        if(s[i].at<minAt) minAt=s[i].at;
    time=minAt;
    while(completed<n){
        int idx=-1,bestPriority=INT_MAX;
        for(i=0;i<n;i++){
            if(s[i].at<=time && s[i].rt>0 && s[i].priority<bestPriority){
                bestPriority=s[i].priority;
                idx=i;
            }
        }
        if(idx==-1){
            time++;
            continue;
        }
        rawPid[rawCount++]=s[idx].pid;
        s[idx].rt--;
        time++;
        if(s[idx].rt==0){
            completed++;
            s[idx].tat=time-s[idx].at;
            s[idx].wt=s[idx].tat-s[idx].bt;
        }
    }
    int ganttPid[2000], ganttStart[2000], ganttEnd[2000], gc=0;
    int t=minAt;
    for(i=0;i<rawCount;){
        int curPid=rawPid[i];
        int start=t;
        while(i<rawCount && rawPid[i]==curPid){
            t++;
            i++;
        }
        ganttPid[gc]=curPid;
        ganttStart[gc]=start;
        ganttEnd[gc]=t;
        gc++;
    }
    printCompressedGantt(ganttPid,ganttStart,ganttEnd,gc);
    printf("\nProcess\tAT\tBT\tPriority\tWT\tTAT\n");
    for(i=0;i<n;i++){
        avgwt+=s[i].wt;
        avgtat+=s[i].tat;
        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\n",
        s[i].pid,s[i].at,s[i].bt,s[i].priority,
        s[i].wt,s[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f",avgwt/n);
    printf("\nAverage Turnaround Time = %.2f\n",avgtat/n);
}
void roundRobin(){
    struct Process r[20];
    int i;
    int tq;
    int time=0;
    int completed=0;
    int completedFlag[20]={0};
    int inQueue[20]={0};
    int queue[2000], front=0, rear=0;
    float avgwt=0,avgtat=0;
    int ganttPid[2000], ganttStart[2000], ganttEnd[2000], gc=0;
    for(i=0;i<n;i++){
        r[i]=p[i];
        r[i].rt=r[i].bt;
    }
    printf("Enter Time Quantum: ");
    scanf("%d",&tq);
    int minAt=r[0].at;
    for(i=1;i<n;i++)
        if(r[i].at<minAt) minAt=r[i].at;
    time=minAt;
    for(i=0;i<n;i++)
        if(r[i].at<=time && !inQueue[i] && !completedFlag[i]){
            queue[rear++]=i;
            inQueue[i]=1;
        }
    while(completed<n){
        if(front==rear){
            int nextAt=INT_MAX;
            for(i=0;i<n;i++)
                if(!completedFlag[i] && r[i].at<nextAt)
                    nextAt=r[i].at;
            time=nextAt;
            for(i=0;i<n;i++)
                if(r[i].at<=time && !inQueue[i] && !completedFlag[i]){
                    queue[rear++]=i;
                    inQueue[i]=1;
                }
            continue;
        }
        int idx=queue[front++];
        inQueue[idx]=0;
        int start=time;
        int exec=(r[idx].rt>tq)?tq:r[idx].rt;
        time+=exec;
        r[idx].rt-=exec;
        ganttPid[gc]=r[idx].pid;
        ganttStart[gc]=start;
        ganttEnd[gc]=time;
        gc++;
        for(i=0;i<n;i++)
            if(r[i].at<=time && !inQueue[i] && !completedFlag[i] && i!=idx){
                queue[rear++]=i;
                inQueue[i]=1;
            }
        if(r[idx].rt>0){
            queue[rear++]=idx;
            inQueue[idx]=1;
        }
        else{
            completedFlag[idx]=1;
            completed++;
            r[idx].tat=time-r[idx].at;
            r[idx].wt=r[idx].tat-r[idx].bt;
        }
    }
    printCompressedGantt(ganttPid,ganttStart,ganttEnd,gc);
    printf("\nProcess\tAT\tBT\tWT\tTAT\n");
    for(i=0;i<n;i++){
        avgwt+=r[i].wt;
        avgtat+=r[i].tat;
        printf("P%d\t%d\t%d\t%d\t%d\n",
        r[i].pid,r[i].at,r[i].bt,r[i].wt,r[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f",avgwt/n);
    printf("\nAverage Turnaround Time = %.2f\n",avgtat/n);
}
int main(){
    int i,choice;
    int needPriority=0;
    printf("Enter number of processes: ");
    scanf("%d",&n);
    for(i=0;i<n;i++)
    {
        p[i].pid=i+1;
        printf("Enter Arrival Time of P%d: ",i+1);
        scanf("%d",&p[i].at);
        printf("Enter Burst Time of P%d: ",i+1);
        scanf("%d",&p[i].bt);
    }
    do
    {
        printf("\n\nCPU Scheduling Menu\n");
        printf("1. FCFS\n2. SJF (Non-Preemptive)\n3.SRTF (Preemptive SJF)\n4. Priority Scheduling (Non-Preemptive)\n5. Priority Scheduling (Preemptive)\n6. Round Robin\n7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d",&choice);
        if((choice==4 || choice==5) && !needPriority)
        {
            for(i=0;i<n;i++)
            {
                printf("Enter Priority of P%d (lower number = higher priority): ",p[i].pid);
                scanf("%d",&p[i].priority);
            }
            needPriority=1;
        }
        switch(choice)
        {
            case 1:
                fcfs();
                break;
            case 2:
                sjf();
                break;
            case 3:
                srtf();
                break;
            case 4:
                priorityScheduling();
                break;
            case 5:
                preemptivePriority();
                break;
            case 6:
                roundRobin();
                break;
            case 7:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid Choice\n");
        }
    }while(choice!=7);
    return 0;
}