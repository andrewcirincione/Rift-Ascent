"""Authored dialogue; compile to immutable C strings without consuming combat RNG."""
from pathlib import Path
import json
q=Path(__file__).parent
entries={
'Warden':'''I kept this gate when the bells commanded it. The bells are silent now. I hear those behind it still.
They named the furnace mercy, and gave me the key. Let the hand that signed that order answer mine.
The iron remembers my gauntlet. Would that the dead remembered aught kinder.
I broke the key too late. I shall not break the oath I made after.
Here stood my watch. Here ended every road I was sworn to guard. Tonight, one road shall open.
A child passed these bars beneath a cloak of white. I looked away. It was my first honest act.
The ash falls upward. Even the dead must climb to be heard in this accursed place.
My sword was given for the keeping of peace. Peace hath proved a locked door and a mouthful of cinders.
They taught us that the gate must hold. None taught us what a keeper should do when the fire was within.
I have brought no prayer for these ruins. Only the key, and the strength to turn it against its maker.
The warrants came from these blue halls. Rain may cleanse their marble. It shall not cleanse their names.
Each star above this court was bought with a light extinguished below. I would see the reckoning.
My armor rusts in their holy water. Good. Let this place learn that iron can change.
I once took a seal for truth. Now I shall ask the man behind the seal to stand before my blade.
They sent a king of borrowed stars into the pit. Even their chosen are fuel when they cease to kneel.
These stairs were raised above the smoke. How easy it must have been to mistake that distance for innocence.
The river carries tablets of sentence. Not one bears the face of the hand that carried it out.
No chain is forged in one fire. I have found the second furnace, though they call this one a court.
I can hear the lower bells beneath this water. Their law hath not drowned them deeply enough.
Let the learned count my sins. I have counted the doors they ordered shut.
White stone. Golden doors. Not a grain of ash admitted. I shall bring them what they buried beneath.
I sought a voice worth obeying. At the summit I hear only an order, repeating after its speaker is gone.
The light hurts less than the memory of believing in it. Keep thy heaven. Give me the gate.
These saints have no eyes. At last, an honest likeness of those who watched us burn.
If pardon waits behind that door, let it wait. I have not climbed so far to be forgiven by a king.
My watch ends where the first command began. Then the road below shall belong to those who walk it.
The child's ribbon was white before the smoke. I wonder whether these halls remember that colour without gold.
I was promised that a faithful hand would enter heaven. Behold what faith hath made of mine.
No crown is so high that its shadow cannot be followed home.
The sword grows heavy. Good. Let the last blow carry all that the first oath did not.
Another stair. Another gate. I shall not ask whether this one deserves to open.
The throne is broken, yet the command survives. There is more than one way to keep a prison.
I have no bell to mark the end of watch. Until the road is clear, I remain.
These walls wear the shape of places I have buried. The rift is a poor keeper of the dead.
If the ascent hath no end, neither shall the turning of this key.
A thousand empty heavens cannot make one locked door just.
The ash knows the way upward. I follow it, as I should have followed the living.
My oath is mine now. No king shall speak through it again.
Beyond each broken crown, another silence. I shall keep walking until silence means peace.
Let the rift raise its gates. I have learned at last which side of them I serve.'''.splitlines(),
'Vesper':'''Below the choir, every breath hath weight. These are the voices we were taught to call an echo.
They sent me down in white, that no stain might be seen from heaven. The ash hath been more honest.
The child asked whether angels needed knives. I had no answer. I have carried the question ever since.
A keeper turned his face from the bars. One small mercy, and the whole order began to unravel.
I cut the star-seal before I cut the rope. That was the first command I ever truly broke.
No hymn reaches this far. Yet I have heard more prayer in these ruins than in all the gilded choir.
My cloak was woven to make a death look holy. Let it serve the living once.
The furnace took the names after we took the witnesses. I know this work. I shall know its ending.
I was told the lower wards had no innocents. Strange, then, that I was forbidden to look upon their faces.
Keep thy wings, distant saints. A narrow ledge and a steady hand will bear me higher.
Here the choir's whispers became law. Here a red seal weighed more than a child's breath.
I know these ledgers. Their blank spaces were my work. Tonight I shall write in a different hand.
A borrowed king looked upon a broken seal and hesitated. Perhaps I was not the only knife that turned.
The blue court taught us that the stars see all. Then why were we ordered to leave no witness?
Water hides the footsteps poorly. I prefer a world that admits where someone hath passed.
Each order bore a star, each star a chain. I have brought a blade fine enough for either.
They called my mercy a flaw in the instrument. Let them discover what else a flawed instrument may do.
The drowned bells still count the guilty. They have not yet learned to count their own keepers.
I need no shadow in this court. They have spent an age teaching themselves not to see.
A seal can be broken without breaking the hand beneath it. I wish I had learned that sooner.
Home. How small a word for so vast a wound.
The choir will know my tread. I was raised to cross these stones without a sound; now I mean to be heard.
They shall call this cloak defiled. I remember when it was clean, and what I did while wearing it.
Every blind saint faces the throne. Not one faces the road by which the children came.
I have returned the knife they gave me. They may judge the manner of its return for themselves.
The first lesson was silence. The last shall be refusal.
I once knelt where that light falls. The stone remembers my knees. It shall remember my feet.
No confession. No plea. I have come to sever the voice that made obedience sound like grace.
The child lives beyond their knowing. For that alone, this heaven hath already failed.
Let the choir sing my sentence. I shall cut the breath between its words.
The thread runs farther than the throne. Very well. My hand is steady.
Another white door, another promise of absolution. I have no use for either.
I cut one command and found a thousand knots. The work was never going to end with a king.
The rift remembers my silence. Let it become acquainted with my answer.
There are no witnesses here. I shall remember enough for all of them.
My cloak grows dark at the hem. I have ceased mistaking that for shame.
A path need not lead home to be worth keeping open.
They made me to leave no trace. These broken gates shall be my signature.
If the voice returns, I shall listen only long enough to find its throat.
Somewhere below, a child grows older. For that small, stubborn miracle, I climb.'''.splitlines(),
'Proxy':'''They cast me beneath their stars when I asked whose sentence I was signing. The question survived the fall.
One ember of the borrowed sky remains. I shall see whether a made thing may choose what it illuminates.
The furnace knows no title. An instructive beginning for a king.
I remember a throne I never sat upon. I remember these screams more clearly. Which memory is mine?
My crown was loaned; the blade was not meant to question. Neither was I. We have both exceeded our design.
A severed seal gave me one unwritten moment. I have built this ascent upon it.
The lower gate was held in the sovereign's name. I have come to learn whether that name contains a man.
They called the condemned a necessary tribute. I was made to repeat the words. I shall not repeat them now.
There is a warmth in this star that no court ever taught me. I will keep it from their furnace.
If I am but another's shadow, let them explain why it is I who must bear the weight of his sword.
The drowned court. My cradle, my kingdom, and the first lie I learned to call a memory.
They left my throne beneath the water. A prudent place to hide an empty chair.
I know every star in this ceiling. I was taught their names before I was permitted one of my own.
Here they crowned me Proxy. A title that promised authority and withheld a self.
The blade remembers my hand. For once, something in this court remembers truly.
Their judges measured my obedience and called the result a soul. I have returned to dispute the measure.
A white knife broke the seal; an iron keeper opened his hand. Small acts, beyond the reach of their calculations.
I hear the old instruction in these halls: rule until He wakes. I begin to suspect that none wished Him to.
The water reflects a crown. I shall look beneath it before I claim what once was mine.
No star hath moved since my exile. The court confuses stillness with order, as it ever did.
Above the stars at last. If the sovereign sleeps here, I shall be an ill-mannered guest.
The light is borrowed too. I can feel the same absence behind it that they stitched beneath my crown.
I was made to hold a place for someone greater. Let us see what hath kept him so long.
No more decrees by an absent hand. Whoever dwells within shall speak in his own voice.
They named my blade sovereign, and its bearer substitute. One of those judgments must now fail.
A crown hangs where a face should be. I have climbed far enough to recognise a familiar deception.
If all my memories were given, this choice remains ungiven. I shall treasure it more than the throne.
The stars grow restless in my grasp. Perhaps even borrowed lights desire an end to service.
I do not seek to become the king they promised. I seek to learn what remains when the promise is refused.
Open thy golden doors. The regent hath come to discharge the sovereign.
Another empty throne. I shall keep the sword and leave the chair.
The sky repeats itself, but I need not.
If a borrowed life can outlast its lender, what debt remains?
They gave me stars to count my duties. I have begun naming them for things I choose.
Beyond the final heaven, no order awaits. The silence is almost kind.
I was fashioned to continue in another's absence. Now I continue in my own name.
The rift offers kingdoms without number. A single honest step is worth more.
This blade hath broken a crown. It need not become one.
Perhaps there is no original, only an endless court of substitutes. I shall be the first to leave its place empty.
The last star is not an ending. While I can carry it, there is light enough for one more stair.'''.splitlines()
}
# Each row is player / boss / player. Five distinct exchanges per pairing.
bosses={
'Warden':[
'''I kept thy gate. I shall keep it no longer.|A keeper is a hinge. A hinge doth not choose.|Then hear what iron sounds like when it breaks.
Those within thee had names.|Fuel hath no need of names.|I remember enough to bury thee beneath them.
Who bade thee burn the wards?|The seal descended. The gate obeyed. As didst thou.|Aye. That is why I have come.
Thy fire could not take the child.|One coal escaped. The furnace endureth.|One opened door is how a prison ends.
Stand aside. My watch is finished.|Thy oath is forged into these chains.|Then I shall break it link by link.'''.splitlines(),
'''Thy seal ordered the gate shut.|The hand that turned the key stands before me.|And the hand that signed shall stand beside it.
I have brought thy warrants home.|These halls receive petitions, not threats.|Call it what thou wilt. Hear it all the same.
How many names fit beneath one seal?|As many as order requireth.|Then let order learn the weight of one sword.
The water hath not washed thy law clean.|Law needeth no cleansing. Only obedience.|That well hath run dry.
I seek the voice above this court.|No keeper may pass beyond his station.|I have left my station in ashes.'''.splitlines(),
'''Was it thy voice that ordered the fire?|Thou didst need a voice. I gave thee one.|I need it no longer.
Open the lower gate.|It is thy hand that holds it closed, even now.|Then witness the hand change.
I have not come for pardon.|Every servant returns to ask release.|I have come to release the road.
What remains of a king without his keepers?|What remains of a keeper without his king?|A man who must choose for himself.
The dead followed me up thy stair.|Then thou hast brought my kingdom home.|Nay. I have brought its end.'''.splitlines()],
'Vesper':[
'''I know what lies behind thy bars.|Thou didst once make the delivery.|And now I have come for the keeper.
The child is beyond thy reach.|The white cloak returneth empty-handed.|Look more closely at my hands.
No more witnesses for thy fire.|Silence was thy gift to us.|Take back what was given.
Thy chains bear the choir's blessing.|As doth thy knife.|Then let blessing be turned against blessing.
I cut the seal. I shall cut the furnace from it.|A broken instrument cannot undo its work.|It can refuse the next command.'''.splitlines(),
'''Thou didst call the child a witness.|I called the child an inconvenience.|I have learned to be one also.
My sentence bore thy star.|Mercy is treason in a consecrated hand.|Then consecrate this.
Where ends thy law and begins the choir's desire?|There is no difference.|For once, thou speakest plainly.
I remember the names thy ledger omits.|An assassin's memory is poor testimony.|My blade shall provide the signature.
Thou gavest my order to another knife.|There are always other knives.|Then I shall sever the hand that points them.'''.splitlines(),
'''I have returned thy white cloak.|Thou hast stained it with disobedience.|The first stain worth keeping.
The child lives.|A single voice against a heaven.|Thou soundest afraid of it.
No more commands from behind a golden face.|Without my voice, what art thou?|Someone who can hear another.
The choir taught me to leave no witness.|Thou wert my finest silence.|Listen carefully. This is refusal.
I have found the end of the thread.|Cut it, and thou shalt have no place to return.|Then there shall be room for a road.'''.splitlines()],
'Proxy':[
'''I signed no sentence for these souls.|Thy crown was upon the seal.|Then the crown shall answer to its bearer.
They cast me down to be consumed.|All borrowed lights return to the furnace.|This one hath learned to burn elsewhere.
Dost thou know the sovereign's face?|I know the hunger beneath his name.|Then we have both been poorly served.
I carry a star beyond thy measure.|A coal, dressed for court.|Approach, and learn the difference.
I was made to rule in his absence.|Absence is the master of us all.|No longer mine.'''.splitlines(),
'''Thou didst crown me, then condemn me.|The office was true. The occupant was defective.|Behold the defect returning.
Whose memories didst thou place within me?|Those sufficient for obedience.|Thou shouldst have given me fewer questions.
The throne beneath thy water is empty.|It awaits a more faithful substitute.|Let it wait until the stars go cold.
I have brought the sovereign blade.|A borrowed weapon confers no authority.|Then this shall be an unauthorised act.
Did the king ever bid thee make me?|The realm required a voice.|And thou didst fear what it might say.'''.splitlines(),
'''I have ruled long in thy absence.|My absence is the throne thou didst serve.|Then I shall leave it vacant.
Which of my memories was thine?|Every thought began as my command.|This one shall end it.
I was told to wait until thou didst wake.|A useful promise to an obedient shadow.|I have grown tired of waiting.
What stands beneath that crown?|The shape of everything thou wert made to desire.|An emptiness, then. I know it well.
The regent hath come to discharge thee.|Without thy sovereign, thou art nothing.|Then nothing shall choose its own name.'''.splitlines()]
}
for name,lines in entries.items():assert len(lines)==40,(name,len(lines))
for name,acts in bosses.items():
 assert len(acts)==3
 for rows in acts:
  assert len(rows)==5
  for row in rows:assert len(row.split('|'))==3
order=['Warden','Vesper','Proxy']
data={'characters':order,'entries':[[entries[n][a*10:a*10+10] for a in range(4)] for n in order],'bosses':[[[r.split('|') for r in rows] for rows in bosses[n]] for n in order]}
(q/'story.json').write_text(json.dumps(data,indent=2)+'\n')
def c(v):return json.dumps(v,ensure_ascii=True) if isinstance(v,str) else '{'+','.join(c(x) for x in v)+'}'
(q/'source/story170_data.h').write_text('#pragma once\nstatic const char*const r170_entry[3][4][10]='+c(data['entries'])+';\nstatic const char*const r170_boss[3][3][5][3]='+c(data['bosses'])+';\n')
print('PASS authored 120 act-entry statements and 45 three-beat boss exchanges')
