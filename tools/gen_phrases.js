// Phase 29: ~130 phrases in groups, 8 languages each (ES, EN, FR, DE, IT, PT, ZH, KO).
// The first six must stay ASCII (the bitmap font), so no accents: German uses ae/oe/ue.
// Generates the S_TK_* strings in i18n and the group table in talk.cpp.
const fs = require('fs');
process.chdir(require('path').join(__dirname, '..'));   // the sketch folder

// [group, ...lines]; a line is [es, en, fr, de, it, pt, zh, ko]
const G = {
  HUNGRY: [
    ['Tengo hambre!', 'I\'m hungry!', 'J\'ai faim !', 'Ich habe Hunger!', 'Ho fame!', 'Estou com fome!', '我饿了!', '배고파!'],
    ['Mi pancita suena!', 'My tummy is rumbling!', 'Mon ventre gargouille !', 'Mein Bauch knurrt!', 'Mi brontola la pancia!', 'Minha barriga ronca!', '肚子咕咕叫!', '배에서 꼬르륵!'],
    ['Comemos algo?', 'Can we eat something?', 'On mange quelque chose ?', 'Essen wir etwas?', 'Mangiamo qualcosa?', 'Vamos comer algo?', '我们吃点东西吧?', '뭐 좀 먹을까?'],
    ['Una baya, por favor!', 'A berry, please!', 'Une baie, s\'il te plait !', 'Eine Beere, bitte!', 'Una bacca, per favore!', 'Uma fruta, por favor!', '请给我一个果子!', '열매 하나만 줘!'],
    ['Huele a comida rica...', 'Something smells yummy...', 'Ca sent bon...', 'Da riecht was lecker...', 'Che profumino...', 'Que cheirinho bom...', '好香啊...', '맛있는 냄새...'],
    ['Necesito energia!', 'I need a snack!', 'Il me faut un gouter !', 'Ich brauche einen Snack!', 'Mi serve uno spuntino!', 'Preciso de um lanche!', '我需要点心!', '간식이 필요해!'],
  ],
  DIRTY: [
    ['Necesito un bano!', 'I need a bath!', 'Il me faut un bain !', 'Ich brauche ein Bad!', 'Mi serve un bagno!', 'Preciso de um banho!', '我要洗澡!', '목욕하고 싶어!'],
    ['Nos banamos?', 'Can we have a bath?', 'On prend un bain ?', 'Baden wir?', 'Facciamo il bagno?', 'Vamos tomar banho?', '我们洗澡吧?', '목욕할까?'],
    ['Estoy todo sucio...', 'I\'m all muddy...', 'Je suis tout sale...', 'Ich bin ganz schmutzig...', 'Sono tutto sporco...', 'Estou todo sujo...', '我好脏啊...', '온몸이 지저분해...'],
    ['Burbujas, porfa!', 'Bubbles, please!', 'Des bulles, stp !', 'Seifenblasen, bitte!', 'Bolle, per favore!', 'Bolhas, por favor!', '想要泡泡!', '거품 목욕 해줘!'],
    ['Quiero oler a flores', 'I want to smell nice', 'Je veux sentir bon', 'Ich will gut riechen', 'Voglio profumare', 'Quero ficar cheiroso', '我想香香的', '좋은 냄새 나고 싶어'],
  ],
  SLEEPY: [
    ['Que sueno...', 'Yawn... so sleepy.', 'Bah... j\'ai sommeil.', 'Gaehn... so muede.', 'Che sonno...', 'Que sono...', '哈欠...好困。', '하암... 졸려.'],
    ['Una siestita?', 'A little nap?', 'Une petite sieste ?', 'Ein kleines Nickerchen?', 'Un pisolino?', 'Um cochilo?', '睡个午觉?', '낮잠 잘까?'],
    ['Mis ojitos se cierran', 'My eyes are closing', 'Mes yeux se ferment', 'Mir fallen die Augen zu', 'Mi si chiudono gli occhi', 'Meus olhos estao fechando', '眼睛快闭上了', '눈이 감겨'],
    ['Estoy cansadito', 'I\'m a bit tired', 'Je suis fatigue', 'Ich bin muede', 'Sono stanchino', 'Estou cansadinho', '我有点累', '조금 피곤해'],
    ['Me arropas?', 'Will you tuck me in?', 'Tu me bordes ?', 'Deckst du mich zu?', 'Mi rimbocchi le coperte?', 'Me cobre?', '帮我盖被子?', '이불 덮어줄래?'],
  ],
  SICK: [
    ['No me siento bien...', 'I don\'t feel well...', 'Je ne me sens pas bien...', 'Mir geht es nicht gut...', 'Non mi sento bene...', 'Nao me sinto bem...', '我不太舒服...', '몸이 안 좋아...'],
    ['Me duele la pancita', 'My tummy hurts', 'J\'ai mal au ventre', 'Mein Bauch tut weh', 'Mi fa male la pancia', 'Minha barriga doi', '我肚子疼', '배가 아파'],
    ['Achis! Achis!', 'Achoo! Achoo!', 'Atchoum ! Atchoum !', 'Hatschi! Hatschi!', 'Ecci! Ecci!', 'Atchim! Atchim!', '阿嚏!阿嚏!', '에취! 에취!'],
    ['Vamos al Centro?', 'Can we go to the Center?', 'On va au Centre ?', 'Gehen wir ins Center?', 'Andiamo al Centro?', 'Vamos ao Centro?', '去宝可梦中心吧?', '센터에 갈까?'],
    ['Una medicina, porfa', 'Some medicine, please', 'Un medicament, stp', 'Etwas Medizin, bitte', 'Una medicina, per favore', 'Um remedio, por favor', '请给我药', '약 좀 줘'],
  ],
  LONELY: [
    ['Te extrane!', 'I missed you!', 'Tu m\'as manque !', 'Ich habe dich vermisst!', 'Mi sei mancato!', 'Senti sua falta!', '我想你了!', '보고 싶었어!'],
    ['Juegas conmigo?', 'Will you play with me?', 'Tu joues avec moi ?', 'Spielst du mit mir?', 'Giochi con me?', 'Brinca comigo?', '陪我玩好吗?', '나랑 놀아줄래?'],
    ['Estaba solito...', 'I was all alone...', 'J\'etais tout seul...', 'Ich war ganz allein...', 'Ero tutto solo...', 'Eu estava sozinho...', '我一个人好孤单...', '혼자 있었어...'],
    ['No te vayas!', 'Don\'t go away!', 'Ne pars pas !', 'Geh nicht weg!', 'Non andare via!', 'Nao va embora!', '别走!', '가지 마!'],
    ['Me haces un carino?', 'Can I have a cuddle?', 'Un calin ?', 'Kuscheln wir?', 'Una coccola?', 'Me da um carinho?', '抱抱我?', '안아줄래?'],
  ],
  BORED: [
    ['Que aburrido...', 'I\'m so bored...', 'Je m\'ennuie...', 'Mir ist langweilig...', 'Che noia...', 'Que tedio...', '好无聊...', '심심해...'],
    ['Jugamos un juego?', 'Let\'s play a game!', 'On fait un jeu ?', 'Spielen wir ein Spiel?', 'Facciamo un gioco?', 'Vamos jogar?', '我们玩游戏吧!', '게임하자!'],
    ['Vamos al parque!', 'Let\'s go to the park!', 'Allons au parc !', 'Gehen wir in den Park!', 'Andiamo al parco!', 'Vamos ao parque!', '去公园吧!', '공원 가자!'],
    ['Hagamos algo divertido', 'Let\'s do something fun', 'Faisons un truc drole', 'Lass uns was Lustiges tun', 'Facciamo qualcosa di bello', 'Vamos fazer algo legal', '做点好玩的吧', '재미있는 거 하자'],
    ['Una carrera?', 'Race you!', 'On fait la course ?', 'Wettrennen?', 'Facciamo una gara?', 'Apostamos corrida?', '比赛跑步吧?', '달리기 시합?'],
  ],
  HAPPY: [
    ['Que bonito dia!', 'What a lovely day!', 'Quelle belle journee !', 'Was fuer ein schoener Tag!', 'Che bella giornata!', 'Que dia lindo!', '多美好的一天!', '정말 좋은 하루야!'],
    ['Estoy feliz!', 'I\'m so happy!', 'Je suis tout content !', 'Ich bin so froh!', 'Sono felicissimo!', 'Estou tao feliz!', '我好开心!', '너무 행복해!'],
    ['Que bueno que estas aqui', 'I\'m glad you\'re here', 'Contente que tu sois la', 'Schoen, dass du da bist', 'Che bello averti qui', 'Que bom que voce esta aqui', '很高兴你在这里', '네가 있어서 좋아'],
    ['Mira lo que se hacer!', 'Look what I can do!', 'Regarde ce que je sais faire !', 'Schau, was ich kann!', 'Guarda cosa so fare!', 'Olha o que eu sei fazer!', '看我会做什么!', '나 이거 할 수 있어!'],
    ['Hoy voy a ser valiente', 'Today I\'ll be brave', 'Aujourd hui je serai brave', 'Heute bin ich mutig', 'Oggi saro coraggioso', 'Hoje vou ser corajoso', '今天我要勇敢', '오늘은 용감해질 거야'],
    ['La la la!', 'La la la!', 'La la la !', 'La la la!', 'La la la!', 'La la la!', '啦啦啦!', '랄라라!'],
    ['Somos un gran equipo!', 'We make a great team!', 'On fait une super equipe !', 'Wir sind ein tolles Team!', 'Siamo una bella squadra!', 'Somos um otimo time!', '我们是最棒的组合!', '우리는 최고의 팀이야!'],
    ['Quiero aprender algo nuevo', 'I want to learn something', 'Je veux apprendre un truc', 'Ich will was lernen', 'Voglio imparare qualcosa', 'Quero aprender algo', '我想学点新东西', '새로운 걸 배우고 싶어'],
    ['Me gusta mi casa', 'I like our home', 'J\'aime notre maison', 'Ich mag unser Zuhause', 'Mi piace casa nostra', 'Gosto da nossa casa', '我喜欢我们的家', '우리 집이 좋아'],
    ['Que haremos hoy?', 'What shall we do today?', 'On fait quoi aujourd hui ?', 'Was machen wir heute?', 'Cosa facciamo oggi?', 'O que vamos fazer hoje?', '今天做什么呢?', '오늘 뭐 할까?'],
    ['Cuentame algo!', 'Tell me a story!', 'Raconte-moi une histoire !', 'Erzaehl mir was!', 'Raccontami una storia!', 'Me conta uma historia!', '给我讲个故事!', '이야기 해줘!'],
    ['Estoy listo para todo!', 'I\'m ready for anything!', 'Je suis pret a tout !', 'Ich bin zu allem bereit!', 'Sono pronto a tutto!', 'Estou pronto pra tudo!', '我什么都准备好了!', '뭐든 준비됐어!'],
  ],
  LOVE: [
    ['Te quiero mucho!', 'I love you lots!', 'Je t\'aime fort !', 'Ich hab dich lieb!', 'Ti voglio tanto bene!', 'Te amo muito!', '我好爱你!', '많이 사랑해!'],
    ['Eres mi mejor amigo', 'You\'re my best friend', 'Tu es mon meilleur ami', 'Du bist mein bester Freund', 'Sei il mio migliore amico', 'Voce e meu melhor amigo', '你是我最好的朋友', '넌 내 최고의 친구야'],
    ['Amigos para siempre!', 'Best friends forever!', 'Amis pour toujours !', 'Freunde fuer immer!', 'Amici per sempre!', 'Amigos para sempre!', '永远是好朋友!', '영원한 친구!'],
    ['Me cuidas muy bien', 'You take good care of me', 'Tu prends bien soin de moi', 'Du kuemmerst dich gut um mich', 'Ti prendi cura di me', 'Voce cuida bem de mim', '你把我照顾得很好', '날 잘 돌봐줘서 고마워'],
    ['Un abrazo gigante!', 'A giant hug!', 'Un calin geant !', 'Eine Riesenumarmung!', 'Un abbraccio gigante!', 'Um abraco gigante!', '一个大大的拥抱!', '꼭 안아줘!'],
    ['Gracias por todo', 'Thank you for everything', 'Merci pour tout', 'Danke fuer alles', 'Grazie di tutto', 'Obrigado por tudo', '谢谢你做的一切', '모든 게 고마워'],
    ['Contigo todo es mejor', 'Everything\'s better with you', 'Tout est mieux avec toi', 'Mit dir ist alles schoener', 'Con te e tutto meglio', 'Com voce tudo e melhor', '有你一切都更好', '너랑 있으면 다 좋아'],
    ['Eres especial', 'You\'re special', 'Tu es special', 'Du bist besonders', 'Sei speciale', 'Voce e especial', '你很特别', '넌 특별해'],
  ],
  MORNING: [
    ['Buenos dias!', 'Good morning!', 'Bonjour !', 'Guten Morgen!', 'Buongiorno!', 'Bom dia!', '早上好!', '좋은 아침!'],
    ['Dormiste bien?', 'Did you sleep well?', 'Bien dormi ?', 'Gut geschlafen?', 'Hai dormito bene?', 'Dormiu bem?', '睡得好吗?', '잘 잤어?'],
    ['Hora del desayuno!', 'Breakfast time!', 'C\'est l\'heure du petit-dej !', 'Zeit fuers Fruehstueck!', 'Ora di colazione!', 'Hora do cafe!', '早餐时间!', '아침 먹을 시간!'],
    ['Un nuevo dia!', 'A brand new day!', 'Une nouvelle journee !', 'Ein ganz neuer Tag!', 'Un giorno nuovo!', 'Um novo dia!', '新的一天!', '새로운 하루!'],
    ['A cepillarse los dientes!', 'Time to brush our teeth!', 'On se brosse les dents !', 'Zaehne putzen!', 'Laviamoci i denti!', 'Vamos escovar os dentes!', '刷牙时间到!', '양치하자!'],
  ],
  AFTERNOON: [
    ['Buenas tardes!', 'Good afternoon!', 'Bon apres-midi !', 'Guten Tag!', 'Buon pomeriggio!', 'Boa tarde!', '下午好!', '좋은 오후!'],
    ['Como te fue hoy?', 'How was your day?', 'Ta journee s\'est bien passee ?', 'Wie war dein Tag?', 'Com\'e andata oggi?', 'Como foi seu dia?', '你今天过得怎么样?', '오늘 하루 어땠어?'],
    ['Hora de la merienda!', 'Snack time!', 'L\'heure du gouter !', 'Zeit fuer einen Snack!', 'Ora della merenda!', 'Hora do lanche!', '点心时间!', '간식 시간!'],
    ['Hiciste la tarea?', 'Did you do your homework?', 'Tu as fait tes devoirs ?', 'Hast du deine Hausaufgaben?', 'Hai fatto i compiti?', 'Fez a licao de casa?', '作业做完了吗?', '숙제 했어?'],
  ],
  EVENING: [
    ['Buenas noches!', 'Good evening!', 'Bonsoir !', 'Guten Abend!', 'Buonasera!', 'Boa noite!', '晚上好!', '좋은 저녁!'],
    ['Ya casi es hora de dormir', 'It\'s nearly bedtime', 'C\'est bientot l\'heure du dodo', 'Bald ist Schlafenszeit', 'Quasi ora di dormire', 'Quase hora de dormir', '快到睡觉时间了', '곧 잘 시간이야'],
    ['Que cenamos?', 'What\'s for dinner?', 'On mange quoi ce soir ?', 'Was gibt es zum Abendessen?', 'Cosa c\'e per cena?', 'O que tem pro jantar?', '晚饭吃什么?', '저녁 뭐 먹어?'],
    ['Mira, salen las estrellas', 'Look, the stars are out', 'Regarde, les etoiles', 'Schau, die Sterne', 'Guarda, le stelle', 'Olha, as estrelas', '看,星星出来了', '봐, 별이 떴어'],
    ['Fue un buen dia', 'It was a good day', 'C\'etait une belle journee', 'Das war ein guter Tag', 'E stata una bella giornata', 'Foi um bom dia', '今天真不错', '좋은 하루였어'],
  ],
  NIGHT: [
    ['Dulces suenos...', 'Sweet dreams...', 'Fais de beaux reves...', 'Traeum was Schoenes...', 'Sogni d\'oro...', 'Bons sonhos...', '做个好梦...', '좋은 꿈 꿔...'],
    ['Ya es muy tarde!', 'It\'s very late!', 'Il est tres tard !', 'Es ist schon spaet!', 'E molto tardi!', 'Ja esta tarde!', '已经很晚了!', '벌써 늦었어!'],
    ['La luna esta bonita', 'The moon is pretty', 'La lune est jolie', 'Der Mond ist schoen', 'La luna e bellissima', 'A lua esta linda', '月亮真美', '달이 예뻐'],
  ],
  SUN: [
    ['Que dia tan soleado!', 'What a sunny day!', 'Quel beau soleil !', 'So ein sonniger Tag!', 'Che giornata di sole!', 'Que dia de sol!', '阳光真好!', '햇살 좋은 날!'],
    ['El sol se siente rico', 'The sun feels nice', 'Le soleil fait du bien', 'Die Sonne tut gut', 'Il sole e piacevole', 'O sol esta gostoso', '晒太阳真舒服', '햇볕이 따뜻해'],
    ['Salimos a jugar?', 'Shall we play outside?', 'On joue dehors ?', 'Spielen wir draussen?', 'Giochiamo fuori?', 'Vamos brincar la fora?', '出去玩吧?', '밖에서 놀까?'],
    ['Cielo azul!', 'Blue sky!', 'Ciel tout bleu !', 'Blauer Himmel!', 'Cielo azzurro!', 'Ceu azul!', '蓝天!', '파란 하늘!'],
  ],
  CLOUD: [
    ['Cuantas nubes...', 'So many clouds...', 'Que de nuages...', 'So viele Wolken...', 'Quante nuvole...', 'Quantas nuvens...', '好多云...', '구름이 많아...'],
    ['Esa nube parece un Pokemon', 'That cloud looks like a Pokemon', 'Ce nuage ressemble a un Pokemon', 'Die Wolke sieht aus wie ein Pokemon', 'Quella nuvola sembra un Pokemon', 'Aquela nuvem parece um Pokemon', '那朵云像宝可梦', '저 구름 포켓몬 같아'],
    ['Esta nublado hoy', 'It\'s cloudy today', 'C\'est nuageux aujourd hui', 'Heute ist es wolkig', 'Oggi e nuvoloso', 'Esta nublado hoje', '今天多云', '오늘은 흐려'],
  ],
  RAIN: [
    ['Esta lloviendo!', 'It\'s raining!', 'Il pleut !', 'Es regnet!', 'Piove!', 'Esta chovendo!', '下雨了!', '비가 와!'],
    ['Plic, plic, plic...', 'Pitter patter...', 'Plic, ploc...', 'Tropf, tropf...', 'Plic, plic...', 'Ping, ping...', '滴答滴答...', '똑똑똑...'],
    ['Saltamos en los charcos?', 'Puddle jumping?', 'On saute dans les flaques ?', 'In Pfuetzen springen?', 'Saltiamo nelle pozzanghere?', 'Pular nas pocas?', '去踩水坑吗?', '물웅덩이 뛰기?'],
    ['No olvides el paraguas', 'Don\'t forget an umbrella', 'N\'oublie pas ton parapluie', 'Vergiss den Schirm nicht', 'Non scordare l\'ombrello', 'Nao esqueca o guarda-chuva', '别忘了带伞', '우산 챙겨'],
  ],
  STORM: [
    ['Truenos! Quedate cerca', 'Thunder! Stay close to me', 'Le tonnerre ! Reste pres de moi', 'Donner! Bleib bei mir', 'Tuoni! Stammi vicino', 'Trovao! Fica perto de mim', '打雷了!待在我身边', '천둥이야! 곁에 있어줘'],
    ['Que tormenta!', 'What a storm!', 'Quel orage !', 'Was fuer ein Gewitter!', 'Che temporale!', 'Que tempestade!', '好大的暴风雨!', '폭풍우가 대단해!'],
    ['Vi un rayo!', 'I saw lightning!', 'J\'ai vu un eclair !', 'Ich habe einen Blitz gesehen!', 'Ho visto un fulmine!', 'Vi um raio!', '我看到闪电了!', '번개를 봤어!'],
  ],
  FOG: [
    ['No veo nada!', 'I can\'t see a thing!', 'Je ne vois rien !', 'Ich sehe nichts!', 'Non vedo niente!', 'Nao vejo nada!', '什么都看不见!', '아무것도 안 보여!'],
    ['Hay neblina hoy', 'It\'s foggy today', 'Il y a du brouillard', 'Heute ist es neblig', 'C\'e nebbia oggi', 'Tem neblina hoje', '今天有雾', '오늘 안개가 꼈어'],
    ['Que misterioso...', 'How mysterious...', 'Que c\'est mysterieux...', 'Wie geheimnisvoll...', 'Che mistero...', 'Que misterioso...', '好神秘...', '신비로워...'],
  ],
  SNOW: [
    ['Esta nevando!', 'It\'s snowing!', 'Il neige !', 'Es schneit!', 'Nevica!', 'Esta nevando!', '下雪了!', '눈이 와!'],
    ['Hacemos un muneco?', 'Let\'s build a snowman!', 'On fait un bonhomme ?', 'Bauen wir einen Schneemann!', 'Facciamo un pupazzo?', 'Vamos fazer um boneco?', '堆雪人吧!', '눈사람 만들자!'],
    ['Copos de nieve!', 'Snowflakes!', 'Des flocons !', 'Schneeflocken!', 'Fiocchi di neve!', 'Flocos de neve!', '雪花!', '눈송이!'],
    ['Guerra de bolas de nieve!', 'Snowball fight!', 'Bataille de boules de neige !', 'Schneeballschlacht!', 'Battaglia di palle di neve!', 'Guerra de bolas de neve!', '打雪仗!', '눈싸움!'],
  ],
  NYSNOW: [
    ['Nieve desde Nueva York!', 'Snow all the way from New York!', 'De la neige de New York !', 'Schnee aus New York!', 'Neve da New York!', 'Neve de Nova York!', '纽约来的雪!', '뉴욕에서 온 눈!'],
    ['En Nueva York hace frio!', 'It\'s cold in New York!', 'Il fait froid a New York !', 'In New York ist es kalt!', 'A New York fa freddo!', 'Esta frio em Nova York!', '纽约好冷!', '뉴욕은 추워!'],
  ],
  HOT: [
    ['Que calor!', 'It\'s so hot!', 'Il fait chaud !', 'Ist das heiss!', 'Che caldo!', 'Que calor!', '好热啊!', '너무 더워!'],
    ['Un helado, porfa!', 'An ice cream, please!', 'Une glace, stp !', 'Ein Eis, bitte!', 'Un gelato, per favore!', 'Um sorvete, por favor!', '请给我冰淇淋!', '아이스크림 줘!'],
    ['Necesito agua fresca', 'I need cold water', 'Il me faut de l\'eau fraiche', 'Ich brauche kaltes Wasser', 'Mi serve acqua fresca', 'Preciso de agua gelada', '我需要冰水', '시원한 물이 필요해'],
  ],
  COLD: [
    ['Brrr, que frio!', 'Brr, it\'s cold!', 'Brrr, il fait froid !', 'Brrr, ist das kalt!', 'Brrr, che freddo!', 'Brrr, que frio!', '好冷啊!', '으으, 추워!'],
    ['Necesito una bufanda', 'I need a scarf', 'Il me faut une echarpe', 'Ich brauche einen Schal', 'Mi serve una sciarpa', 'Preciso de um cachecol', '我需要围巾', '목도리가 필요해'],
    ['Un chocolate caliente?', 'Hot cocoa?', 'Un chocolat chaud ?', 'Heisse Schokolade?', 'Una cioccolata calda?', 'Um chocolate quente?', '来杯热可可?', '핫초코 마실까?'],
  ],
  GARDEN: [
    ['El jardin esta listo!', 'The garden is ready to pick!', 'Le jardin est pret !', 'Der Garten ist reif!', 'L\'orto e pronto!', 'A horta esta pronta!', '花园可以收获了!', '정원 수확할 시간!'],
    ['Crecio algo rico!', 'Something yummy grew!', 'Quelque chose a pousse !', 'Da ist was gewachsen!', 'E cresciuto qualcosa!', 'Algo gostoso cresceu!', '长出好吃的了!', '맛있는 게 자랐어!'],
    ['Vamos a cosechar!', 'Let\'s go harvest!', 'Allons recolter !', 'Lass uns ernten!', 'Andiamo a raccogliere!', 'Vamos colher!', '去收获吧!', '수확하러 가자!'],
  ],
  STUDY: [
    ['Estudiamos un rato?', 'Shall we study a bit?', 'On etudie un peu ?', 'Lernen wir ein bisschen?', 'Studiamo un po\'?', 'Vamos estudar um pouco?', '我们学习一会儿吧?', '조금 공부할까?'],
    ['Me encantan los numeros', 'I love numbers', 'J\'adore les nombres', 'Ich liebe Zahlen', 'Adoro i numeri', 'Adoro numeros', '我喜欢数字', '숫자가 좋아'],
    ['Juguemos al quiz!', 'Let\'s do the quiz!', 'On fait le quiz !', 'Machen wir das Quiz!', 'Facciamo il quiz!', 'Vamos fazer o quiz!', '来做测验吧!', '퀴즈 하자!'],
  ],
  ARCADE: [
    ['Vamos al arcade!', 'Let\'s go to the arcade!', 'Allons a l\'arcade !', 'Ab in die Spielhalle!', 'Andiamo in sala giochi!', 'Vamos ao fliperama!', '去游戏厅吧!', '오락실 가자!'],
    ['Quiero mas boletos!', 'I want more tickets!', 'Je veux plus de tickets !', 'Ich will mehr Tickets!', 'Voglio altri biglietti!', 'Quero mais bilhetes!', '我想要更多彩票!', '티켓 더 모을래!'],
    ['Batimos un record?', 'Shall we beat a record?', 'On bat un record ?', 'Brechen wir einen Rekord?', 'Battiamo un record?', 'Vamos bater um recorde?', '我们破纪录吧?', '기록 깰까?'],
    ['Quiero una medalla de oro!', 'I want a gold badge!', 'Je veux un badge d\'or !', 'Ich will ein Gold-Abzeichen!', 'Voglio una medaglia d\'oro!', 'Quero uma medalha de ouro!', '我想要金牌!', '금메달 갖고 싶어!'],
  ],
  NURSERY: [
    ['Hay un huevo en la guarderia!', 'An egg is waiting at the Nursery!', 'Un oeuf attend a la pension !', 'In der Pension wartet ein Ei!', 'Un uovo aspetta alla pensione!', 'Um ovo espera na creche!', '托儿所有一颗蛋!', '보육원에 알이 있어!'],
    ['Que saldra del huevo?', 'What will hatch?', 'Qu\'est-ce qui va eclore ?', 'Was wohl schluepft?', 'Cosa uscira dall\'uovo?', 'O que vai nascer?', '会孵出什么呢?', '뭐가 태어날까?'],
  ],
  MAIL: [
    ['Llego una carta!', 'A letter came for us!', 'Une lettre est arrivee !', 'Ein Brief ist da!', 'E arrivata una lettera!', 'Chegou uma carta!', '来信了!', '편지가 왔어!'],
    ['Leemos el correo?', 'Shall we read the mail?', 'On lit le courrier ?', 'Lesen wir die Post?', 'Leggiamo la posta?', 'Vamos ler a carta?', '我们看看信吧?', '편지 읽을까?'],
  ],
  QUEST: [
    ['Seguimos la aventura?', 'Shall we continue our quest?', 'On continue la quete ?', 'Machen wir mit der Mission weiter?', 'Continuiamo la missione?', 'Vamos continuar a missao?', '继续冒险吧?', '모험 계속할까?'],
    ['Hoy toca un paso mas!', 'One more quest step today!', 'Une etape de plus aujourd hui !', 'Heute ein Schritt mehr!', 'Oggi un passo in piu!', 'Mais um passo hoje!', '今天再完成一步!', '오늘 한 단계 더!'],
  ],
  JOB: [
    ['Trabaje mucho hoy!', 'I worked hard today!', 'J\'ai bien travaille !', 'Ich habe fleissig gearbeitet!', 'Ho lavorato tanto oggi!', 'Trabalhei muito hoje!', '我今天好努力工作!', '오늘 열심히 일했어!'],
    ['Gane monedas para nosotros', 'I earned coins for us', 'J\'ai gagne des pieces', 'Ich habe Muenzen verdient', 'Ho guadagnato monete', 'Ganhei moedas pra gente', '我赚了金币', '코인 벌었어'],
  ],
  BDAY: [
    ['Es nuestro cumple-mes!', 'It\'s our birthday month!', 'C\'est notre anniversaire !', 'Wir haben Geburtstag!', 'E il nostro compleanno!', 'E nosso aniversario!', '是我们的生日!', '우리 생일이야!'],
    ['Hay pastel?', 'Is there cake?', 'Il y a du gateau ?', 'Gibt es Kuchen?', 'C\'e la torta?', 'Tem bolo?', '有蛋糕吗?', '케이크 있어?'],
    ['Gracias por otro mes juntos', 'Thanks for another month together', 'Merci pour ce mois ensemble', 'Danke fuer noch einen Monat', 'Grazie per un altro mese insieme', 'Obrigado por mais um mes juntos', '谢谢又陪我一个月', '함께한 한 달 고마워'],
  ],
  HALLOWEEN: [
    ['Dulce o truco!', 'Trick or treat!', 'Des bonbons ou un sort !', 'Suesses oder Saures!', 'Dolcetto o scherzetto!', 'Gostosuras ou travessuras!', '不给糖就捣蛋!', '과자 안 주면 장난칠 거야!'],
    ['Buu! Te asuste?', 'Boo! Did I scare you?', 'Bouh ! Je t\'ai fait peur ?', 'Buh! Erschreckt?', 'Bu! Ti ho spaventato?', 'Bu! Te assustei?', '哇!吓到你了吗?', '왁! 놀랐지?'],
    ['Me gustan las calabazas', 'I love pumpkins', 'J\'adore les citrouilles', 'Ich liebe Kuerbisse', 'Adoro le zucche', 'Adoro aboboras', '我喜欢南瓜', '호박이 좋아'],
  ],
  WINTER: [
    ['Felices fiestas!', 'Happy holidays!', 'Joyeuses fetes !', 'Frohe Feiertage!', 'Buone feste!', 'Boas festas!', '节日快乐!', '즐거운 연휴!'],
    ['Habra regalos?', 'Will there be presents?', 'Il y aura des cadeaux ?', 'Gibt es Geschenke?', 'Ci saranno regali?', 'Vai ter presentes?', '会有礼物吗?', '선물 있을까?'],
    ['Me encantan las luces', 'I love the lights', 'J\'adore les lumieres', 'Ich liebe die Lichter', 'Adoro le luci', 'Adoro as luzes', '我喜欢彩灯', '불빛이 좋아'],
  ],
  EASTER: [
    ['Busquemos huevos!', 'Let\'s hunt for eggs!', 'Cherchons des oeufs !', 'Lass uns Eier suchen!', 'Cerchiamo le uova!', 'Vamos cacar ovos!', '去找彩蛋吧!', '달걀 찾으러 가자!'],
    ['Un huevo de colores!', 'A colourful egg!', 'Un oeuf colore !', 'Ein buntes Ei!', 'Un uovo colorato!', 'Um ovo colorido!', '彩色的蛋!', '알록달록 달걀!'],
  ],
  SUMMER: [
    ['Vamos a la playa!', 'Let\'s go to the beach!', 'Allons a la plage !', 'Ab an den Strand!', 'Andiamo al mare!', 'Vamos a praia!', '去海边吧!', '해변 가자!'],
    ['Viva el verano!', 'Hooray for summer!', 'Vive l\'ete !', 'Hurra, Sommer!', 'Evviva l\'estate!', 'Viva o verao!', '夏天万岁!', '여름 최고!'],
  ],
  BOND: [
    ['Siempre estare contigo', 'I\'ll always be with you', 'Je serai toujours avec toi', 'Ich bin immer bei dir', 'Saro sempre con te', 'Sempre estarei com voce', '我会一直陪着你', '항상 곁에 있을게'],
    ['Confio en ti', 'I trust you', 'J\'ai confiance en toi', 'Ich vertraue dir', 'Mi fido di te', 'Confio em voce', '我相信你', '널 믿어'],
    ['Juntos somos fuertes', 'Together we\'re strong', 'Ensemble on est forts', 'Zusammen sind wir stark', 'Insieme siamo forti', 'Juntos somos fortes', '在一起我们很强', '함께라면 강해'],
    ['Eres el mejor entrenador', 'You\'re the best trainer', 'Tu es le meilleur dresseur', 'Du bist der beste Trainer', 'Sei il miglior allenatore', 'Voce e o melhor treinador', '你是最棒的训练家', '넌 최고의 트레이너야'],
  ],
  EVOLVE: [
    ['Siento que voy a cambiar...', 'I feel like I\'m about to change...', 'Je sens que je vais changer...', 'Ich glaube, ich veraendere mich...', 'Sento che sto per cambiare...', 'Sinto que vou mudar...', '我觉得我要变了...', '곧 변할 것 같아...'],
    ['Algo me esta pasando!', 'Something is happening to me!', 'Il m\'arrive quelque chose !', 'Mit mir passiert etwas!', 'Mi sta succedendo qualcosa!', 'Algo esta acontecendo comigo!', '我身上发生了什么!', '뭔가 일어나고 있어!'],
  ],
};

