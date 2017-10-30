I have a mutex lock semphores make sure the semaphores for customers and barbers decrement and increment properly. 
I have created 4 pthreads. 
3 phtreads the calculation of the barber 
and 1 pthread the calculation of the customers. 
Barber thread checks to see if there is a customer waiting or the time has run out.
The mutex locks the process of the barber.
if there is a barber avaliable
Then if barb is done cutting the customer will leave and barber will be
available.
Else no free barbers.
Customer thread checks for the time.
The mutex locks the process.  
There is a 1/2 chance of a customer arriving.
if the customers arrives.
If there are enough seat then costumers waiting is incremented. If there are
barberers available the barber with start cutting hair.
Else if there are no more seats then customer will leave make seats is 10.
ELse no customer will arrive.
