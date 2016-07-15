//#include <string.h>
#include "comfunc.h"
#include "basic_types.h"

uint8 checksum (uint8 *data,uint8 len)
{
	uint8 cs = 0;

	while(len-- > 0)
		cs += *data++;
	return(cs);
}

void mymemcpy(void *dst,void *src,uint8 len)
{
	while(len--)
	{
		*(char *)dst = *(char *)src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}
}
uint8 memcmp_my(const void *s1, const void *s2, uint8 n)
{
    while (n && *(char*)s1 == *(char*)s2) 
    {
        s1 = (char*)s1 + 1;
        s2 = (char*)s2 + 1;
        n--;
    }

    if(n > 0)
    {
        return(1);
    }
    return(0);
}

uint8 is_all_xx(const void *s1, uint8 value, uint8 n)
{
    while(n && *(char*)s1 == value)
    {
        s1 = (char*)s1 + 1;
        n--;
    }
    return(!n);
}


void memset_my(void *s1, uint8 value, uint8 n)
{
    do
    {
        *(char*)s1 = value;
        s1 = (char *)s1 + 1;
    }
    while(--n);
}
void* memmove_my(void* dest, void* source, uint16 n) 
{
    int8* ret = (int8*)dest; 

    if (ret <= (int8*)source || ret >= (int8*)source + n)
    { 
        while (n --)
        {    
           *ret++ = *(int8*)source;
           source = (int8 *)source + 1;
        }
    }
    else
    {
        ret += n - 1; 
        source = (int8*)source + n - 1; 
        while (n--)
        {
           *ret-- = *(int8*)source;
           source = (int8*)source -1;
        }
    } 
    return dest;
}

uint8 find_max(uint8 buf[], uint8 n)
{
    uint8 i;
    uint8 max_d;

    max_d = buf[0];
    for(i=0; i<n; i++)
    {
        if(max_d < buf[i]) max_d = buf[i];
    }
    return(max_d);
}

uint16 sp_crc16_with_init(uint16 crc, const uint8 *buf, uint8 size)
{
  unsigned char i;

  while(size--!=0)
  {
    for(i = 0x80; i != 0; i>>=1)
    {
      if((crc&0x8000) != 0) 
      {
          crc <<= 1; 
          crc ^= 0x1021;
      } /* 車角那?CRC3?辰?2?迄?車CRC */
      else 
      {
          crc <<= 1;
      }
      if(((*buf)&i)!=0) 
      {
          crc ^= 0x1021; /* ?迄?車谷?㊣???米?CRC */
      }
    }
    buf++;
  }
  return(crc);
}

uint8 bcd2bin(uint8 val)
{
    uint8 bin = 0; 
    while(val >= 0x10)
    {
        val -= 0x10;
        bin += 10;
    } 
    bin += val; 
    return(bin);
}
uint8 bin2bcd(uint8 val)
{
    uint8 bcd = 0 ;
    while(val >= 10)
    {
        val -= 10;
        bcd+= 0x10;
    } 
    bcd += val; 
    return(bcd);
}

uint8 sum (unsigned char *data,uint8 len)
{
		unsigned char cs = 0;
		while(len-- > 0)
		{
				cs += *data++;
		}
		return(cs);
}


void delay1ms(uint16 m)
{
		uint16 i;
    while(m--)
    {
        for( i = 0; i < 833 ; i++);
    }
}

void numeric2bcd(uint32 value,uint8 *bcd, uint8 bytes)
{
		uint8 x;
		if(bytes > 5)
		{	
				bytes = 5;
		}
		while(bytes--)
		{
				x = value % 100u;
				*bcd = ((x/10u)<<4) + (x%10u);
				bcd++;
			  value /= 100u;
		}
}
 
uint32 bcd2numeric(uint8 *bcd, uint8 bytes)
{ 
		uint32 ret = 0;
		if(bytes > 4)
		{	
				bytes = 4;
		}
		while (bytes-- > 0)
		{
				ret *= 100u; 
				ret += bcd2bin(bcd[bytes]);
		} 
		return ret;
}

