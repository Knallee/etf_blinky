#include "scheduler.h"
#include "timer.h"

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>


static pthread_mutex_t g_mut;
static atomic_int run = 1;

#define MAINTAG "[MAIN]: "
#define ISRTAG  "\t\t\t[ISR]: "


void sys_enterCritical(void)
{
    pthread_mutex_lock(&g_mut);
}


void sys_exitCritical(void)
{
    pthread_mutex_unlock(&g_mut);
}


uint32_t timerGetUptime(void)
{
    // TODO(noxet): use system uptime
    return 0;
}


void *mainThread(void *arg)
{
    (void) arg;

    int processedEvents = 0;
    while (processedEvents < 20)
    {
        event_s ev = evRun();
        switch (ev.code)
        {
            case EV_NOP:
                printf(MAINTAG "EV_NOP: %d\n", ev.eventData);
                break;
            case EV_BUTTON_PRESSED:
                printf(MAINTAG "EV_PRESSED: %d\n", ev.eventData);
                processedEvents++;
                break;
            case EV_BUTTON_RELEASED:
                printf(MAINTAG "EV_RELEASED: %d\n", ev.eventData);
                processedEvents++;
                break;
            case EV_BUTTON_ISR_DISABLED:
                break;
        }

        //sleep(1);
    }

    run = 0;
    return NULL;
}


void *isrThread(void *arg)
{
    (void) arg;

    static int nice = 69;
    static int chill = 420;
    while (run)
    {
        event_s ev = NEW_EVENT();
        ev.code = EV_BUTTON_PRESSED;
        ev.eventData = nice++;
        printf(ISRTAG "Add event %d: %d\n", ev.code, ev.eventData);
        bool ret = evAdd(&ev, &(uint32_t){TIME_NOW});
        if (!ret) printf(ISRTAG "Event handler full!\n");

        //usleep(600000);

        ev.code = EV_BUTTON_RELEASED;
        ev.eventData = chill++;
        printf(ISRTAG "Add event %d: %d\n", ev.code, ev.eventData);
        ret = evAdd(&ev, &(uint32_t){TIME_NOW});
        if (!ret) printf(ISRTAG "Event handler full!\n");

        //usleep(600000);
    }

    return NULL;
}


int main(void)
{
    printf("Running scheduler test\n");

    pthread_mutex_init(&g_mut, NULL);

    pthread_t tMain;
    pthread_t tISR;

    evInit();

    pthread_create(&tMain, NULL, mainThread, NULL);
    pthread_create(&tISR, NULL, isrThread, NULL);

    pthread_join(tMain, NULL);
    pthread_join(tISR, NULL);
}
