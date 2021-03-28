/* autor: xgorca00
starts: 18.4.2020
 */



#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<semaphore.h>
#include<sys/mman.h>
#include<time.h>
#include<sys/stat.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<fcntl.h>
#include<limits.h>
#include<string.h>


#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer) {munmap((pointer), sizeof((pointer)));}

 /* names are wierd because iam programing on mac and ON sem_open should be name as sem_t */

sem_t *xgorca00_semafor_counter = NULL;             /* for counting each number of any proces */
sem_t *xgorca00_semafor_no_judge = NULL;            /* if judgfe is in hall noone can leave or entered hall*/
sem_t *xgorca00_semafor_certificate = NULL;         /* when judge starts confirmation noone can got the registration*/
sem_t *xgorca00_semafor_register = NULL;            /* when checked != entered judge must wait until ==*/
sem_t *xgorca00_semafor_mutex = NULL;
sem_t *xgorca00_semafor_end = NULL;                 /* judge finished and waiting for last INM to leave*/

FILE *file;

int *counter = NULL;        /* for counting each number of any proces */
int *entered = NULL;        /* how many INM are in hall */
int *checked = NULL;                    /* how many INM are checked*/
int *how_many_processes = NULL;         /* in main is set to number of INM and is decrementing in each proces*/
int *in_hall = NULL;                    /* how many INM are in hall*/
int *judge = NULL;                     /* set to 1 or 0 it depends on judge if she is in hall or not*/
int *done = NULL;                       /* how many imigrants leaves*/

