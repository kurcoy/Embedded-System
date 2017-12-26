#include "slip.h"
#include <stdio.h>
#include <stdlib.h>

/**************************************************************************
* �������ƣ� slipPacket()
* ��������:  SLIP���.
* ��������� 
* ��������� 
* �� �� ֵ�� slip�����ı��ĳ���
* ����˵���� 
* �޸�����    �汾��     �޸���	     �޸�����
* -----------------------------------------------
* 29/03/2010  V1.0	     tln          N/A
**************************************************************************/ 
int  slipPacket
(
unsigned char *srcbuf ,            /*Դbuf*/
unsigned char *destbuf,            /*slip������buf*/
unsigned int srclen               /*slip ���ǰ�ĳ���*/
)
{
    unsigned int  i,j;
    j=0;
    i=1;

    destbuf[0]=END;
	
    if (srclen==0)
    {
       destbuf[1]=END; 
	   return 2;
    }
    while(srclen--) 
    {   
        switch(srcbuf[j]) 
        {        
            case END:
        
                destbuf[i++]=ESC;
                destbuf[i++]=ESC_END;
                break;        
        
            case ESC:
        
                destbuf[i++]=ESC;
                destbuf[i++]=ESC_ESC;
                break;        
        
            default:
        
                destbuf[i++]=srcbuf[j];
        }
        
        j++;
    }

    destbuf[i++]=END;             
    return i;
}



/**************************************************************************
* �������ƣ� slipUnPacketPro()
* ��������:  SLIP���.
* ��������� 
* ��������� 
* �� �� ֵ�� ��slip֡����
* ����˵���� 
* �޸�����    �汾��     �޸���	     �޸�����
* -----------------------------------------------
* 29/03/2010  V1.0	     tln          N/A
**************************************************************************/ 
int  slipUnPacketPro
(
unsigned char *srcbuf ,            /*Դbuf*/
unsigned int srclen               /*slip �����ĳ���*/
)
{
    unsigned int  i = 0;
    unsigned int  j =0;

	if(srclen==0)
	{
		printf("slipUnPacketPro : the packet is empty.\n");
		return(0);/*ERROR->0 tln 10.3.29*/
	}
	while(i<srclen)
    {
    	if(srcbuf[i]==ESC)
    	{
    		i++;
			if(srcbuf[i]==ESC_END)
			{
				srcbuf[j++]=END;
				i++;
			}
			else if(srcbuf[i]==ESC_ESC)
			{
				srcbuf[j++]=ESC;
				i++;
			}
			else
			{
				printf("slipUnPacket : data is error.\n");
				return(0);/*ERROR->0 tln 10.3.29*/
			}
    	}
		else
		{
			srcbuf[j++]=srcbuf[i++];
		}
			
    }
	return(j);
}