#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <inttypes.h>
#include "p3180215-p3150231-p3190307-pizza.h"

int main(int argc, char **argv)
{
   
    if (argc != 3)
    {
        print_error_message(argv[0]);
        exit(1);
    }


    print_init_param_msg(argv[0], argv[1], argv[2]);

    // the first argument is the number of customers
    customers = atoi(argv[1]);

    // the second arguments is the seed that is set by user
    unsigned int seedp = (unsigned) atoi(argv[2]);

    init(); 

    clock_gettime(CLOCK_MONOTONIC, &main_thread_start); 

    // create the threads
    create_threads_orders(threads, threads_data, customers, &seedp);

    join_orders(threads, customers);

    clock_gettime(CLOCK_MONOTONIC, &main_thread_end); 

    print_statistics();

    clear(); // release the resources
   

    return 0; 
}


//-----------------------------------------------------------------------------
void print_error_message(const char *name)
{
    printf("[x]> ERROR: The %s program needs 2 obligatory argumets.\n"
           "     The first argument represents the number of customers\n"
           "     the seconds represents the intial seed for the random\n"
           "     generator.\n",
           name);
}

//-----------------------------------------------------------------------------
void print_init_param_msg(const char *name,
                          const char *cust_num,
                          const char *init_seed)
{
    printf("[i]> The %s program will be executed with the"
           " following parameters:\n"
           "     -> 1st. Number of customers: %d\n     "
           "-> 2nd. Initial random seed: %d\n     "
           "-> The time is treated as SECONDS\n\n",
           name, atoi(cust_num), atoi(init_seed));
}

//-----------------------------------------------------------------------------
void print_statistics(void)
{
    unsigned int max = 0;
    unsigned int sum = 0;
    unsigned int cooling_max = 0;
    unsigned int cooling_sum = 0;

    for (int i = 0; i < customers; i++)
    {
        // delivery calculations
        max =
            threads_data[i]->delivery_duration > max
                ? threads_data[i]->delivery_duration
                : max;
        sum += threads_data[i]->delivery_duration;
        // cooling calculations
        cooling_max =
            threads_data[i]->cooling_duration > cooling_max
                ? threads_data[i]->cooling_duration
                : cooling_max;
        cooling_sum += threads_data[i]->cooling_duration;
    }
    unsigned int mean = sum / customers;  // calculate the mean of delivery
    unsigned int cooling_mean = sum / customers; // calculate the mean of cooling
    
    // print delivery statistics
    printf("\n\n[i]> MAX orders' delivery time: %u seconds\n", max);
    printf("[i]> MEAN orders' delivery time: %u seconds\n", mean);
    
    // print cooling statistics
    printf("\n\n[i]> MAX orders' cooling time: %u seconds\n", cooling_max);
    printf("[i]> MEAN orders' cooling time: %u seconds\n", cooling_mean);

    // Print total revenue and sales statistics
    printf("\n[i]> Total revenue: %.2f euros\n", total_revenue);
    printf("[i]> Pizzas sold - Margarita: %u, Pepperoni: %u, Special: %u\n", sales_margarita, sales_pepperoni, sales_special);

    // calculate the duration of the whole process
    unsigned int diff = calc_diff_sec(&main_thread_start, &main_thread_end);
    unsigned int diff_mins = diff / 60;
    unsigned int diff_secs = diff % 60;
    printf("\n[i]> Total time for program: %u minutes %u seconds\n",
           diff_mins, diff_secs);
}


void sleep_rand_r(unsigned int *__seedp_)
{
    sleep(RAND_BETWEEN(T_ORDER_LOW, T_ORDER_HIGH, __seedp_));
}


unsigned int calc_diff_sec(struct timespec *start,
                           struct timespec *end)
{
    
    return end->tv_sec - start->tv_sec;
}


