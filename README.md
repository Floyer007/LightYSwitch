# LightYSwitch
This is the source-code for the LightYSwitch (LYS).<br />
The key-feature is a simple design and a sensor-response to get the real switch-state.

## Usage
Take a look at the hardware-chapter and the functions-chapter.

Install the WeMos D1-Mini library for your Arduino-Workspace.<br />
[I hope this one is the correct tutorial: https://wiki.wemos.cc/tutorials:get_started:get_started_in_arduino]

To adapt it, change the Pin-definitions and most important the "ssid" and "password" for your WiFi-Network.

## Hardware
If you don't like to change the code:<br />
A button should be connected between GND and Pin 14 (switchPin).<br />
A reed-contact or any other detection-switch schould be connectet between GND and Pin 5 (detectPin).<br />
The servo-motor should be connected to a PWM-compatible Pin. I'd choose Pin 4  (servoPin).<br />
There is already an LED on the WeMos-Board. Therefore we don't have to connect anything. Pin 2 (ledPin).

## Functions

- Move the servo to it off-position (only if on before): http://IP_ADRESS/0
- Move the servo to it on-position (only if off before): http://IP_ADRESS/1
- Get the switch-state: http://IP_ADRESS/stat
- Check if the button was pressed before: http://IP_ADRESS/button
- Reset the button-state: http://IP_ADRESS/b_reset
- The servo moves to a center-position after each movement.
- The servo is software-detached after each action to minimize noisy sounds.

## Future changes
The system is currently based on a server, which is polling the LYS based on a cronjob.<br />
To trigger a event on the server with the button (in my case disabling all lights) I should use a service between LYS and the server to make it realtime capable.<br />
If there is someone who like to do it, feel free to make a pull-request.

The code could be cleaned up, because there are many double codelines.