// checks: ASCII, no '%', no stray backslashes in the first six
let total = 0;
for (const [g, lines] of Object.entries(G)) for (const l of lines) {
  if (l.length !== 8) throw new Error(g + ' needs 8 languages: ' + l[1]);
  for (let i = 0; i < 6; i++) if (/[^\x20-\x7e]/.test(l[i]) || l[i].includes('%')) throw new Error('not ASCII/has %: ' + g + ' ' + l[i]);
  total++;
}
console.log('lines', total, 'groups', Object.keys(G).length);
module.exports = G;

if (require.main === module) {
  // the strings
  const S = [];
  const groups = [];
  for (const [g, lines] of Object.entries(G)) {
    const ids = lines.map((l, i) => { const id = `S_TK_${g}_${i + 1}`; S.push([id, ...l]); return id; });
    const OLD = { HUNGRY: "S_TALK_HUNGRY", DIRTY: "S_TALK_DIRTY", SLEEPY: "S_TALK_SLEEPY", SICK: "S_TALK_SICK", LONELY: "S_TALK_LONELY", BORED: "S_TALK_PLAY", LOVE: "S_TALK_LOVE", HAPPY: "S_TALK_HAPPY", STUDY: "S_TALK_STUDY", GARDEN: "S_TALK_GARDEN", RAIN: "S_TALK_RAIN", SNOW: "S_TALK_SNOW", NYSNOW: "S_TALK_NY_SNOW" };
    if (OLD[g]) ids.push(OLD[g]);   // the 13 lines it already had join their group
    groups.push([g, ids]);
  }
  let h = fs.readFileSync('i18n.h', 'utf8');
  if (/\bS_TK_/.test(h)) throw new Error('already there');
  const nl = h.includes('\r\n') ? '\r\n' : '\n';
  const rows = [];
  for (let i = 0; i < S.length; i += 6) rows.push('  ' + S.slice(i, i + 6).map(r => r[0]).join(', ') + ',');
  h = h.replace('  STR_COUNT' + nl, '  // what it says (phase 29)' + nl + rows.join(nl) + nl + '  STR_COUNT' + nl);
  let c = fs.readFileSync('i18n.cpp', 'utf8');
  const cnl = c.includes('\r\n') ? '\r\n' : '\n';
  const lines = c.split(cnl);
  const st = lines.findIndex(l => l.startsWith('static const char *const STRINGS['));
  const en = lines.findIndex((l, i) => i > st && l === '};');
  let lang = 0;
  for (let i = st + 1; i < en; i++) if (/\},\s*$/.test(lines[i]) && !/^\s*\{/.test(lines[i])) {
    const k = lines[i].lastIndexOf('}');
    const body = [];
    for (let j = 0; j < S.length; j += 6) body.push('    ' + S.slice(j, j + 6).map(r => JSON.stringify(r[1 + lang])).join(', ') + ',');
    lines[i] = lines[i].slice(0, k).trimEnd() + cnl + '    // what it says' + cnl + body.join(cnl) + ' ' + lines[i].slice(k);
    lang++;
  }
  if (lang !== 8) throw new Error('rows');
  fs.writeFileSync('i18n.h', h); fs.writeFileSync('i18n.cpp', lines.join(cnl));
  // the group table
  const enumLine = 'enum TalkGroup : uint8_t {\n  ' + groups.map(([g]) => 'TG_' + g).join(', ') + ',\n  TG_COUNT\n};';
  const tbl = groups.map(([g, ids]) => `static const StrId TK_${g}[] = { ${ids.join(', ')} };`).join('\n');
  const idx = 'static const TalkSet TALK_SETS[TG_COUNT] = {\n' + groups.map(([g, ids]) => `  { TK_${g}, ${ids.length} },`).join('\n') + '\n};';
  fs.writeFileSync('talk_groups.h', `// Generated from tools/phrases (phase 29): the groups of things it says.\n#pragma once\n#include "i18n.h"\n${enumLine}\nstruct TalkSet { const StrId *lines; uint8_t n; };\n${tbl}\n${idx}\n`);
  console.log('written');
}