void create_threads_orders(pthread_t *threads,
                           THREAD_DATA_T **threads_data,
                           unsigned int customers,
                           unsigned int *seedp)
{
    int rc = 0;
    unsigned int __seedp_ = *seedp;
    for (int i = 0; i < customers; i++)
    {
        threads_data[i] = create_thread_data((i + 1), __seedp_);
        threads_data[i]->num_pizzas = 
            RAND_BETWEEN(N_ORDER_LOW, N_ORDER_HIGH, &threads_data[i]->seedp);
        rc = pthread_create(&threads[i], null,
                            &orders, threads_data[i]);
        if (rc != 0)
        {
            pthread_mutex_lock(&print_lock); 
            printf("[x]> Failed to create order [%d]\n", i);
            pthread_mutex_unlock(&print_lock); 
        }
        sleep_rand_r(seedp); 
    }
}


void join_orders(pthread_t *threads, unsigned int customers)
{
    for (int i = 0; i < customers; ++i)
        pthread_join(threads[i], null);
}


struct thread_data *create_thread_data(unsigned int thread_number,
                                      unsigned int seedp)
{
    THREAD_DATA_T *new_thread =
        (THREAD_DATA_T *)malloc(sizeof(struct thread_data));
    if (new_thread == null)
        return null;                                
    new_thread->thread_num = thread_number; 
    new_thread->delivery_duration = 0;
    new_thread->cooling_duration = 0;   
    new_thread->seedp = seedp + thread_number; 
    return new_thread;
}


void init(void)
{

    
    threads = malloc(customers * sizeof(pthread_t));
    
    // initialize the memory for the data of the threads
    threads_data = malloc(customers * sizeof(THREAD_DATA_T));
   
    // initialize a lock for the stdout
    pthread_mutex_init(&print_lock, NULL);
   
    // initialize a lock for the cooks
    pthread_mutex_init(&cook_lock, NULL);
   
    // initialize a lock for the ovens
    pthread_mutex_init(&oven_lock, NULL);
    
    // initialize a lock for the telephone
    pthread_mutex_init(&telephone_lock, NULL);
    
    available_telephone_operators = N_TEL;
}

//-----------------------------------------------------------------------------
void clear(void)
{

    // free the threads
    free(threads);

    // clear the memory of threads data
    for (int i = 0; i < customers; i++)
        free(threads_data[i]);
    free(threads_data);

    // destroy the mutex for the stdout
    pthread_mutex_destroy(&print_lock);

    // destroy the mutex for the cooks
    pthread_mutex_destroy(&cook_lock);

    // destroy the condition variable for the cooks
    pthread_cond_destroy(&cook_cond);

    // destroy the mutex for the ovens
    pthread_mutex_destroy(&oven_lock);

    // destroy the condition variable for the ovens
    pthread_cond_destroy(&oven_cond);

    // destroy the mutexes for revenue and statistics
    pthread_mutex_destroy(&revenue_lock);
    pthread_mutex_destroy(&stats_lock);
    
    // destroy the mutex for the telephone
    pthread_mutex_destroy(&telephone_lock);
     // destroy the condition variable for the telephone
    pthread_cond_destroy(&telephone_cond);
    
    // destroy the mutex for the delivery
    pthread_mutex_destroy(&delivery_lock);
     // destroy the condition variable for the delivery
    pthread_cond_destroy(&delivery_cond);
}

