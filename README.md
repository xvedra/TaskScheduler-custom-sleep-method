# TaskScheduler-custom-sleep-method
Call SleepMethod with minimal time to next task. Use it to sleep.

In bool Scheduler::execute(), add:

....

unsigned long tu, mtu;

....

mtu = 0xFFFFFFFF;

....

while (!iPaused && iCurrent) {


if(!iCurrent->canceled())

{

  tu = (long) iCurrent->iDelay - ( (long) ((_TASK_TIME_FUNCTION() - iCurrent->iPreviousMillis)) );
  
  if(tu < mtu) mtu = tu;
  
}    

....

//(*iSleepMethod)( tFinish-tStart );

(*iSleepMethod)(mtu);


Now you can sleep for all time betwen tasks.

If your sleep method stop time like millis() remenber to use external_millis() in ALL software (in external libs too!)

