# Ethics and authorized use

HackCard RP2350 is a lab board: NFC tags, a USB keyboard/mouse, storage, and LEDs. It has no Wi-Fi radio.

## You may use it when

- You own the tags and the computer it is plugged into, or
- You have permission from the owner, or
- You are in a class or lab that already allowed the exercise.

## Do not use it to

- Read or rewrite badges, cards, or tags you do not own
- Type into a computer without the operator knowing
- Present a written contact as someone else's

USB HID examples wait for a button or a countdown so you can focus your own editor first. NFC write examples target a tag you place on the coil.

`NFC_Kill_Type2` permanently closes a Type 2 tag. It asks for the lab PIN, shows the UID, and waits until you type YES. Use a spare tag you own. Do not use it on a payment card or on a badge you do not own. `NFC_Erase_Type2` only clears the user area, so that tag can be written again.

Tap-to-lock and the other NFC-to-HID sketches act on the computer the board is plugged into, and only for a tag you enrolled.

Unlock sketches type the password you write into that sketch. They do nothing until you replace `change-me`. Keep that sketch on hardware you control, and only enroll a tag you own.

Software license: [MIT](../LICENSE).
