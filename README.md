# orwe521_impulsedisplay

Based on Arduino

Tool to display power and consumed electric work based on pulse output of a energymeter (like OR-WE-521)


# function

reacts on the positive impulse from a OR-WE-521 
an impulse is sent for each watt-hour that was measured, 
meaning 1000 imp sums up to 1 kW/h

impulse is expected on "SIGNAL-PIN"" signal has to been validated (stable over MIN-SIGNAL-PERIOD milisecs)
to exclude noise

next impulse the mean power can be calculated as 3600 / secs

## Accuracy

the display is actualized each time an impulse from energy meter is received.

as the signals occur seldomer if the power is low, the display shows not the correct actual value.

On 10W signal is send each 6minutes!



# requirements

get analogKbd and put it in the library directory of Arduino IDE

git clone https://github.com/benoitjoh/analogKbd 
