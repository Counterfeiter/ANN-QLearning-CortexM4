/*
 * ann.h
 *
 *  Created on: 22.08.2016
 *      Author: Basti
 */

#ifndef ANN_H_
#define ANN_H_

#ifndef FIXEDFANN
#include "floatfann.h"
#else
#include "fixedfann.h"
#endif

//set this define to train more complex led pattern
//undefine to choose just on led to blink
#define ANN_FEEDBACK_OUT_TO_IN

extern void 		ann_start_qlearning(int epochs, float gamma, float epsilon);

#endif /* ANN_H_ */
