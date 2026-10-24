#!/usr/bin/env python3
"""1.1.3: recover the PSP audio channel and embedded soundtrack on resume.
Only explicitly supplied music tracks are permitted; an unreadable EBOOT stays music-silent."""
from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
def rep(a,b):
 global s
 if a not in s: raise RuntimeError("v113 audio missing: "+a[:95])
 s=s.replace(a,b,1)
rep('''    soundtrack_open();
    while(running){''',
'''    soundtrack_open();
    unsigned reopen_counter=0;
    while(running){
        /* Memory stick I/O may be invalidated by PSP suspend/resume. */
        if(!soundtrack_active && (++reopen_counter%160u)==0u)
            soundtrack_open();
        if(channel<0) {
            channel=sceAudioChReserve(-1,512,PSP_AUDIO_FORMAT_STEREO);
            if(channel<0){sceKernelDelayThread(50000);continue;}
        }''')
rep('''        rift_audio_generate(&music,audio_buffer,512,track,1);''',
'''        /* SFX only. NEVER enable a synthesized music fallback. The
           final EBOOT is packaged with the user's ten supplied WAV tracks. */
        rift_audio_generate(&music,audio_buffer,512,track,1);''')
rep('''           }else audio_last=0;
           int l=audio_last+(int)audio_buffer[i*2]*2/3;
           int r=audio_last+(int)audio_buffer[i*2+1]*2/3;''',
'''           }else audio_last=0;
           int l=soundtrack_active?audio_last+(int)audio_buffer[i*2]*2/3:
              (int)audio_buffer[i*2];
           int r=soundtrack_active?audio_last+(int)audio_buffer[i*2+1]*2/3:
              (int)audio_buffer[i*2+1];''')
rep('''        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0)break;
    }
    soundtrack_close();''',
'''        if(sceAudioOutputBlocking(channel,0x4000,audio_buffer)<0){
            /* A failed blocking output must not kill the sole music worker. */
            sceAudioChRelease(channel);
            channel=-1;
            soundtrack_close();
            rift_sound_reset(&music);
            reopen_counter=159u; /* retry stream immediately */
        }
    }
    soundtrack_close();''')
rep('''    sceAudioChRelease(channel);
    return 0;''',
'''    if(channel>=0)sceAudioChRelease(channel);
    return 0;''')
rep('''     if(audio_length<=0)return 0;''',
'''     if(audio_length<=0){soundtrack_close();return 0;}''')
p.write_text(s)
print("PASS 1.1.3: recover audio on channel failure / suspend, reopen and user-supplied music only")