void init() {  



    file = fopen("proj2.out","w");
    setbuf(file,NULL);

    if(file == NULL)
    {
        printf("Error!");   
        exit(1);             
    }

     if  ((xgorca00_semafor_end = sem_open("/xgorca00_semafor_end", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
        printf("sem_open: ");
        exit(EXIT_FAILURE);
    }

    if  ((xgorca00_semafor_register = sem_open("/xgorca00_semafor_register", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED){
        printf("sem_open: ");
        exit(EXIT_FAILURE);
    }

 
    if  ((xgorca00_semafor_counter = sem_open("/xgorca00_semafor_counter", O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED){
        printf("sem_open: ");
        exit(EXIT_FAILURE);
    }
     
    if ((xgorca00_semafor_no_judge = sem_open("/xgorca00_semafor_no_judge", O_CREAT|O_EXCL, 0666, 1)) == SEM_FAILED) {
        printf("sem_open: ");
        exit(EXIT_FAILURE);
   }

   if ((xgorca00_semafor_certificate = sem_open("/xgorca00_semafor_certificate", O_CREAT|O_EXCL, 0666, 0)) == SEM_FAILED) {
        printf("sem_open: ");
        exit(EXIT_FAILURE);
   }
    
    MMAP(how_many_processes);
    MMAP(counter);
    MMAP(entered);
    MMAP(checked);
    MMAP(in_hall);
    MMAP(judge);
    MMAP(done);
}



void cleanup(){

     fclose(file);

    UNMAP(how_many_processes);
    UNMAP(counter);
    UNMAP(entered);
    UNMAP(checked); 
    UNMAP(in_hall);
    UNMAP(judge);
    UNMAP(done);

    if (sem_close(xgorca00_semafor_end) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/xgorca00_semafor_end") == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
     }   

     if (sem_close(xgorca00_semafor_register) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/xgorca00_semafor_register") == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
     }   

    if (sem_close(xgorca00_semafor_counter) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/xgorca00_semafor_counter") == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
     }

    if (sem_close(xgorca00_semafor_no_judge) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/xgorca00_semafor_no_judge") == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }

     if (sem_close(xgorca00_semafor_certificate) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink("/xgorca00_semafor_certificate") == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    
}

void print(char name[], int num_of_inm,char name2[]){
    if((strcmp(name2,"leaves")) == 0){
            fprintf(file,"%-10d: %s%-5d  %s:%-17s : %-7d : %-7d : %d \n",*counter,name,num_of_inm,name2,"",*entered,*checked,*in_hall);
    }
    else if((strcmp(name2,"wants to certificate")) == 0){
            fprintf(file,"%-10d: %s%-5d  %s:%-3s : %-7d : %-7d : %d \n",*counter,name,num_of_inm,name2,"",*entered,*checked,*in_hall);
    }
    else if((strcmp(name2,"enters")) == 0){
            fprintf(file,"%-10d: %s%-5d  %s:%-18s: %-7d : %-7d : %d \n",*counter,name,num_of_inm,name2,"",*entered,*checked,*in_hall);
    }
    else if((strcmp(name2,"starts")) == 0){
        fprintf(file,"%-10d: %s%-5d  %s:\n",*counter,name,num_of_inm,name2);
    }
    else if((strcmp(name2,"got certificate")) == 0){
        fprintf(file,"%-10d: %s%-5d  %s:%-9s: %-7d : %-7d : %d \n",*counter,name,num_of_inm,name2,"",*entered,*checked,*in_hall);
    }
    else{
        fprintf(file,"%-10d: %s%-5d  %s:%-18s: %-7d : %-7d : %d \n",*counter,name,num_of_inm,name2,"",*entered,*checked,*in_hall);
    }
    
}

void print_judge(char name[],char name2[]){
    if((strcmp(name2,"enter")) == 0){
            fprintf(file,"%-10d: %-8s  %s:%-19s: %-7d : %-7d : %d \n",*counter,name,name2,"",*entered,*checked,*in_hall);
    }
    else if((strcmp(name2,"waits for INM")) == 0){
            fprintf(file,"%-10d: %-8s  %s:%-11s: %-7d : %-7d : %d \n",*counter,name,name2,"",*entered,*checked,*in_hall);
    }
    else if((strcmp(name2,"starts confirmation")) == 0){
            fprintf(file,"%-10d: %-8s  %s:%-5s: %-7d : %-7d : %d \n",*counter,name,name2,"",*entered,*checked,*in_hall);
    }
    else if((strcmp(name2,"end confirmation")) == 0){
            fprintf(file,"%-10d: %-8s  %s:%-8s: %-7d : %-7d : %d \n",*counter,name,name2,"",*entered,*checked,*in_hall);
    }
     else if((strcmp(name2,"wants to enter")) == 0){
            fprintf(file,"%-10d: %-8s  %s:\n",*counter,name,name2);
    }
    else if((strcmp(name2,"leaves")) == 0){
            fprintf(file,"%-10d: %-8s  %s:%-18s: %-7d : %-7d : %d \n",*counter,name,name2,"",*entered,*checked,*in_hall);
    }
    else{
            fprintf(file,"%-10d: %-8s  %s:\n",*counter,name,name2);
    }
    
}

int check_arguments(char arg1[],char arg2[],char arg3[],char arg4[],char arg5[]){

    int first = atoi(arg1);   /*num of proceses for imigrants */
    int second = atoi(arg2);     /*max-time for generating process imigrants */
    int third = atoi(arg3);      /*max-time for judge to come to hall*/
    int fourth = atoi(arg4);      /* certificating*/
    int fifth = atoi(arg5);


    if (first < 1){
        return -1;
    }
    else if(second < 0 || second > 2000){
        return -1;
    }
    else if(third < 0 || third > 2000){
        return -1;
    }
    else if(fourth < 0 || fourth > 2000){
        return -1;
    }
    else if(fifth < 0 || fifth > 2000){
        return -1;
    }

    return 1;

}


void misleep(int timer){   /* random sleep time generator */
       
    srand(time(0)); 
    if (timer != 0){

        usleep(rand() % timer * 1000) ;
        
    }

}

void INM_REGISTER(int num_of_inm, int it){                  /* INM are registered */
    sem_wait(xgorca00_semafor_counter);{
        (*counter)++;
        print("INM",num_of_inm,"wants to certificate");
        
    }
    sem_post(xgorca00_semafor_counter);
    
    misleep(it);   

    sem_wait(xgorca00_semafor_counter);{
        (*counter)++;
        (*done)--;
        print("INM",num_of_inm,"got certificate");
    }
    sem_post(xgorca00_semafor_counter);
}



void INM_checking(int num_of_inm) {                 /* INM are checked*/
    
    sem_wait(xgorca00_semafor_counter); {
            (*counter)++;
            (*checked)++;
            print("INM",num_of_inm,"checks");
    }
    sem_post(xgorca00_semafor_counter);
}

void proces_INM(int num_of_inm, int it){

    sem_wait(xgorca00_semafor_counter);
            (*counter)++;
            print("INM",num_of_inm,"starts");
            sem_post(xgorca00_semafor_counter);

    sem_wait(xgorca00_semafor_no_judge);
        
        sem_wait(xgorca00_semafor_counter);
            {
                
             (*counter)++;
             (*entered)++;
             (*in_hall)++;
             print("INM",num_of_inm,"enters");
         }
        
        sem_post(xgorca00_semafor_counter);
    sem_post(xgorca00_semafor_no_judge);

        sem_trywait(xgorca00_semafor_register);{

            INM_checking(num_of_inm);
        }

        if((*entered == *checked) && (*judge == 1)){
        
        sem_post(xgorca00_semafor_register);
        }

        sem_wait(xgorca00_semafor_certificate);
         INM_REGISTER(num_of_inm, it);
        

    sem_wait(xgorca00_semafor_no_judge);
        sem_wait(xgorca00_semafor_counter);
        {
            (*counter)++;
            (*in_hall)--;
            print("INM",num_of_inm,"leaves");
            (*how_many_processes)--;
            
        }
        sem_post(xgorca00_semafor_counter);
      sem_post(xgorca00_semafor_no_judge); 

  
    exit(0);
}

void judge_in_hall(int it){

    sem_wait(xgorca00_semafor_counter);
            {
                (*counter)++;
                print_judge("judge","wants to enter");
            }
            
            sem_post(xgorca00_semafor_counter);

    sem_wait(xgorca00_semafor_no_judge);{
    
        sem_wait(xgorca00_semafor_counter);
            *judge = 1;
            (*counter)++;
            print_judge("judge","enter");
        
    
        if(*entered > *checked){
                     
                    (*counter)++;
                    print_judge("judge","waits for INM");
                    
                    sem_post(xgorca00_semafor_counter);  
                    sem_wait(xgorca00_semafor_register);
         }   
        else
        {
           sem_post(xgorca00_semafor_counter);  
        }
        
        
        sem_trywait(xgorca00_semafor_certificate);{
            sem_wait(xgorca00_semafor_counter);{

                    (*counter)++;
                    print_judge("judge","starts confirmation");
                    
                    misleep(it);

                    (*counter)++;
                    
                    (*checked) = 0;
                    (*entered) = 0;
                    print_judge("judge","end confirmation");
            }
            sem_post(xgorca00_semafor_counter);


        }
        if(*in_hall > 0){
            sem_post(xgorca00_semafor_certificate);
        }

        
        misleep(it);
        sem_wait(xgorca00_semafor_counter);{

            (*counter)++;
            *judge = 0;
            print_judge("judge","leaves");
        }
        sem_post(xgorca00_semafor_counter);

    }        
    sem_post(xgorca00_semafor_no_judge);
}


 
void proces_judge(int time_for_repeat, int jt)
{
    
    while(1)
    {
        misleep(time_for_repeat);
        if(*done == 0){
            sem_wait(xgorca00_semafor_counter);
            (*counter)++;
            print_judge("judge","finishes");
            sem_post(xgorca00_semafor_counter);
            break;
        }
            judge_in_hall(jt);
        
    }  
   exit(0);
}

void imigrants_generator(int number_of_imigrants,int time_for_generating, int it)
{
   
    for (int i = 1; i < number_of_imigrants + 1; ++i){
         
         pid_t INM = fork();
         
        if (INM < 0) {
        fprintf( stderr, "problem with fork\n" );
        exit(1);
        }
        else if (INM == 0){
            misleep(time_for_generating);
            proces_INM(i,it);
        }
        
    }
    for (int i = 0; i < number_of_imigrants  ;i ++){
        wait(NULL);
    }

    exit(0);
   
}

int main(int argc, char *argv[])
{
    if (argc < 6 || argc > 6){
        fprintf( stderr, "wrong arguments\n" );
        exit(1);
    }
    else if (check_arguments(argv[1],argv[2],argv[3],argv[4],argv[5]) == -1){
        fprintf( stderr, "wrong arguments\n" );
        exit(1);
    }

    int pi = atoi(argv[1]);   /*num of proceses for imigrants */
    int ig = atoi(argv[2]);     /*max-time for generating process imigrants */
    int jg= atoi(argv[3]);      /*max-time for judge to come to hall*/
    int it= atoi(argv[4]);      /* certificating*/
    int jt = atoi(argv[5]);     /* delay of judging decision*/

    init();                     /* initializing  semaphores and shared variables*/

    *judge = 0;
    *counter = 0;
    *entered = 0;
    *checked = 0;
    *how_many_processes = pi;       /* setting to pi becauces we will decrement it and finding if it is evrithing done*/
    *done = pi;
    
    pid_t hall = fork();
    
    if (hall < 0) {
        fprintf( stderr, "problem with fork\n" );
        exit(1);
    }
    else if (hall == 0) {
            imigrants_generator(pi,ig,it);
         
    }
    else{
        pid_t JUD_P  = fork();
        if (JUD_P   < 0) {
        fprintf( stderr, "problem with fork\n" );
        exit(1);
        }
        if (JUD_P == 0){
         proces_judge(jg,jt); 
        }
        waitpid(JUD_P ,NULL,0);
        }
    

   waitpid(hall,NULL,0);
    
    cleanup();              /* cleaning semaphores and shared variables*/
    

    return 0;

}
