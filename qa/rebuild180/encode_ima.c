/* Two independent IMA states, one byte per stereo frame. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
static const int steps[89]={7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
static const int index_delta[8]={-1,-1,-1,-1,2,4,6,8};
int main(int argc,char**argv){if(argc!=3)return 2;FILE*in=fopen(argv[1],"rb"),*out=fopen(argv[2],"wb");if(!in||!out)return 3;
 int pred[2]={0},ix[2]={0};int16_t pcm[2];unsigned frames=0;double signal=0,error=0;
 while(fread(pcm,sizeof(pcm),1,in)==1){unsigned byte=0;
 for(int c=0;c<2;c++){int diff=pcm[c]-pred[c],code=diff<0?8:0;if(diff<0)diff=-diff;int st=steps[ix[c]],delta=st>>3;
 if(diff>=st){code|=4;diff-=st;delta+=st;}st>>=1;if(diff>=st){code|=2;diff-=st;delta+=st;}st>>=1;if(diff>=st){code|=1;delta+=st;}
 pred[c]+=(code&8)?-delta:delta;if(pred[c]>32767)pred[c]=32767;if(pred[c]<-32768)pred[c]=-32768;
 ix[c]+=index_delta[code&7];if(ix[c]<0)ix[c]=0;if(ix[c]>88)ix[c]=88;byte|=code<<(4*c);
 signal+=(double)pcm[c]*pcm[c];double e=pcm[c]-pred[c];error+=e*e;
 }fputc(byte,out);frames++;}
 if(ferror(in)||fclose(out))return 4;fclose(in);printf("%u %.3f\n",frames,10*log10(signal/error));return 0;}
