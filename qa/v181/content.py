"""Authored 1.8.1 side stories and always-available relics."""
from pathlib import Path
import json
r=Path(__file__).parent
stories=[
[
("THE UNBURIED STANDARD","A torn war-banner stands among graves with no names. Someone has kept its brazier burning.","WARDEN: I know this standard. They sent us here to die under it.","You pull it free. Beneath the iron point lies the last gift of a soldier who refused."),
("THE LAST WATCH","An empty suit of armor guards a grave. Its spear lowers when your shadow touches the stone.","WARDEN: Your watch is over. Mine isn't.","The armor collapses without a sound. A warm relic remains inside its cold breastplate."),
("A DEBT IN ASH","A wounded deserter recognizes the mark on your sword and reaches for a weapon he cannot lift.","WARDEN: Keep your breath. The men who gave that order still owe me theirs.","He gives you the route his commander feared to take, and the lesson he learned surviving it."),
("THE NINTH GRAVE","Eight graves surround an open ninth. The headstone bears your name, cut in fresh stone.","WARDEN: Whoever dug this should have made it deeper.","At the bottom waits a weapon lesson engraved by a hand exactly like your own."),
("THE FALSE CHAPLAIN","A chaplain asks you to kneel among the dead. None of his prayers cast a shadow.","WARDEN: I buried better men than your god. Stand aside.","His vestments empty themselves. The thing sewn inside them is older than his faith."),
("THE BLIND SMITH","A blind smith hears your sword before your footsteps. He knows every notch by its grief.","SMITH: Steel remembers what the hand tries to forget.","He teaches you where to strike when the enemy believes its armor has made it safe."),
("THE UNLIT FORGE","A forge has gone cold around a hammer that still beats. No hand holds it.","WARDEN: You can stop when the work is done. It isn't done.","You close your fist around the haft. Something stubborn and living answers from the iron."),
("A SWORD WITHOUT A NAME","A broken sword asks to be carried. Its old name has been scraped away so often the metal is thin.","WARDEN: You don't need a name. You need an enemy.","The blade crumbles into your gauntlet, leaving a fighting rhythm that will not be forgotten."),
("THE APPRENTICE'S TEST","An apprentice shows you a shield no hammer can dent. He asks what you think.","WARDEN: It won't save the arm that holds it.","One demonstration ruins the shield. The apprentice rewards the lesson with a better one."),
("THE CHAINMAKER","A chainmaker offers links forged for kings, saints, and condemned soldiers. One set has your measurements.","WARDEN: Make them smaller. They're for the men who sent me.","He laughs once, then gives you the only link his furnace could never melt."),
("THE RED FERRY","A ferryman asks for a memory instead of coin. The river carries shields face down.","WARDEN: Take the sound of their orders. I have no use for it.","Across the water, the silence leaves room for a technique you once knew before the war."),
("THE SUNKEN HELM","A helmet rises from the river without a head beneath it. Its visor is full of stars.","WARDEN: Even the dead are looking for a way out.","The current sets an old military relic at your feet, then takes the helmet back."),
("THE BRIDGE OF SPEARS","A bridge of broken spears holds above a black ravine. Each step sounds like a marching company.","WARDEN: We crossed worse with less. Move.","At the far end, a phantom sergeant salutes and shows you the stroke that broke his siege."),
("THE WOUNDED GIANT","A giant lies chained beside the river. It points at the lock instead of your sword.","WARDEN: I'm not here to finish someone else's punishment.","The freed giant draws a battle lesson in the mud, then disappears beneath the current."),
("THE SHORE OF BELLS","Bells roll onto the shore like stones. One rings only when your heart does.","WARDEN: If that's a warning, it arrived years late.","You carry the small bell away. Its weight settles beside the old scars."),
("THE EMPTY PEW","A single pew remains inside a burned chapel. A soldier's ghost leaves room beside him.","SOLDIER: Did we win? WARDEN: Not yet.","He nods as if that is enough, and shares the last formation his company held."),
("THE BROKEN CONFESSIONAL","A voice behind the grille offers absolution. It already knows the number of your dead.","WARDEN: I didn't come to be forgiven. I came to make it matter.","The grille breaks under your hand. A relic falls from a mouth that was never human."),
("THE SAINT'S SCAR","A stone saint bears the same scar as your chest. Fresh dust falls from it with every heartbeat.","WARDEN: You don't get to wear that unless you earned it.","The statue splits. The pattern inside its stone teaches you how to break a stronger guard."),
("THE CAPTAIN'S TABLE","Your old captain sits at a table set for the dead. He orders you to surrender your sword.","WARDEN: You lost the right to give me orders when you left them behind.","His chair turns to ash. On the table remains the tactic he never trusted you to learn."),
("THE VOW BELOW","Under the chapel floor, someone has written your oath in chains instead of ink.","WARDEN: I kept my part. Now the tower keeps its end.","You tear one link from the wall. The rest begin to tremble.")
],
[
("THE UNBOUGHT MASK","A masked merchant offers Vesper a face that no one has ever betrayed.","VESPER: A face is only useful until somebody learns to trust it.","She leaves the mask and takes the method hidden in its fastening."),
("THE SILENT AUCTION","An auctioneer sells the last words of assassins. He goes silent when Vesper raises her hand.","VESPER: Keep the words. Show me what they left behind.","A small relic changes hands without a bid, a receipt, or a witness."),
("THE COUNTERFEIT SHADOW","A child in the market wears Vesper's shadow like a cloak. It knows where her knives are hidden.","VESPER: Give it back before it teaches you something permanent.","The shadow returns with a stranger's technique folded into its edges."),
("THE POISONER'S COURTESY","An old poisoner pours two cups and drinks from both before offering either.","VESPER: That only proves we have different tolerances.","He smiles, abandons the cups, and shares a recipe too careful to write down."),
("THE THREAD DEALER","A woman sells threads taken from unfinished lives. One stretches from her stall to Vesper's wrist.","VESPER: Cut it. I prefer my exits unannounced.","The cut thread coils around a relic that has been waiting for a new owner."),
("THE MOON'S SECOND FACE","The canal reflects a second moon. It appears only when Vesper turns her back.","VESPER: If you're following me, learn to be quieter.","Ripples arrange themselves into the footwork of someone who never needed a bridge."),
("THE DROWNED MESSENGER","A messenger floats upright beneath the canal. His sealed letter is dry.","VESPER: Dead couriers usually mean the message is worth reading.","The letter dissolves. Its seal remains, warm with a promise the sender could not keep."),
("A NAME IN THE WATER","Someone whispers Vesper's old name from under the water. She does not answer.","VESPER: That person is gone. You're late.","The canal releases a lesson in leaving no trace, then forgets she was there."),
("THE LANTERN WITHOUT OIL","A lantern burns over the canal without oil or flame. Its light reveals every hidden blade except hers.","VESPER: Almost good enough.","She studies its blind spot until the darkness offers her another way to strike."),
("THE LAST GONDOLA","A boat arrives with no ferryman. Every seat holds the outline of a person who vanished.","VESPER: One crossing. No questions.","A silver relic waits beneath her seat. The boat is gone before she can step away."),
("THE UNRUNG BELL","A bell hangs above the roofs, wrapped in cloth thicker than armor. Something inside it scratches.","VESPER: Some things are buried for the sound they make.","She learns the knot, the angle, and the quiet way to make a guarded thing fall."),
("THE ROOFTOP WITNESS","A stone gargoyle says it saw her coming. Its eyes are sewn shut.","VESPER: Then you saw what I wanted you to see.","It spits out a relic stolen from the last person who believed its testimony."),
("THE KNIFE IN THE WEATHER","Rain bends around an invisible blade suspended between two roofs. It points toward her heart.","VESPER: You're going to need a hand to hold that.","She takes the blade's lesson and leaves its owner with nothing to aim."),
("THE WHITE FEATHER","A white feather lands on Vesper's shoulder. The sky above the roof has no birds.","VESPER: Tell whoever sent you that I don't take orders from heights.","Its barbs unfold into a precise diagram of a killing stroke."),
("THE HANGED LISTENER","A figure hangs beneath a bell tower, listening through the rope. It knows who follows her.","VESPER: Listen carefully. They're about to stop.","The rope parts. A small charm remains where its knot once held."),
("THE OPEN SANCTUARY","A sanctuary door opens before Vesper touches it. Every candle inside leans away from her.","VESPER: Good. We understand each other.","In the empty nave, a forgotten discipline waits where the faithful were afraid to look."),
("THE CONFESSOR'S EYE","A confessor offers to remove one memory. His open palm holds an eye that is not his.","VESPER: I keep the memories. They keep my aim honest.","He closes his hand and offers an older relic instead."),
("THE CHOIR OF ONE","One singer fills an abandoned choir stall with a hundred voices. All fall quiet when she enters.","VESPER: Which one of you knows how to leave?","The lowest voice teaches her, and the rest turn their faces to the wall."),
("THE UNFINISHED PRAYER","A prayer has been scratched into the sanctuary floor, stopping one word before its purpose.","VESPER: Some sentences are better left bleeding.","She reads the spaces between the words and learns how to turn hesitation into a weapon."),
("THE DOOR BEHIND THE ALTAR","Behind the altar is a door with no hinges. Its lock has already been picked from the other side.","VESPER: Finally. Someone with manners.","A relic waits in the threshold. She takes it without offering the darkness her name.")
],
[
("THE UNWRITTEN DECREE","In an astral archive, a decree bears the Proxy's seal and no words. The page refuses every pen.","PROXY: Sensible. Most laws improve when nobody writes them.","The blank page reveals an older method of command, beyond signatures and crowns."),
("THE LIBRARIAN OF DUST","A librarian catalogs the ashes of fallen kingdoms. The Proxy's shelf is still empty.","PROXY: Leave room. I intend to disappoint your predictions.","The librarian gives him a relic filed under rulers who changed the ending."),
("THE BORROWED CONSTELLATION","A star chart shows a constellation stolen from another sky. Its stars move when he breathes.","PROXY: A throne is just a borrowed place to stand. So is a heaven.","He takes the chart's arrangement into memory and returns the stolen sky."),
("THE KING WHO WAS A FOOTNOTE","A defeated king survives as a footnote in an unfinished history. He asks for a longer reign.","PROXY: You had one lifetime. Use the margin better.","The footnote becomes a lesson in making a small advantage rule an entire battle."),
("THE SEALED INDEX","An index lists every door the Proxy will open. The final entry has been torn out.","PROXY: At last. A future with the decency to keep a secret.","Inside the binding is a relic that no catalog will admit to owning."),
("THE BLIND OBSERVATORY","A telescope points into stone. Through its lens, an entire kingdom continues to burn.","PROXY: Lower it. We have spent enough time admiring our mistakes.","He studies the counterweight instead and learns a sharper balance of force."),
("THE ORRERY'S MISSING HEART","An orrery turns around an empty central ring. Every planet strains to escape it.","PROXY: Even the heavens know when their sovereign is absent.","He removes the ring's hidden relic, and the small worlds finally move freely."),
("A STAR WITHOUT A SUBJECT","A star refuses to appear in any constellation. The other lights keep their distance.","PROXY: Keep your independence. Lend me your technique.","Its light draws a single perfect stroke across his blade before returning to the dark."),
("THE ASTRONOMER'S ERROR","An astronomer insists the tower is the center of creation. His instruments point toward the Proxy.","PROXY: Your instruments confuse importance with noise.","They settle when he stops speaking, revealing a precise lesson in controlled power."),
("THE FALLEN MERIDIAN","A broken meridian divides the observatory into two different nights. Both contain the same approaching dawn.","PROXY: Two kingdoms. One horizon. We can work with that.","At the fracture he finds a relic that remembers how divided things once fit."),
("THE COURT OF EMPTY CHAIRS","An empty court rises when the Proxy enters. The chairs scrape in perfect unison.","PROXY: Sit. I have heard enough agreement from furniture.","Their silence teaches him the force of a command that needs no audience."),
("THE CROWN'S PETITION","A crown petitions for a head worthy of its burden. Its jewels are dried tears.","PROXY: Try ruling yourself first.","The crown breaks into a small relic, lighter and more useful than its title."),
("THE LAST ADVOCATE","An advocate argues a case against the sun. Every witness has been reduced to ash.","PROXY: A compelling case. Your choice of defendant needs work.","He helps rewrite the argument into a strategy for confronting something stronger."),
("THE THRONE THAT REMEMBERS","A throne remembers every person who sat in it and mistakes the Proxy for all of them.","PROXY: Then remember this one standing.","Its stone cracks around an old lesson in refusing the position an enemy prepares."),
("THE ROYAL INVENTORY","An inventory lists the possessions of a vanished court: seven swords, no bread, and one surviving oath.","PROXY: There is your explanation. Put the oath first.","The final clerk hands him a relic the treasury could never price."),
("THE CLOCK BETWEEN WORLDS","A clock hangs over a bridge with no opposite bank. Its hands count decisions instead of hours.","PROXY: I have never trusted a clock that knows too much.","He crosses when the hands disagree and learns to act between an enemy's certainties."),
("THE MINUTE OF GLASS","One minute lies shattered across the bridge. Each fragment shows a different defeat.","PROXY: Keep the warnings. I will take the piece that still has room.","The clear fragment becomes a relic, holding a silence between two heartbeats."),
("THE UNCLAIMED ECLIPSE","An eclipse waits above the bridge for someone to name it. Every suggested name makes it larger.","PROXY: You are entitled to neither a title nor my attention.","Denied a name, it contracts into a lesson in spending power only where it matters."),
("THE BRIDGE TOLL","A keeper asks for the weight of his crown. The Proxy places his sword on the scale instead.","PROXY: This is what it actually weighs.","The scale balances, and the keeper teaches him the stroke that settles an unequal bargain."),
("THE SECOND DAWN","Dawn rises beneath the bridge while night remains above. For a moment the tower has two shadows.","PROXY: Let them both follow. I am not turning back.","Between the shadows lies a relic untouched by either world's light.")
]]
relics=[
("Ashen Abacus","Every 5 cards played in a combat, gain 1 Energy."),
("Mourning Censer","Start each combat by applying 2 Weak to all enemies."),
("Pilgrim's Spur","Gain 1 additional Energy at the start of turn 2."),
("Glacial Nail","The first Attack hit each turn applies 1 Weak."),
("Martyr's Prism","Once each turn, after an enemy hit costs HP, gain 5 Block."),
("Scorched Sundial","At the start of every fourth turn, gain 2 Strength."),
("Widow's Loom","The first card Exhausted each turn heals 2 HP."),
("Ivory Tithe","After an elite or boss victory, gain 35 additional Gold."),
("Astral Lodestone","The first Power played each turn grants 1 Energy."),
("Bell of Ash","After your tenth card in a combat, apply 3 Weak and 3 Vulnerable to all enemies."),
("Vessel of Rain","After combat, heal 4 HP if you drank a potion during that combat."),
("Suture Crown","On pickup, gain 8 maximum HP and heal 8. After each combat, heal 1 HP."),
("Hollow Sextant","At turn start, after drawing, gain 6 Block if your draw pile has 3 or fewer cards."),
("Severed Psalm","Each Attack hit against an enemy below half health deals 4 additional damage."),
("Iron Testament","End your turn with no Block: gain 8 Block at the start of the next turn."),
("Candle of Mercy","Once per combat, survive an enemy hit at one-quarter HP or less to heal 12 HP."),
("Black Rosette","At the start of turn 3, apply 4 Poison to all enemies."),
("Quiet Astrolabe","Start combat with an extra card if your deck has 3 Powers; otherwise gain 4 Block."),
("Cracked Reliquary","Whenever you skip a combat card reward, gain 1 maximum HP and heal 1."),
("Ember Writ","Your first Attack played each combat grants 1 Energy.")
]
assert len(relics)==20 and [len(s) for s in stories]==[20]*3
rows=[]
for c,events in enumerate(stories):
 rows.append([dict(title=t,lines=list(lines),art=i//5,reward_kind=int(i%5 in (1,4)),rarity=2 if i%5==3 else 1,relic=74+(i+c*7)%20) for i,(t,*lines) in enumerate(events)])
(r/'content.json').write_text(json.dumps(dict(stories=rows,relics=[dict(id=74+i,name=n,description=d) for i,(n,d) in enumerate(relics)]),indent=2)+'\n')
q=lambda s:json.dumps(s,ensure_ascii=True)
out=['#ifndef EXPEDITION_STORIES_H','#define EXPEDITION_STORIES_H','typedef struct {const char*title;const char*lines[3];int art,reward_kind,rarity,relic;} ExpeditionStory;','static const ExpeditionStory expedition_stories[3][20]={']
for events in rows:
 out.append('{')
 for e in events:out.append('{'+q(e['title'])+',{'+','.join(map(q,e['lines']))+'},'+','.join(str(e[k]) for k in ['art','reward_kind','rarity','relic'])+'},')
 out.append('},')
out+=['};','#endif']
(r/'source/expedition_stories.h').write_text('\n'.join(out)+'\n')
(r/'source/expedition_relics.inc').write_text('\n'.join('{'+q(n)+','+q(d)+'},' for n,d in relics)+'\n')
print('Authored 60 character events / 180 dialogue pages and 20 base relics')
