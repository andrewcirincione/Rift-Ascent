/* History changes dialogue only. No new roll, save layout or cutscene page.
   Wins and durable retirement receipts gate recognition; merely creating or
   abandoning a live slot is not evidence of a completed prior journey. */
int game_memory_stage(const Game*g){
 if(!g||!g->ascent.id||g->revision184.custom)return 0;
 unsigned wins=ascent_account.wins[0]+ascent_account.wins[1]+ascent_account.wins[2];
 if(g->ascent.won&&wins)wins--; /* The first ending is not already a return. */
 unsigned completed=ascent_memory.count;
 if(g->ascent.slot<ASCENT_SLOTS&&ascent_memory.last[g->ascent.slot]==g->ascent.id&&completed)completed--;
 unsigned history=MAX(wins,completed);return !history?0:history==1?1:history<4?2:3;
}
const char*game_loop_story(const Game*g){
 int stage=game_memory_stage(g)-1;if(stage<0)return 0;
 int c=CLAMP(g->character,0,2),a=CLAMP(g->act-1,0,2),page=CLAMP(g->selection,0,2);
 static const char*entry[3][3]={
  {"The ash holds my bootprints. Same stride. Same broken heel. I have climbed these steps before, and the tower has kept the marks.",
   "That bell rang before I touched the door. I know the note. I heard it at the end of another life.",
   "The same correction appears beneath my signature. This is not a new ledger. Someone has restored the page I crossed out."},
  {"The scars return before the wounds. Whatever drags me back cannot quite remember how to make me forget.",
   "The choir has changed one word of its prayer. My name. It remembers more each time I come back.",
   "Every ascent leaves an omission the next record cannot explain. The reset is imperfect. I can learn where its seams are."},
  {"An oath, a name, a stolen testimony. Three anchors holding the same prison shut. This time I climb to find where mine was fastened.",
   "They erased my name to make a perfect anchor. But every return teaches someone to say it. The tower cannot silence all of us.",
   "It resets the record, not what we have learned. Every lesson carried back is another piece of its machinery coming loose."}
 };
 static const char*challenge[3][3]={
  {"I remember killing something behind this door.","You know my footsteps. Why?","We have held this audience before."},
  {"The tower keeps bringing me back. Tell me what it wants.","You remember every death. I can hear them in your voice.","Someone has made a mechanism out of our lives."},
  {"You are guarding an anchor, not a throne.","The prison remembers me. Now I remember it.","I know who keeps rewriting this record. Move."}
 };
 static const char*reply[3][3]={
  {"The furnace consumes the body. The oath survives. We burn you again because it does.","Ash keeps the shape of every offering. Even yours.","I keep the fire beneath the lock. Break me, and you still have a lock."},
  {"A sentence is not ended merely because the condemned has died.","The record is corrected. Your name returns to the first page.","I judge the lives the tower repeats. I did not write its law."},
  {"You have taken this crown before. You mistake a lock for its keeper.","Three lives hold the seal: an oath, a name, a stolen testimony. None were given willingly.","Kill the keeper. Climb to the maker. Until the anchors are freed, the rift will call you home."}
 };
 static const char*answer[3][3]={
  {"Then this time I take the answer with me.","Keep listening. I intend to leave an echo.","Then I shall remember what you refuse to say."},
  {"I was forged to hold a chain. I can learn to break one.","You can bury a name. You cannot make it stop belonging to me.","A machine can be understood. And anything understood can be unmade."},
  {"Next time, fewer chains.","We will remember each other. Start being afraid.","I will bring back more than a victory."}
 };
 if(game_story_kind(g)==2&&(g->act==3||g->scroll%2==0)){
  if(a==2&&page==1&&!ascent_secret(c))return "The crown remembers every rebellion. Yet here you stand, choosing yours again.";
  return page==0?challenge[stage][c]:page==1?reply[a][stage]:answer[stage][c];
 }
 if(game_story_kind(g)==1&&(g->act==1||g->scroll%3==0))return entry[stage][c];
 return 0;
}
const char*game_loop_event(const Game*g){
 int stage=game_memory_stage(g)-1;if(stage<0)return 0;
 int page=CLAMP(g->selection,0,2),c=CLAMP(g->character,0,2),id=g->expedition.story_id;
 if(g->screen==ABOMINATION_BEFORE&&page==1)return stage<2?
  "One of the voices in its wings speaks a word you heard before your last death. It has been listening through every return.":
  "The gods in its wings are memories of worlds the rift erased. The tower was built to cage their hunger. You were made its living hinges.";
 if(g->screen==ABOMINATION_AFTER&&page==2)return stage<2?
  "The wound above the tower remains. Beneath the silence, you hear the first bell of the journey. Something still knows the way back.":
  "The body is broken. The anchors remain. This time you carry the truth below: the next ascent must free the lives that hold the prison shut.";
 if(g->screen!=EXPEDITION_STORY||page!=1||!(id==2||id==4||id==8||id==11||id==14||id==18))return 0;
 static const char*lines[3][3]={
  {"For an instant, another scar lies beneath the one on your hand. The encounter feels remembered rather than discovered.",
   "A voice nearly calls you by a name you lost. It stops, as though warned. You know you have heard that hesitation before.",
   "A correction in the stone matches your handwriting. You remember making it here, though these hands never did."},
  {"An old inscription calls the Warden an anchor. Beneath it, in your own hand: I did not agree to this.",
   "The same prayer is scratched out twice. A third line survives: what the tower cannot erase, it must carry into the next life.",
   "The lights mark missing testimony. Beside each is a life spent restoring the same false account of what happened."},
  {"Beneath the marks of your weapon are two other signs: a nameless blade and an unsigned page. Other hands are working at the same chain.",
   "Someone has left three names beneath the prayer. Yours is intact. You add the mark of a broken chain beside it.",
   "Three paths cross in the carving. One witness cannot free the record alone. The oath and the nameless blade must remember too."}
 };
 return lines[stage][c];
}
const char*game_loop_ascent(const Game*g,int kind,int page){
 if(!game_memory_stage(g))return 0;
 int c=CLAMP(g->character,0,2);page=CLAMP(page,0,2);
 static const char*resolution[3]={
  "I have stood here before. Still, the gates are open and the living are free. That is a victory worth carrying home.",
  "I remember another silence after this victory. This one holds voices the choir could not erase. I will carry their names home.",
  "I have signed this ending before. Tonight the names remain legible. Whatever returns, those people have had one day of their own."
 };
 if(kind==0&&page==2)return resolution[c];
 if((kind==1||kind==2)&&!ascent_secret(c))return 0;
 static const char*revelation[3]={
  "The king was a lock, and I have broken it before. Something above the light keeps rebuilding the cage around my oath.",
  "The choir is dead again. The voice behind it remembers all our silences. This time I know where to listen.",
  "The names are vanishing in the order they returned. The court was only a copying desk. Its author is still revising the record."};
 static const char*unmaker[3][3]={
  {"UNMAKER: I bound your oath to the rift. Each death restores the seal. Each victory repairs what the climb has broken.",
   "WARDEN: You made a prison and called us its walls.",
   "UNMAKER: You were meant to return empty. Those memories you carry are the flaw."},
  {"UNMAKER: I erased your name to leave an empty place the rift could always fill. That is how you return.",
   "VESPER: And every time, someone learns to say it again.",
   "UNMAKER: A remembered name is an anchor coming loose. You were never supposed to remember one another."},
  {"UNMAKER: Your testimony certifies the seal. When the prison weakens, I restore the record and the witness who signed it.",
   "PROXY: Then every name I restore is a correction you cannot silence.",
   "UNMAKER: The thing above us devours worlds. I gave it three lives to repeat instead. Do you understand what your freedom may cost?"}
 };
 if(kind==1&&page==0)return revelation[c];
 if(kind==2)return unmaker[c][page];
 if(kind==3&&page==2)return "The maker falls. The rift still remembers its anchors. You descend with a truth it cannot take back: none of you has been climbing alone.";
 return 0;
}
