#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_func(void *arg)
{
    int mode = *((int *)arg);
    if (mode == 1)
    {
        printf("ნაკადი 1: ვსრულდები return-ით\n");
        return (void *)10;
    }
    else if (mode == 2)
    {
        printf("ნაკადი 2: ვსრულდები pthread_exit-ით\n");
        pthread_exit((void *)20);
    }
    else
    {
        while (1)
        {
            printf("ნაკადი 3: ვმუშაობ უსასრულოდ... \n");
            sleep(1);
        }
    }
}

int main()
{
    pthread_t t1, t2, t3;
    int m1 = 1, m2 = 2, m3 = 3;

    void *res1, *res2;

    pthread_create(&t1, NULL, thread_func, &m1);
    pthread_create(&t2, NULL, thread_func, &m2);
    pthread_create(&t3, NULL, thread_func, &m3);

    pthread_join(t1, &res1);
    pthread_join(t2, &res2);
    printf("მიღებული შედეგები:  %ld, %ld\n", (long)res1, (long)res2);

    sleep(4);
    printf("მთავარი: ვაუქმებ მე-3 ნაკადს\n");
    pthread_cancel(t3);
    pthread_join(t3, NULL);

    printf("ყველა ნაკადი დაიხურა\n");
    return 0;
}