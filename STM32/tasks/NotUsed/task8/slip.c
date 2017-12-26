#include "slip.h"
#include <stdio.h>
#include <stdlib.h>

/**************************************************************************
* 函数名称： slipPacket()
* 功能描述:  SLIP打包.
* 输入参数： 
* 输出参数： 
* 返 回 值： slip编码后的报文长度
* 其它说明： 
* 修改日期    版本号     修改人	     修改内容
* -----------------------------------------------
* 29/03/2010  V1.0	     tln          N/A
**************************************************************************/ 
int  slipPacket
(
unsigned char *srcbuf ,            /*源buf*/
unsigned char *destbuf,            /*slip打包后的buf*/
unsigned int srclen               /*slip 打包前的长度*/
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
* 函数名称： slipUnPacketPro()
* 功能描述:  SLIP解包.
* 输入参数： 
* 输出参数： 
* 返 回 值： 解slip帧长度
* 其它说明： 
* 修改日期    版本号     修改人	     修改内容
* -----------------------------------------------
* 29/03/2010  V1.0	     tln          N/A
**************************************************************************/ 
int  slipUnPacketPro
(
unsigned char *srcbuf ,            /*源buf*/
unsigned int srclen               /*slip 打包后的长度*/
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