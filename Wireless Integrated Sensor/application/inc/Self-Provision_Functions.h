/*
 * Self-Provision_Functions.h
 *
 *  Created on: Jul 15, 2020
 *      Author: rquin
 */

#ifndef SELF_PROVISION_FUNCTIONS_H_
#define SELF_PROVISION_FUNCTIONS_H_

#define ONE_SECOND 					32768
#define TIMER_ID_RESTART            78

extern uint16_t Unicast;
extern char unicast_array[4];

void Self_Provision_Device(void);
#endif /* SELF_PROVISION_FUNCTIONS_H_ */