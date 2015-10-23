#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

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
   teller->checked_in = 1; 
   teller->doing_service = 0;
 
   // TODO: add teller to list
   if (teller_list == NULL) {
      teller_list = teller;
   } else {
      teller->next = teller_list->next;
      teller_list = teller;
   }
}

void teller_check_out(p_teller teller)
{
   // TODO: check if the teller is done
   // if not, wait for teller to be done
 
   // TODO: remove teller from list
 
   teller->checked_in = 0;     
}

p_teller do_banking(int customer_id)
{
   pthread_mutex_lock(&mutex);

   // TODO: check if list contains a teller (=teller is available)
   // if not, wait for teller to become available 
   while (teller_list == NULL) {
      pthread_cond_wait(&teller_available, &mutex)
   }

   printf("Customer %d is served by teller %d\n", customer_id, teller->id);
   teller->doing_service = 1;
   pthread_mutex_unlock(&mutex);

   return teller;
}

void finish_banking(int customer_id, p_teller teller)
{
   pthread_mutex_lock(&mutex);
   while (teller.doing_service) {
      pthread_cond_wait(&teller.done, &mutex);
   }
   printf("Customer %d is done with teller %d\n", customer_id, teller->id);

   // Put teller back to the teller_list
   // change the teller_available to true

   teller->doing_service = 0;
   // TODO: re-enter teller into list
}

void* teller(void *arg)
{
   p_teller me = (p_teller) arg;
  
   // perform an initial checkin
   teller_check_in(me);

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

   // TODO: initialize mutexes/condition variables here
   pthread_mutex_init(&mutex, NULL);

   for (i=0; i<NUM_TELLERS; i++) 
   { 
      tellers[i].id = i;
      tellers[i].next = NULL;
      
      // TODO: create teller threads here
      pthread_create(&tellers[i].thread, NULL, )
   }

   pthread_t thread;

   for (i=0; i<NUM_CUSTOMERS; i++) 
   {
      // TODO: create customer threads here
   }

   for (i=0; i<NUM_TELLERS; i++) 
   {
      // wait for all tellers to be done
      pthread_join(tellers[i].thread, NULL);
   }

   return 0;
}
