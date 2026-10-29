# New-card balance review — revised 1.8.4

Reviewed all 60 character unlock cards and both upgrade states against the existing card pools and their actual engine behavior. Concepts, art, rarity and unlock progression are retained. These are targeted adjustments rather than a change to enemy difficulty.

| Cards | Adjustment | Reason |
|---|---|---|
| Blood Tithe, Quiet Step, Starlit Writ | Exhaust; Starlit Writ gives 1/2 Stars | Prevent repeatable free cycling into permanent Strength, Block or Stars. Blood Tithe's HP cost remains. |
| Merciless Mercy | Exhaust | Keep the conditional healing attack without allowing unlimited healing from a stalled fight. |
| Broken Zodiac | Exhaust | Retain the large burst of Stars and group Weak as a single-use setup card. |
| Kingdom of One | 2 Energy and Exhaust | Its large Block plus repeatable permanent Strength was substantially stronger than comparable defensive setup cards. |
| Red Bastion | 15/20 Block | The 2-HP payment no longer buys 18/24 Block repeatedly for one Energy. |
| White Wake | 4/5 damage, three hits | Retain Strength/multi-hit synergy while reducing its unconditional one-Energy output. |
| Blue Mandate | 6/8 damage, two hits | Its Star generation remains; reduce the damage bundled into the same one-Energy card. |
| Crown Breaker | 23/30 damage | Retain Vulnerable and strong rare damage, with a smaller unconditional damage advantage. |
| Pale Intercession | 18/25 Block | Preserve its one-Energy emergency defense and cleanse, with a more appropriate one-use Block value. |
| Exile's Mantle | At most 8 extra Block from Stars | Preserve the stockpiling incentive without giving a common repeatable card 18/22 Block for one Energy. |
| Astral Levy | 2 Energy; 14/19 damage | Preserve 4/5 Stars while charging for the combined resource and attack output. |
| Empty Court | 2 Energy | Preserve its 12/17 Block and two-card draw without greatly exceeding existing draw-and-defense cards. |
| The Last Decree | 0 Energy | A buff: Stars and Exhaust already pay for its variable card draw. |
| Threadcutter | Energy requires an actual discard | Correct the case where an empty hand received the Energy without paying the stated discard cost. |

The remaining 44 cards keep their existing values. This includes the high-impact Ancient finishers, cards with meaningful HP or Star costs, and synergy cards whose requirements already constrain their payoff.

Validation is recorded in `evidence/card-balance.csv` and `evidence/revision184r2.txt`: 240 actual combat cases across all cards/upgrades, one and two enemies, Poison, Strength, Dexterity, missing HP, Stars and Forge. Previews are checked against the resolved damage and Block; the original state must remain unchanged by previewing. Separate tiny-deck checks verify that the two free draw/resource loops terminate and that Threadcutter only grants Energy after a real discard. This is targeted balance testing, not a measured win-rate claim or a claim that every possible combo has been exhausted.

Starter/generated Proxy cards inherited the same rarity number as Ancient gifts. New reward/shop selection excludes those cards while keeping old saved offers readable. Boss packs still contain eligible Rare or Ancient cards.
