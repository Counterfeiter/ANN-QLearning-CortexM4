/*
 * ann.c
 *
 *  Created on: 22.08.2016
 *      Author: Basti
 */

#include "stm32f4xx_hal.h"
#include "ann.h"

static void ann_setLEDs(int index);
static void ann_getLEDs(fann_type *val);
static void ann_displayVector(fann_type *val);
static void ann_getLEDsWithOld(fann_type *val);


//use fann lib and qlerning to train a ann to set desired led pattern
//source is ported from here -> http://outlace.com/Reinforcement-Learning-Part-3/
void ann_start_qlearning(int epochs, float gamma, float epsilon)
{
	int num_inputs = 8;
	int num_outputs = 4;
	//create ann -> 1 hidden layer with 4 neurons
	struct fann *ann = fann_create_standard(3, num_inputs, 12, num_outputs);

	//could also be another alg.
	fann_set_training_algorithm(ann, FANN_TRAIN_INCREMENTAL);

	//stepwise is more then two times faster
	//use symmetric to deal with -1.0 and 1.0 or normal for 0.0 to 1.0
	fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC_STEPWISE);
	fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC_STEPWISE);

	//register two buffers and two pointers, to swap the buffers fast
	fann_type new_inputs[num_inputs];
	fann_type old_inputs[num_inputs];
	fann_type *new_in_p = new_inputs;
	fann_type *old_in_p = old_inputs;

	//set the LEDs the first time
	ann_setLEDs(0);
	//fill the buffer with the LED states
	ann_getLEDsWithOld(old_in_p); //fill array;

	for(int i = 0; i < epochs; i++ )
	{
		int action;
		//run ann network
		fann_type *qval_p = fann_run(ann, old_in_p);

		//copy output data -> later use
		//because qval_p is just a pointer no allocated mem
		fann_type qval[num_outputs];
		for(int x = 0; x < num_outputs; x++)
		{
			qval[x] = qval_p[x];
		}

		//if epsilon is high, we use random actions
		//we should make all possible actions to train this
		if((float)rand() / RAND_MAX < epsilon)
		{
			action = rand() % num_outputs;
			printf("Use random action\n");
		} else {
			//search the maximum in the output array
			fann_type max = -infinityf();
			action = 0;
			for(int x = 0; x < num_outputs; x++)
			{
				//find max value and save the index
				if(qval_p[x] > max)
				{
					max = qval_p[x];
					action = x;
				}
			}

			//some debug output
			printf("Qval LED: ");
			ann_displayVector(qval);
			printf("MaxQ: %f Index: %d\n", max, action);

			printf("Use a winner action\n");
		}

		//do the action that the ann predict...
		//let light up a LED (only one, but could be the same)
		ann_setLEDs(action);

		//waiting for user reward!
		fann_type reward = 0.0;
		printf("Reinforce now!\n");
		while(1)
		{
			//discovery button... -> action is like desired
			if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin))
			{
				reward = 1.0;
				break;
			}

			//new soldered button... -> action is "wrong"
			if(!HAL_GPIO_ReadPin(B2_GPIO_Port, B2_Pin))
			{
				reward = -1.0;
				break;
			}

			//no debounce here...
			// -> ann_setLEDs has some delay... its enough
		}
		//display reward decision
		printf("reward: %f\n", reward);

		//fill array with new state
		ann_getLEDsWithOld(new_in_p);
		//debug output
		printf("Read LED: ");
		ann_displayVector(new_in_p);

		//run ann with new inputs
		fann_type *newQ = fann_run(ann, new_in_p);

		//search the maximum again
		fann_type maxQ = -infinityf();
		for(int x = 0; x < num_outputs; x++)
		{
			maxQ = (newQ[x] > maxQ) ? newQ[x] : maxQ;
		}
		//debug outputs
		printf("NewQ: ");
		ann_displayVector(newQ);
		printf("MaxNewQ: %f\n", maxQ);

		//set the old values as train data (output) - use new max in equation
		qval[action] = (reward < 0.0) ? (reward + (gamma * maxQ)) : reward;

		//create new training data
		struct fann_train_data *train = fann_create_train_array(1, num_inputs, old_in_p, num_outputs, qval);
		//train ann
		fann_train_epoch(ann, train);
		//destroy training data -> could be more efficient implemented
		fann_destroy_train(train);

		//switch pointer -> new data, to old data
		fann_type *temp = old_in_p;
		old_in_p = new_in_p;
		new_in_p = temp;

		//Decrease epsilon
		if(epsilon > 0.1)
			epsilon -= ( 1.0 / epochs );
	}

	//display pattern after training
	//execute forever
	while(1)
	{
		//get position
		ann_getLEDs(new_in_p);
		//execute ann
		fann_type *exec_out = fann_run(ann, new_in_p);

		//search the maximum in the output array
		fann_type max = -infinityf();
		int index = 0;
		for(int x = 0; x < num_outputs; x++)
		{

			if(exec_out[x] > max)
			{
				max = exec_out[x];
				index = x;
			}
		}
		//set next LED
		ann_setLEDs(index);
		//delay a bit
		HAL_Delay(500);
	}
}

//use for some debug output
static void ann_displayVector(fann_type *val)
{
	for(int i = 0; i < 4 ; i++ )
	{
		printf("%f   ", val[i]);
	}
	printf("\n");
}

//get LED states to float vector
static void ann_getLEDs(fann_type *val)
{
	val[0] = HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? 1.0 : -1.0;
	val[1] = HAL_GPIO_ReadPin(LD4_GPIO_Port, LD4_Pin) ? 1.0 : -1.0;
	val[2] = HAL_GPIO_ReadPin(LD5_GPIO_Port, LD5_Pin) ? 1.0 : -1.0;
	val[3] = HAL_GPIO_ReadPin(LD6_GPIO_Port, LD6_Pin) ? 1.0 : -1.0;
}

//get LED states and the states before to a float vector
static fann_type old_led_state[4];
static void ann_getLEDsWithOld(fann_type *val)
{
	val[0] = HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? 1.0 : -1.0;
	val[1] = HAL_GPIO_ReadPin(LD4_GPIO_Port, LD4_Pin) ? 1.0 : -1.0;
	val[2] = HAL_GPIO_ReadPin(LD5_GPIO_Port, LD5_Pin) ? 1.0 : -1.0;
	val[3] = HAL_GPIO_ReadPin(LD6_GPIO_Port, LD6_Pin) ? 1.0 : -1.0;
	val[4] = old_led_state[0];
	val[5] = old_led_state[1];
	val[6] = old_led_state[2];
	val[7] = old_led_state[3];
}

//set LEDs and save state before
static void ann_setLEDs(int index)
{
	//save old state internal!
	ann_getLEDs(old_led_state);

	//delete all LEDs
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 0);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, 0);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, 0);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, 0);

	//to switch all LEDs short time off -> if the same LED lights up
	HAL_Delay(500);

	switch(index)
	{
		case 0:
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1);
			break;
		case 1:
			HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, 1);
			break;
		case 2:
			HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, 1);
			break;
		case 3:
			HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, 1);
			break;
		default:
			printf("Action error!\n");
			break;
	}
}
