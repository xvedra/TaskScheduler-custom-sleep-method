# Arduino TaskScheduler-custom-sleep-method
Use the SleepMethod to minimize the time between tasks and put the board to sleep.

To implement it, add or modify the following code in bool Scheduler::execute():
```

....

unsigned long tu, mtu;

....

mtu = 0xFFFFFFFF;


while (!iPaused && iCurrent) {


if(!iCurrent->canceled())

{

  tu = (long) iCurrent->iDelay - ( (long) ((_TASK_TIME_FUNCTION() - iCurrent->iPreviousMillis)) );
  
  if(tu < mtu) mtu = tu;
  
}    

....

//(*iSleepMethod)( tFinish-tStart );

(*iSleepMethod)(mtu);

....
```

Now, the board can sleep for all the time between tasks.

If your sleep method stops time like millis(), remember to use external_millis() in all software, including external libraries.
