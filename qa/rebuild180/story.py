from pathlib import Path
import json
root=Path(__file__).parent
chars=['WARDEN','VESPER','PROXY']
data=[
[
 ['You built a heaven out of our graves.','I climbed every damn one.','Keep your crown. I came for the head that wore it.'],
 ['That was no throne. It was a lock.','Something below the light is still pulling my chains.','Then I keep climbing. Until there is nothing left to kneel to.'],
 ['UNMAKER: I wrote your suffering into the stone.','WARDEN: Then watch me tear out the fucking page.','UNMAKER: You cannot kill the hand that made you.'],
 ['You forged me to bear your chains.','You should have made them stronger.','No more masters. No more graves. We walk out of here alive.']
],
[
 ['The choir promised I would die without a name.','Listen carefully. That silence is mine.','Tell your saints Vesper sent them.'],
 ['The choir is dead. So who is still singing?','There. A voice behind the wound in the world.','One last throat. One last prayer.'] ,
 ['UNMAKER: Every blade you carried was part of my design.','VESPER: Then you know exactly how this ends.','UNMAKER: I gave you your name. VESPER: You can choke on it.'],
 ['You taught the world to fear the dark.','Now it knows what the dark was hiding from.','My name is Vesper. And no one owns it anymore.']
],
[
 ['You crowned yourself with stolen stars.','I have come to collect.','Let heaven remember who brought the fucking sky down.'],
 ['These stars are not fading. They are being recalled.','A hand beyond the throne still claims them.','It will learn the difference between a sovereign and a servant.'],
 ['UNMAKER: I am the law that holds your heavens together.','PROXY: Then let the heavens break.','UNMAKER: Without me, you are nothing. PROXY: Watch nothing rise.'],
 ['Your throne was a cage with better lighting.','I owe this world more than another king.','Take back your stars. The sky belongs to all of us.']
]]
subjects=[
['a black-armored crown-helmet knight with torn crimson cape standing above a shattered golden throne in a ruined white-gold celestial cathedral',
 'the knight discovering a titanic chain running from a broken heaven through a violet fissure into a hidden cathedral beyond the stars',
 'the knight confronting the Unmaker across a shattered obsidian causeway suspended in a violet cosmic cathedral',
 'the knight breaking his last chains at a ruined cathedral gate as warm dawn reveals a free valley beyond'],
['an ivory-hooded female assassin with antique gold and black leather armor and twin curved knives looking over a silent white-gold celestial choir filled with broken marble saints',
 'the assassin kneeling beside a thin emerald seam of light in a ruined celestial sanctuary as an immense violet cathedral is revealed beyond it',
 'the assassin facing the Unmaker on an ivory bridge in a violet cosmic cathedral, her twin knives lowered and steady',
 'the assassin casting a cracked ivory ceremonial mask into a dying violet rift, turning toward a golden dawn outside an open cathedral'],
['a blue-robed bronze-crowned star sovereign with a gigantic golden greatsword standing before a fallen celestial throne beneath broken constellations',
 'the star sovereign raising a hand as gold constellations reveal hidden chains stretching into a violet cathedral beyond heaven',
 'the star sovereign confronting the Unmaker at the center of a shattered cosmic observatory with greatsword planted before him',
 'the star sovereign laying a broken crown on a stone altar as thousands of liberated gold stars rise over a dawn-lit kingdom']]
records=[]
for c in range(3):
 for kind in range(4):
  key=['ending','revelation','preboss','true-ending'][kind]
  records.append(dict(character=c,kind=kind,key=f'{chars[c].lower()}-{key}',lines=data[c][kind],subject=subjects[c][kind]))
(root/'story.json').write_text(json.dumps(records,indent=2)+'\n')
out=['/* Original dialogue written for the stable-base rebuild. */','static const char*const ascent_dialogue[3][4][3]={']
for c in data:out.append('{'+','.join('{'+','.join(json.dumps(t) for t in lines)+'}' for lines in c)+'},')
out.append('};');(root/'source/ascent_dialogue.h').write_text('\n'.join(out)+'\n')
