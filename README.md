Artificial Neural Network and reinforcement learning (Q-Learning) Blinky or Hello World C-Project
==============

This is an easy example for the use of fann as ANN lib and an Q-Learning algorithm to train a network 

Hardware
--------------
STM32F4 Discovery Board -> CortexM4 with FPU

solder a extra button from GND to PC6!

License
--------------
GPLv2

Libs
--------------
fann (LGPL see http://leenissen.dk/) (added as static library to this project)

Software
--------------
System Workbench (Eclipse based)
STM CubeMX

Introduction
--------------
Press the blue Discovery button if you wan't to train the state change. Press the new button to "untrain" this action.
Do this a 100 times and the pattern will be displayed after training.