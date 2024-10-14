Pizza Ordering System

Overview

This project simulates a multi-threaded pizza ordering and delivery system where customers place orders, and the system handles cooking, baking, and delivering pizzas. The simulation allows you to observe and analyze how different resources (cooks, ovens, delivery personnel, etc.) are managed and utilized in fulfilling orders.
The system makes use of POSIX threads (pthreads) and demonstrates concepts like thread synchronization, mutexes, and condition variables to coordinate shared resources.

Features

Simulates multiple customers ordering pizzas with varying quantities.
Manages available telephone operators, cooks, ovens, and delivery personnel.
Uses mutexes and condition variables for resource management.
Tracks statistics like total revenue, number of pizzas sold by type, and delivery times.
Supports random variations in order preparation times, baking times, and delivery times.


To run the program: ./a.out <number_of_customers> <random_seed>
Example: ./a.out 10 42

