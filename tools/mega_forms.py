"""Which species have a Mega form with usable art in PMD SpriteCollab, and which form folder it is in.

Checked against SpriteCollab's tracker.json and the files themselves (an Idle and a Walk sheet
exist). The folder is sprite/<dex>/<form>/. The alternate colours, Baby, Shadow and Z variants are
not used. mega.h has the same dex numbers (tools/emu/tests/mega_test.cpp checks the two agree).

dex -> form folder. Charizard is Mega X, Mewtwo is Mega Y: those are the ones SpriteCollab drew.
"""
MEGA_FORMS = {
    6: "0001", 65: "0001", 80: "0002", 94: "0001", 115: "0001", 142: "0001", 150: "0002",
    208: "0001", 227: "0001", 229: "0001", 248: "0001", 254: "0001", 282: "0001", 302: "0001",
    303: "0001", 308: "0001", 310: "0001", 319: "0001", 323: "0001", 334: "0001", 354: "0001",
    358: "0001", 359: "0001", 362: "0001", 380: "0001", 381: "0001", 384: "0001", 428: "0001",
    448: "0001", 475: "0001", 491: "0002", 530: "0001", 560: "0001", 604: "0001", 670: "0006",
    691: "0001", 701: "0001", 718: "0005", 719: "0001", 780: "0001", 807: "0001",
}