void advance_a_second(unsigned char bin_time[])
{
  const unsigned char day_of_month[2][12]=
	{
		{31,29,31,30,31,30,31,31,30,31,30,31},
		{31,28,31,30,31,30,31,31,30,31,30,31}
	}; 
  bin_time[0]++;
	if(bin_time[0] < 60)
	  goto bin_time_adjust_exit;
	
	bin_time[0] -= 60;
	bin_time[1]++;
	if(bin_time[1] < 60)
	  goto bin_time_adjust_exit;

	bin_time[1] -= 60;
	bin_time[2]++;
	if(bin_time[2] < 24)
	  goto bin_time_adjust_exit;

	bin_time[2] -=24;
	bin_time[3]++;
	if(bin_time[3] <= day_of_month[0x00 !=(bin_time[5] & 0x03)][bin_time[4]-1])
	  goto bin_time_adjust_exit;

	bin_time[3] = 1;
	bin_time[4] ++;
	if(bin_time[4] <= 12)
	  goto bin_time_adjust_exit;

	bin_time[4] = 1;
	bin_time[5]++;

bin_time_adjust_exit: 
   return;
}
static const unsigned short days[13]=
{
 0,
 31,
 31+28,
 31+28+31,
 31+28+31+30,
 31+28+31+30+31,
 31+28+31+30+31+30,
 31+28+31+30+31+30+31,
 31+28+31+30+31+30+31+31,
 31+28+31+30+31+30+31+31+30,
 31+28+31+30+31+30+31+31+30+31,
 31+28+31+30+31+30+31+31+30+31+30,
 31+28+31+30+31+30+31+31+30+31+30+31,
};
unsigned int my_mktime(unsigned int year, unsigned int mon,
	unsigned int day, unsigned int hour,
	unsigned int min, unsigned int sec)
{
     unsigned int  result ;

     if( mon < 0x01) mon = 0x01;
     if( day < 0x01) day = 0x01;

     while(mon > 12 )
     {
       mon-= 12;
       year++;        
     }
     result =1+ year/4 - year/100 + year/400 ;

     if((0x00 ==(year % 0x04))&&( mon < 0x03))
        result --;
     result += days[mon-1];
	result  = (((result + day -1 +  year*365
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
        return(result);
}
int32 get_bin_time_second(unsigned char bintime[])
{ 
		return(my_mktime(bintime[5],bintime[4],bintime[3],bintime[2],bintime[1],bintime[0]));
}
void my_gmtime(unsigned int s,unsigned char t[])
{
  unsigned short dayofyear;
  unsigned short dayofmonth;

  int i;
     t[0] = s % 60;
     s /= 60;
     t[1] = s % 60;
     s /= 60;
     t[2] = s % 24;
     s /= 24;

     t[5] = 0x00;
     while(1)
     {
        if(  t[5] % 0x04 ) dayofyear = 365;
        else  dayofyear = 366; 
        if( s >= dayofyear)
        {
          s -= dayofyear;
          t[5]++;
        }
        else
          break;
     }
     for( i = 11 ; i >= 0x00 ; i--)
     { 
        dayofmonth =days[i];
        if((0x00 ==( t[5] % 0x04))&& (i >=2 )) dayofmonth++; 
        if(s >= dayofmonth)
        {
          t[4] = i+1;
          s -= dayofmonth;
          t[3]= s+1;
          break;
        }

     }     
}
#if 0
int gnu_difftime( unsigned char t1[], unsigned char t2[]) {
	time_t mt1,mt2;
	struct tm tm1,tm2;
	tm1.tm_sec = t1[0];
	tm1.tm_min = t1[1];
	tm1.tm_hour = t1[2];
	tm1.tm_mday = t1[3];
	tm1.tm_mon = t1[4] -1;
	tm1.tm_year = t1[5]+100;
	tm1.tm_isdst = 0x00;
	tm1.tm_gmtoff = 0x00;

	tm2.tm_sec = t2[0];
	tm2.tm_min = t2[1];
	tm2.tm_hour = t2[2];
	tm2.tm_mday = t2[3];
	tm2.tm_mon = t2[4] -1;
	tm2.tm_year = t2[5]+100;
	tm2.tm_isdst = 0x00;
	tm2.tm_gmtoff = 0x00;

	mt1 = mktime(&tm1);
	mt2 = mktime(&tm2);
	return(mt1 - mt2);

}
#else
int gnu_difftime( unsigned char t1[], unsigned char t2[]) 
{
	unsigned int mt1,mt2;

	mt1 = get_bin_time_second(t1);
	mt2 = get_bin_time_second(t2);
	return(mt1 - mt2);
}
#endif

#if 0
void adjust_bin_time(unsigned char bintime[]) {
	unsigned char ts[6],d=0x00,m,y;
	int i;
	for ( i = 0 ; i< 6 ; i++ )
		ts[i] = t[i];
	while ( ts[0] >= 60 ) {
		ts[1]++;
		ts[0] -=60;
	}
	while ( ts[1] >= 60 ) {
		ts[2]++;
		ts[1] -=60;
	}
	while ( ts[2] >= 24 ) {
		ts[3]++;
		ts[2] -= 24;
	}

	while ( ts[4] >12 ) {
		ts[4] -=12;
		ts[5]++;
	}

	m = ts[4];

	y = ts[5] % 4;		  //leap
	if ( 0 == y ) y =1;
	else y =0;

	switch ( m ) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		d =31;
		break;

	case 4:
	case 6:
	case 9:
	case 11:
		d = 30; break;
	case 2:
		d = 28+y; break;
	default:
		break;

	}
	if ( ts[3] > d ) {
		ts[4]++;
		ts[3] -= d;
	}
	while ( ts[4] >12 ) {
		ts[4] -=12;
		ts[5]++;
	}

	for ( i = 0 ; i < 6 ; i++ )
		t[i] = ts[ i];
}
#else
void adjust_bin_time(unsigned char bintime[]) 
{ 
	unsigned int s;
	s = get_bin_time_second(bintime); 
	my_gmtime(s,bintime);
}
#endif
unsigned char week(unsigned char y, unsigned char m, unsigned char d)
{
 
const unsigned char TAB_X[12]={6,2,2,5,0,3,5,1,4,6,2,4};
 if((m<3)&&((y&0x03)==0))
 {
      return((y+(y>>2)+TAB_X[m-1]+d-1) % 7);
 }
 else
     return((y+(y>>2)+TAB_X[m-1]+d ) % 7);
}
uint8 compare_time_n(uint8 t1[],uint8 t2[],uint8 n)
{
    uint8 i;

    for(i = 0; i < n; i++)
    {
        if(t1[i] > t2[i]) return(1);
        else if(t1[i] < t2[i]) return(0);
    }
    return(0);
}

uint8 check_BCD_format(uint8 buf[], uint8 cnt, uint8 limit_min, uint8 limit_max)
{
	uint8 i = 0;

	for (i = 0; i < cnt; i++)
	{
        if (((buf[i] & 0x0F) > 0x09) || ((buf[i] & 0xF0) > 0x90) || (buf[i] > limit_max) || (buf[i] < limit_min))
        {
            return(FALSE);
        }
	}
	return(TRUE);
}


#if 0
uint16 reverse_data(uint16 data)
{
    uint8 tmp[2];
    word_to_little_bytes(data, tmp);
    return(big_bytes_to_word(tmp));
}
#endif

void	mem_reverse(uint8 *src, uint8 n)
{
	uint8 temp;
	uint8 i;
	uint8 *p;
	p	= src + n - 1;
	i	= n / 2;
	while(i--)
	{
		n--;
		temp	= *src;
		*src++	= *p;
		*p--	= temp;
	}
}

#if 0
struct RXTX_QUEUE {
    uint8 cnt;
    uint8 queue;
};
static struct RXTX_QUEUE rxtx_queue;

void push_rxtx_status(enum RXTX_STATUS status)
{
    if (rxtx_queue.cnt < 0x04)
    {
        rxtx_queue.queue <<= 2;
        rxtx_queue.queue |= status;
        rxtx_queue.cnt++;
    }
}

uint8 get_rxtx_status(void)
{
    /* queue empty */
    if (0x00 == rxtx_queue.cnt) return(NO_STATUS);

    rxtx_queue.cnt--;
    return((rxtx_queue.queue >> (rxtx_queue.cnt<<1)) & 0x03);
}
#endif
