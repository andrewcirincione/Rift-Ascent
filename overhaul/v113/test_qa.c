#include "game.h"
#include "sound.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int first_bit(unsigned m) {for(int k=0;k<MAP_LANES;k++)if(m&(1u<<k))return k;return -1;}
int main(void){
  int cases=0;
  for(unsigned seed=1;seed<=1024;seed++){
    Game g;game_new(&g,seed*2654435761u);
    assert(game_route_visible_mask(&g,0)==7u);
    for(int r=0;r<MAP_ROWS;r++){
      assert(g.floor==r);
      unsigned visible=game_route_visible_mask(&g,r);
      assert(visible && (visible&~7u)==0u);
      for(int lane=0;lane<MAP_LANES;lane++)
         assert(!!(visible&(1u<<lane))==!!game_route_available(&g,lane));
      for(int rr=r;rr<MAP_ROWS;rr++)assert(game_route_visible_mask(&g,rr)!=0);
      int chosen=first_bit(visible);
      assert(chosen>=0);
      g.map_selected[r]=chosen;
      g.lane=chosen;g.floor++;
      cases++;
    }
  }
  /* Confirm fallback synth produces actual sound rather than a silent stream. */
  RiftSound snd;rift_sound_reset(&snd);
  short pcm[4096];memset(pcm,0,sizeof(pcm));
  rift_audio_generate(&snd,pcm,2048,0,0);
  int nonzero=0;for(int i=0;i<4096;i++)if(pcm[i])nonzero++;
  assert(nonzero>100);
  printf("PASS v1.1.3 QA: %d route transitions + nonzero fallback music (%d samples)\n",cases,nonzero);
  return 0;
}
