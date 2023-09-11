# VRGloveController
## Purpose
The microcontroller is programmed to allow sending the glove sensors signals to the Oculus VR headset via Bluetooth protocol, hence allowing the user to interact with the games developed in Unity
## Hardware components per glove
Adafruit Feather board behaves like a bluetooth keyboard: 1

Flex sensors running along the glove fingers: 5

Force sensors on the thumbs and on the palms: 2
## Signal mapping
Each flex sensor signal is represeneted by a unique letter. 

Force sensor signal has 4 strength levels, each level is represented by a unique letter.

Letters are sent to Unity.

Meaning of letters:

- flex sensor
  
&ensp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;q - pinky
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;w - ring
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;e - middle
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;r - index
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;t - thumb
* thumb force
  
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;a - 0
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;s - 1
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;d - 2
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;f - 3
* palm force

&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;g - 0
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;h - 1
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;j - 2
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;k - 3
* other
  
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;z - calibration button pressed
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;x - next calibration scene

Data packet used to transmit these letters is Human Interface Device (HID) packet.
Since each packet cannot contain more than 6 keys, I used (3) HID packets in total.

class: hid_keyboard_report_t
	
Below is the summarized traceability of packet - unique letter - microntroller pin and meaning:

	packet 1	key	inputKeycodes		Pin		Meaning
	[0] ------------ z ---------[0]----------------- 9  ----------- calibration button pressed
	[1] ------------ q ---------[1]----------------- A5 ----------- pinky
	[2] ------------ w ---------[2]----------------- 4  ----------- ring
	[3] ------------ e ---------[3]----------------- 3  ----------- middlet
	[4] ------------ r ---------[4]----------------- 2  ----------- index
	[5] ------------ t ---------[5]----------------- 1  ----------- thumb
	packet 2
	[0] ------------ a ---------[6]----------------- 0  ----------- thumb force level 0
	[1] ------------ s ---------[7]----------------- 0  ----------- thumb force level 1
	[2] ------------ d ---------[8]----------------- 0  ----------- thumb force level 2
	[3] ------------ f ---------[9]----------------- 0  ----------- thumb force level 3
	[4] ------------ x ---------[14]----------------    ----------- next in calibration scene
	[5] --------- nothing ------[ ]-----------------    ----------- 
	packet 3
	[0] ------------ g ---------[10]---------------- A7 ----------- thumb force level 0
	[1] ------------ h ---------[11]---------------- A7 ----------- thumb force level 1
	[2] ------------ j ---------[12]---------------- A7 ----------- thumb force level 2
	[3] ------------ k ---------[13]---------------- A7 ----------- thumb force level 3
	[4] --------- nothing ------[ ]-----------------    ----------- 
	[5] --------- nothing ------[ ]-----------------    ----------- 


## Main functions in the code
There are 4 void functions:

	sendKeycode(...)
		This function sends letter 'z' or letter 'x' 
		It sends only 1 letter each time it is called

	sendKeycodeFlex(...)
		This function sends letters that each represents a finger flex sensor
		If the flex sensor bent passes the threshold, the corresponding letter is sent once
		If the previously bent flex sensor is reversed to relax state, the same letter is sent once
		The flex sensor reading is highest in its relax state, lowest in its max bent state
		Threshold: 
			  for index: 	>= 80% max bent
			  for others: 	>= 70% max bent
		Example output:	
			if index flex sensor reading:
					             exceeds 80% index max bent, send letter 'r' once
			                             still >= 80% index max bent, nothing happens
						     becomes < 80% index max bent, send letter 'r' once
							 
	sendKeycodeForce(...)
		This function sends letters that each represents a force sensor strength level
		If the force sensor compression passes the threshold, the corresponding letter is sent once
		If the previously compressed forcesensor is reversed to relax state, the same letter is sent once
		The force sensor reading is highest in its relax state, lowest in its max compressed state
		The force sensor reading = 1023 - raw reading
		Threshold: 
			  level 3:	>= 90% max compression
			  level 2:	>= 80% max compression
			  level 1:	>= 70% max compression
		Example output:	
			if thumb force sensor reading:
					             exceeds 90% max compression, send letter 'f' once
			                             still >= 90% index max bent, nothing happens
						     becomes within 80 - 90% max compression, send letter 'd' once
						     becomes < 70% max compression, send letter 'a' once
						     still < 70% max compression, nothing happens
						     
	Calibration()
		This function is called when the calibration button is pushed
		It logs the readings of flex and force sensors in their relax and max state
		It sends letter 'z' once to signal Unity that Calibration process is initiated
		It sends letter 'x' once to signal Unity to switch to the next scene

	
	
