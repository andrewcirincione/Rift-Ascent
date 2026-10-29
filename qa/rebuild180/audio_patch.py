from pathlib import Path
root=Path(__file__).parent/'source'
p=root/'psp_start.c';s=p.read_text().replace('#define RIFT_TRACK_COUNT 5','#define RIFT_TRACK_COUNT 10').replace('#define RIFT_MUSIC_FOOTER 56','#define RIFT_MUSIC_FOOTER 136')
s=s.replace('soundtrack_samples[RIFT_TRACK_COUNT];','soundtrack_samples[RIFT_TRACK_COUNT],soundtrack_rate[RIFT_TRACK_COUNT];').replace('audio_half=0,audio_byte=0','audio_byte=0').replace('static int audio_predictor=0,audio_step_idx=0,audio_last=0;','static int audio_predictor[2],audio_step_idx[2],audio_last[2];')
s=s.replace("{'R','I','F','T','M','3','1','1'}","{'R','I','F','T','A','U','1','8'}").replace('footer+48','footer+128').replace('footer+8+i*8','footer+8+i*12').replace('footer+12+i*8','footer+12+i*12')
s=s.replace('if(soundtrack_samples[i]==0||','soundtrack_rate[i]=read_u32(footer+16+i*12);\n   if((soundtrack_rate[i]!=22050&&soundtrack_rate[i]!=44100)||soundtrack_samples[i]==0||').replace('(soundtrack_samples[i]+1u)/2u>','soundtrack_samples[i]>').replace('Rift 1.3.11 five music tracks loaded','Rift 1.8 ten stereo music tracks loaded').replace('audio_pos=audio_length=audio_half=audio_byte=0;','audio_pos=audio_length=audio_byte=0;').replace('audio_predictor=audio_step_idx=audio_last=0;','for(int c=0;c<2;c++)audio_predictor[c]=audio_step_idx[c]=audio_last[c]=0;')
a=s.index('static int soundtrack_sample(void)');b=s.index('static short audio_clip',a)
s=s[:a]+'''static void soundtrack_sample(void){
 if(!soundtrack_active||soundtrack_current<0)return;
 if(!left_samples&&!soundtrack_select(soundtrack_current))return;
 if(audio_pos>=audio_length){
  unsigned want=left_samples<sizeof(audio_cache)?left_samples:sizeof(audio_cache);
  audio_length=sceIoRead(soundtrack_fd,audio_cache,want);audio_pos=0;
  if(audio_length<=0){soundtrack_close();audio_last[0]=audio_last[1]=0;return;}
 }
 unsigned byte=audio_cache[audio_pos++];
 for(int c=0;c<2;c++){
  int nibble=(byte>>(4*c))&15,step=adpcm_steps[audio_step_idx[c]],delta=step>>3;
  if(nibble&1)delta+=step>>2;if(nibble&2)delta+=step>>1;if(nibble&4)delta+=step;
  audio_predictor[c]+=(nibble&8)?-delta:delta;
  if(audio_predictor[c]>32767)audio_predictor[c]=32767;if(audio_predictor[c]<-32768)audio_predictor[c]=-32768;
  audio_step_idx[c]+=adpcm_index_delta[nibble];
  if(audio_step_idx[c]<0)audio_step_idx[c]=0;if(audio_step_idx[c]>88)audio_step_idx[c]=88;
  audio_last[c]=audio_predictor[c];
 }
 left_samples--;
}
''' +s[b:]
s=s.replace('if(audio_phase==0)audio_last=soundtrack_sample();','if(audio_phase==0)soundtrack_sample();').replace('audio_phase=(audio_phase+1u)&3u;','audio_phase=(audio_phase+1u)%(44100u/soundtrack_rate[track]);').replace('else if(!soundtrack_active)audio_last=0;','else if(!soundtrack_active)audio_last[0]=audio_last[1]=0;')
s=s.replace('int l=soundtrack_active&&!paused?audio_last*gain','int l=soundtrack_active&&!paused?audio_last[0]*gain').replace('int r=soundtrack_active&&!paused?audio_last*gain','int r=soundtrack_active&&!paused?audio_last[1]*gain');p.write_text(s)
p=root/'asset_stream.c';s=p.read_text().replace('footer[56]','footer[136]').replace('end<72','end<152').replace('end-56,footer,56','end-136,footer,136').replace('RIFTM311','RIFTAU18').replace('footer+48','footer+128').replace('end-72','end-152').replace('end-56-music','end-136-music');p.write_text(s)
