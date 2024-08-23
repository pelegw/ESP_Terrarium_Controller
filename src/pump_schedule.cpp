// put function definitions here:
//todo - develop a dynamic schedule that works with an array of date, for now it's just a sieve.
#include <Arduino.h>
#include <SparkFunDS3234RTC.h>


//return 15 if schedule is hit.
//pump runs for now - every day at 8,12,16 and 20 hours. 12 run is for 30 seconds, all the rest are for 10.
//On friday at 12 run for 3 minutes

int check_pump_schedule()
{
  int hour_flag = 0;
  switch(rtc.hour()) {
    case 8:
      hour_flag=1;
      break;
    case 16:
      hour_flag=1;
      break;
    case 20:
      hour_flag=1;
      break;
    default:
      hour_flag = 0;
      break;
  }
//leave place to play with minutes further - right now it will set the flag when needs to run on the start of the minute - logic in the main loop will prevent the pump from restarting.
  if (hour_flag==1) {
    switch (rtc.minute()) {
      case 0:
        return 15;

      default:
        return 0;
    }
  }
//now deal with rest of the week vs friday long run
if (rtc.day()==6) {
  if ((rtc.hour()==12) && (rtc.minute()==0)) {
    return 120;
  }
  else {
    return 0;
  }
}
else {
  if ((rtc.hour()==12) && (rtc.minute()==0)) {
    return 15;
  }
  else {
    return 0;
    }
  }
return 0;
}