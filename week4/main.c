#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* thread_work(void* p_param)
{
    printf("მეორადი ნაკადი: დაიწყო მუშაობა\n");
    sleep(2);
    printf("მეორადი ნაკადი: დასრულდა მუშაობა\n");
    return 0;
}

int main()
{
    pthread_t thread;
    
    if(pthread_create(&thread, 0, thread_work, 0)) {
        perror("pthread_create()");
        return 1;
    }

    printf("პირველადი ნაკადი ელოდება მეორადი ნაკადს... \n");
    pthread_join(thread, 0);
    printf("პირველადი ნაკადი: ყველაფერი დასრულდა\n");

    return 0;
}