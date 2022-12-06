/*
 * gsLinkPktQ.c
 *      Author: BINDUSHREE M V
 */
/*-----------------------------------------------------------------------------------------------------------------------------*/
#include "application/inc/gsLink/gsLinkPktQ.h"
#include "application/inc/gslink/GS_Link.h"
/*-----------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------------------------------------*/
void fn_enQ(struct_gsLinkPktQ *Q, struct_gsLinkPkt *pkt)
{
	if(Q->head == -1)
	{
		Q->head = 0;
	}
	else
	{
		if((Q->num_pkts == MAX_GSLINK_PKTS) &&
				(((Q->tail+1) % MAX_GSLINK_PKTS) == Q->head))
		{
			Q->head = (Q->head+1) % MAX_GSLINK_PKTS;
		}
	}
	Q->num_pkts = ( Q->num_pkts < ( MAX_GSLINK_PKTS) ) ? Q->num_pkts+1 : Q->num_pkts;
	Q->tail = (Q->tail+1) % MAX_GSLINK_PKTS;
	Q->gsLinkPkt[Q->tail] = *pkt;
	return ;
}
/*-----------------------------------------------------------------------------------------------------------------------------*/
void fn_deQ(struct_gsLinkPktQ *Q)
{
	if(!fn_isQempty(Q))
	{
		memset(&Q->gsLinkPkt[Q->head],'\0',sizeof(struct_gsLinkPkt));

		if(Q->head == Q->tail)
		{
			Q->head = -1;
			Q->tail = -1;
		}
		else
		{
			Q->head = (Q->head+1) % MAX_GSLINK_PKTS;
		}
		Q->num_pkts = ( Q->num_pkts > 0 ) ? Q->num_pkts-1 : Q->num_pkts;
	}
	return ;
}
/*-----------------------------------------------------------------------------------------------------------------------------*/
uint8_t fn_isQempty(struct_gsLinkPktQ *Q)
{
	 if(Q->num_pkts)
	 {
		 return 0;
	 }
	 return 1;
}
/*-----------------------------------------------------------------------------------------------------------------------------*/
void fn_peekQ(struct_gsLinkPktQ *Q, struct_gsLinkPkt *pkt)
{
	if(!fn_isQempty(Q))
	{
		*pkt = Q->gsLinkPkt[Q->head];
	}
	return ;
}
/*-----------------------------------------------------------------------------------------------------------------------------*/
