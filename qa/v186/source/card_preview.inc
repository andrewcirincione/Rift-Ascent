static void preview_once(const Game*g,int raw,int target,int choice,CardPreview*out){
 (void)choice;Game copy=*g;memset(out,0,sizeof(*out));int index=-1;
 for(int i=0;i<copy.hand_count;i++)if(copy.hand[i]==raw){index=i;break;}if(index<0)return;
 copy.screen=COMBAT;copy.energy=MAX(copy.energy,game_card_cost_now(&copy,raw));
 preview_trace=out;preview_depth=preview_phase=preview_nested=0;preview_choice=-1;
 game_play_card(&copy,index,target);
 preview_trace=0;preview_depth=preview_phase=preview_nested=0;preview_choice=-1;
}
void game_card_preview(const Game*g,int raw,int target,CardPreview*out){memset(out,0,sizeof(*out));if(g&&g->screen==COMBAT&&CARD_ID(raw)<CARD_COUNT)preview_once(g,raw,target,-1,out);}
void game_card_description_now(const Game*g,int raw,int target,char*out,int cap){
 (void)g;(void)target;if(out&&cap>0)snprintf(out,(size_t)cap,"%s",card_description(raw));
}
static int enemy_status_entries(const Game*g,int enemy,int wanted,char*out,int cap){
 if(!g||enemy<0||enemy>=g->enemy_count)return 0;
 const Enemy*e=&g->enemies[enemy];int n=0;
#define FOE_STATUS(value,format) do{int v=(value);if(v){if(n==wanted&&out&&cap>0)snprintf(out,(size_t)cap,format,v);n++;}}while(0)
 FOE_STATUS(e->strength,"Force: %d");
 FOE_STATUS(e->weak,"Falter: %d turns (-25%% damage)");
 FOE_STATUS(e->vulnerable,"Fracture: %d turns (+50%% taken)");
 FOE_STATUS(e->poison,"Wither: %d HP on next trigger");
 FOE_STATUS((*STRANGLE_SLOT(g,enemy)),"Strangle: lose %d HP per card this turn");
 FOE_STATUS((*MANGLE_SLOT(g,enemy)),"Mangle: restore %d FOR next turn");
#undef FOE_STATUS
 return n;
}
int game_enemy_status_count(const Game*g,int e){return enemy_status_entries(g,e,-1,0,0);}
void game_enemy_status_line(const Game*g,int e,int index,char*out,int cap){
 if(out&&cap>0)out[0]=0;enemy_status_entries(g,e,index,out,cap);
}
