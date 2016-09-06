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

extern void 		ann_start_qlearning(int epochs, float gamma, float epsilon);

#endif /* ANN_H_ */
