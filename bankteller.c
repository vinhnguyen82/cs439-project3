#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// #define DEBUG

static pthread_mutex_t mutex;
static pthread_cond_t teller_available;

typedef struct teller_info_t {
   int id;
   int checked_in;
   int doing_service;
   pthread_cond_t done;
   pthread_t thread;
   struct teller_info_t *next;
} *p_teller;

static p_teller teller_list = NULL;


void teller_check_in(p_teller teller) 
{
   pthread_mutex_lock(&mutex);
   teller->checked_in = 1; 
   teller->doing_service = 0;
 
   // TODO: add teller to list
   pthread_cond_init(&teller->done, NULL);

   if (teller_list == NULL) {
      // if teller_list is empty
      teller_list = teller;
      teller_list->next = NULL;
   } else {
      // if not, add to the front
      teller->next = teller_list;
      teller_list = teller;
   }

   // signal the threads waiting for teller
   pthread_cond_signal(&teller_available);
   pthread_mutex_unlock(&mutex);
}

void teller_check_out(p_teller teller)
{
   pthread_mutex_lock(&mutex);
   // check if the teller is done
   // if not, wait for teller to be done
   while (teller->doing_service) {
      pthread_cond_wait(&teller->done, &mutex);
   }
   // remove teller from list
   if (teller == teller_list) {
      // remove easily if teller at the beginning of the list
      teller_list = teller_list->next;
      teller->next = NULL;
   } else {
      // find the node before the teller node
      p_teller temp = teller_list;
      while (temp->next != teller) {
         temp = temp->next;
      }
      // remove the teller
      temp->next = teller->next;
      teller->next = NULL;
   }
   teller->checked_in = 0;    
   pthread_mutex_unlock(&mutex);
}

p_teller do_banking(int customer_id)
{
   pthread_mutex_lock(&mutex);

   // check if list contains a teller (=teller is available)
   // if not, wait for teller to become available 
   while (teller_list == NULL) {
      pthread_cond_wait(&teller_available, &mutex);
   }
   // remove the teller from available teller list
   p_teller teller = teller_list;
   teller_list = teller_list->next;
   teller->next = NULL;
   printf("Customer %d is served by teller %d\n", customer_id, teller->id);
   teller->doing_service = 1;
   pthread_mutex_unlock(&mutex);
   return teller;
}

void finish_banking(int customer_id, p_teller teller)
{
   pthread_mutex_lock(&mutex);
   printf("Customer %d is done with teller %d\n", customer_id, teller->id);

   // Put teller back to the teller_list
   teller->doing_service = 0;
   teller->next = teller_list;
   teller_list = teller;
   pthread_cond_signal(&teller_available);
   pthread_cond_signal(&teller->done);
   pthread_mutex_unlock(&mutex);
}

void* teller(void *arg)
{
   p_teller me = (p_teller) arg;
  
   // perform an initial checkin
   teller_check_in(me);
   #ifdef DEBUG
   printf("teller %d checks in outside while!\n", me->id); 
   #endif

   while(1)
   {
      // in 5% of the cases toggle status
      int r = rand();
      if (r < (0.05 * RAND_MAX))
      {
         if (me->checked_in) 
         {
            printf("teller %d checks out\n", me->id); 
            teller_check_out(me);
            // uncomment line below to let program terminate for testing
            #ifdef DEBUG
            printf("teller %d really checks out!\n", me->id); 
            #endif
            // return 0;
         } else {
            printf("teller %d checks in\n", me->id);
            teller_check_in(me);
         }
      }
   }
}

void* customer(void *arg)
{
   int id = (uintptr_t) arg;

   while (1)
   {
      // in 10% of the cases enter bank and do business
      int r = rand();
      if (r < (0.1 * RAND_MAX))
      {
         p_teller teller = do_banking(id);
         // sleep up to 3 seconds
         sleep(r % 4);
         finish_banking(id, teller);
      }
   }
}

#define NUM_TELLERS 3
#define NUM_CUSTOMERS 5

int main(void)
{
   srand(time(NULL));
   struct teller_info_t tellers[NUM_TELLERS];
   int i;

   // initialize mutexes/condition variables here
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&teller_available, NULL);

   for (i=0; i<NUM_TELLERS; i++) 
   { 
      tellers[i].id = i;
      tellers[i].next = NULL;
      
      // create teller threads
      pthread_create(&tellers[i].thread, NULL, teller, (void *) &tellers[i]);
   }

   pthread_t thread;

   for (i=0; i<NUM_CUSTOMERS; i++) 
   {
      // create customer threads
      pthread_create(&thread, NULL, customer, (void*) (uintptr_t) i);
   }

   for (i=0; i<NUM_TELLERS; i++) 
   {
      // wait for all tellers to be done
      pthread_join(tellers[i].thread, NULL);
   }

   return 0;
}
