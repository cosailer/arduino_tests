#include <avr/sleep.h>

void setup ()

{

set_sleep_mode (SLEEP_MODE_PWR_DOWN);

sleep_enable();

sleep_cpu ();

}  // end of setup

void loop () { }
