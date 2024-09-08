#include <pthread.h>
#include <time.h>

#if !defined(ONE_MINUTE)
#define ONE_MINUTE 60
#endif

#if !defined(N_TEL)
#define N_TEL 2  
#endif

#if !defined(N_COOK)
#define N_COOK 2
#endif

#if !defined(N_OVEN)
#define N_OVEN 10
#endif

#if !defined(N_DELIVERER)
#define N_DELIVERER 10
#endif

#if !defined(T_ORDER_LOW)
#define T_ORDER_LOW (1 * ONE_MINUTE)
#endif

#if !defined(T_ORDER_HIGH)
#define T_ORDER_HIGH (5 * ONE_MINUTE)
#endif

#if !defined(N_ORDER_LOW)
#define N_ORDER_LOW 1
#endif

#if !defined(N_ORDER_HIGH)
#define N_ORDER_HIGH 5
#endif

#if !defined(T_PREP)
#define T_PREP (1 * ONE_MINUTE)
#endif

#if !defined(T_BAKE)
#define T_BAKE (10 * ONE_MINUTE)
#endif

#if defined(NULL) && !defined(null)
#define null NULL
#endif

#if !defined(T_PAYMENT_LOW)
#define T_PAYMENT_LOW (1 * ONE_MINUTE) 
#endif

#if !defined(T_PAYMENT_HIGH)
#define T_PAYMENT_HIGH (3 * ONE_MINUTE)
#endif

#if !defined(T_PACK)
#define T_PACK (1 * ONE_MINUTE)
#endif

#if !defined(T_DELLOW)
#define T_DELLOW (5 * ONE_MINUTE)
#endif

#if !defined(T_DELHIGH)
#define T_DELHIGH (15 * ONE_MINUTE)
#endif

#if !defined(Cm)
#define Cm 10
#endif

#if !defined(Cp)
#define Cp 11
#endif

#if !defined(Cs)
#define Cs 12
#endif

#if !defined(Pm)
#define Pm 0.35
#endif

#if !defined(Pp)
#define Pp 0.25
#endif

#if !defined(Ps)
#define Ps 0.4
#endif

#if !defined(P_FAIL)
#define P_FAIL 0.05
#endif



double total_revenue;
unsigned int sales_margarita;
unsigned int sales_pepperoni;
unsigned int sales_special;
unsigned int successful_orders;
unsigned int failed_orders;

//locks for the stats
pthread_mutex_t revenue_lock;
pthread_mutex_t stats_lock;

pthread_mutex_t print_lock;

pthread_mutex_t cook_lock; //lock for the cook
pthread_cond_t cook_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t oven_lock; //lock for the oven
pthread_cond_t oven_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t delivery_lock; //lock for the delivery
pthread_cond_t delivery_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t telephone_lock; //lock for the telephone
pthread_cond_t telephone_cond = PTHREAD_COND_INITIALIZER;
int available_telephone_operators;




#if !defined(RAND_BETWEEN)
#define RAND_BETWEEN(low, high, seedp_ptr) \
    ((rand_r(seedp_ptr) % high) + low)
#endif


typedef struct thread_data
{
    unsigned int thread_num;   // order data
    unsigned int num_pizzas;   // # of pizzas
    unsigned int seedp;        // seed for the rand_r for each thread
    struct timespec start;     // start of order time
    struct timespec baked;     // just baked time
    struct timespec packed;    // just packed time
    struct timespec end;       // until transfer to customer
    unsigned int delivery_duration; // delivery duration
    unsigned int cooling_duration;  // cooling duration
} THREAD_DATA_T;


void print_error_message(const char *name);


void print_init_param_msg(const char *name,
                          const char *cust_num,
                          const char *init_seed);


void sleep_rand_r(unsigned int *__seedp_);


void create_threads_orders(pthread_t *threads,
                           THREAD_DATA_T **threads_data,
                           unsigned int customers,
                           unsigned int *seedp);


void join_orders(pthread_t *threads, unsigned int customers);


THREAD_DATA_T *create_thread_data(unsigned int thread_number,
                                 unsigned int seedp);


unsigned int calc_diff_sec(struct timespec *__start_,
                           struct timespec *__end_);


void print_statistics(void);


void init(void);


void clear(void);


void *orders(void *);


unsigned int ncook_c = N_COOK; // the current number of cooks


unsigned int noven_c = N_OVEN; // the current number of ovens


unsigned int ndeliverer_c = N_DELIVERER; // the current number of deliveries

unsigned int ntelephone_c = N_TEL; // the current number of telephones


pthread_t *threads = null; 


THREAD_DATA_T **threads_data = null; 

unsigned int customers = 0; 

struct timespec main_thread_start; 

struct timespec main_thread_end; 