//-----------------------------------------------------------------------------
void *orders(void *__data_)
{
    THREAD_DATA_T *data = (THREAD_DATA_T *)__data_; 

    unsigned int payment_time = RAND_BETWEEN(T_PAYMENT_LOW, T_PAYMENT_HIGH, &data->seedp);

    sleep(payment_time);

    double payment_failed = (double) rand() / RAND_MAX;
    if (payment_failed < P_FAIL)
    {
        pthread_mutex_lock(&print_lock);
        printf("[i]> Payment for Order [%d] failed. Cancelling order.\n", data->thread_num);
        pthread_mutex_unlock(&print_lock);

        
        pthread_mutex_lock(&stats_lock);
        failed_orders++;
        pthread_mutex_unlock(&stats_lock);

       
        pthread_mutex_lock(&telephone_lock);
        available_telephone_operators++;
        pthread_cond_signal(&telephone_cond);
        pthread_mutex_unlock(&telephone_lock);


        pthread_exit(NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &data->start);
    
    // decide the type of pizza based on probabilities
    double rand_val = (double)rand_r(&data->seedp) / RAND_MAX;
    
    double pizza_cost;

    if (rand_val < Pm)
    {
        pizza_cost = Cm;
        pthread_mutex_lock(&stats_lock);
        sales_margarita++;
        pthread_mutex_unlock(&stats_lock);
    }
    else if (rand_val < Pm + Pp)
    {
        pizza_cost = Cp;
        pthread_mutex_lock(&stats_lock);
        sales_pepperoni++;
        pthread_mutex_unlock(&stats_lock);
    }
    else
    {
        pizza_cost = Cs;
        pthread_mutex_lock(&stats_lock);
        sales_special++;
        pthread_mutex_unlock(&stats_lock);
    }

    // Update revenue
    pthread_mutex_lock(&revenue_lock);
    total_revenue += pizza_cost;
    pthread_mutex_unlock(&revenue_lock);
    
    
    pthread_mutex_lock(&telephone_lock);
    while (available_telephone_operators == 0)
    {
        pthread_mutex_lock(&print_lock);
        printf("[i]> Order [%d] is waiting for a free telephone operator.\n", data->thread_num);
        pthread_mutex_unlock(&print_lock);

        pthread_cond_wait(&telephone_cond, &telephone_lock);
    }
    available_telephone_operators--;
    pthread_mutex_unlock(&telephone_lock);

    pthread_mutex_lock(&print_lock);
    printf("[i]> Order [%d] has been assigned a telephone operator.\n", data->thread_num);
    pthread_mutex_unlock(&print_lock);
    

    pthread_mutex_lock(&print_lock);
    printf("[i]> Payment for Order [%d] successful. Registering order.\n", data->thread_num);
    pthread_mutex_unlock(&print_lock);

    
    sleep(T_ORDER_HIGH);

   
    pthread_mutex_lock(&telephone_lock);
    available_telephone_operators++;
    pthread_cond_signal(&telephone_cond);
    pthread_mutex_unlock(&telephone_lock);

    pthread_mutex_lock(&print_lock);
    printf("[i]> Order [%d] has released a telephone operator.\n", data->thread_num);
    pthread_mutex_unlock(&print_lock);
    

    // lock to try to commit a cook
    pthread_mutex_lock(&cook_lock); 
    while (ncook_c == 0)
    {
        pthread_mutex_lock(&print_lock); 
        printf("[i]> Order [%d] didn't find a free cook.\n",data->thread_num);
        printf("[i] Order number %d failed(cook).\n", data->thread_num);
        pthread_mutex_unlock(&print_lock); 
        
        pthread_cond_wait(&cook_cond, &cook_lock);
    } 
    ncook_c--;                        
    pthread_mutex_unlock(&cook_lock); 



    for (int i = 0; i < data->num_pizzas; i++)
        sleep(T_PREP);

    pthread_mutex_lock(&print_lock); 
    printf("[i]> Order [%d] is prepared\n", data->thread_num);
    pthread_mutex_unlock(&print_lock); 

    // lock to try to commit an oven
    pthread_mutex_lock(&oven_lock); 
    while (noven_c == 0)
    {
        pthread_mutex_lock(&print_lock);
        printf("[i]> Order [%d] didn't find a free oven.\n",data->thread_num);
        printf("[i] Order number %d failed(oven).\n", data->thread_num);
        pthread_mutex_unlock(&print_lock); 
        
        pthread_cond_wait(&oven_cond, &oven_lock);
    } 
    noven_c--;                        
    pthread_mutex_unlock(&oven_lock); 

    //lock to release a cook after the commitment of the oven
    pthread_mutex_lock(&cook_lock);
    ncook_c++;                          
    pthread_cond_broadcast(&cook_cond); 
    pthread_mutex_unlock(&cook_lock);

    pthread_mutex_lock(&print_lock);
    printf("[i]> Order [%d] has been placed to the oven and a cook has been released\n",
           data->thread_num);
    pthread_mutex_unlock(&print_lock); 


    sleep(T_BAKE);

    
    clock_gettime(CLOCK_MONOTONIC, &data->baked);

    pthread_mutex_lock(&print_lock); 
    printf("[i]> Order [%d] has been baked\n", data->thread_num);
    pthread_mutex_unlock(&print_lock); 

    
    // lock to try to commit a delivery
    pthread_mutex_lock(&delivery_lock); 
    while (ndeliverer_c == 0)
    {
        pthread_mutex_lock(&print_lock); 
        printf("[i]> Order [%d] didn't find a free delivery.\n",data->thread_num);
        printf("[i] Order number %d failed(delivery).\n", data->thread_num);
        pthread_mutex_unlock(&print_lock); 
        pthread_cond_wait(&delivery_cond, &delivery_lock);
    } 

    // wait the packing time
    sleep(T_PACK);

    // save the time where the order has been packed
    clock_gettime(CLOCK_MONOTONIC, &data->packed); 
    ndeliverer_c--;                        
    pthread_mutex_unlock(&delivery_lock); 

    unsigned int diff_packed_sec = calc_diff_sec(&data->start, &data->packed);

    pthread_mutex_lock(&print_lock); 
    printf("[i]> Order [%d] packed in %u secs\n",
           data->thread_num, diff_packed_sec);
    pthread_mutex_unlock(&print_lock); 

    //lock to release an oven
    pthread_mutex_lock(&oven_lock); 
    noven_c++;      
    pthread_cond_broadcast(&oven_cond); 
    pthread_mutex_unlock(&oven_lock);   

    pthread_mutex_lock(&print_lock); 
    printf("[i]> Order [%d] an oven has been released\n",
           data->thread_num);
    pthread_mutex_unlock(&print_lock); 

    // the time to go to transfer pizza to the customer
    unsigned int time_to_transfer =
        RAND_BETWEEN(T_DELLOW, T_DELHIGH, &data->seedp);

    sleep(time_to_transfer); 

    // save the time where the order has been transfered to the customer
    clock_gettime(CLOCK_MONOTONIC, &data->end); // MT-safe

    pthread_mutex_lock(&print_lock); 
    printf("[i]> Order [%d] order has been transfered to the customer\n",
           data->thread_num);
    pthread_mutex_unlock(&print_lock); 

    sleep(time_to_transfer); 

    //lock to release a delivery
    pthread_mutex_lock(&delivery_lock); 
    ndeliverer_c++;                      
    pthread_cond_broadcast(&delivery_cond); 
    pthread_mutex_unlock(&delivery_lock);   

    pthread_mutex_lock(&print_lock); 
    printf("\n[i]> Order [%d] delivery has been returned and released for next order\n",
           data->thread_num);
    pthread_mutex_unlock(&print_lock); 
    

    unsigned int diff_sec = calc_diff_sec(&data->start, &data->end);
    data->delivery_duration = diff_sec;
    unsigned int diff_cool_sec = calc_diff_sec(&data->baked, &data->end);
    data->cooling_duration = diff_cool_sec;

    // update the statistics for successful orders
    pthread_mutex_lock(&stats_lock);
    successful_orders++;
    pthread_mutex_unlock(&stats_lock);

    pthread_mutex_lock(&print_lock); 

    printf("[i]> Order [%d] delivered in %u secs\n",
           data->thread_num, diff_sec);

    pthread_mutex_unlock(&print_lock);
    return null;
}
