#include "i18n.h"
#include "pet.h"        // MED_COUNT
#include <Preferences.h>

Lang gLang = LANG_DEFAULT;

// String table [language][id]. No accents or enyes: the firmware's bitmap
// font does not have them (that is why Spanish already used "Esta", "bano", etc.).
static const char *const STRINGS[LANG_COUNT][STR_COUNT] = {
  // ---------------- ES ----------------
  {
    "Esta evolucionando!", "Nam nam!", "Le gusta!", "Tiene hambre!", "Necesita un bano!",
    "Esta agotado...", "Esta triste...", "Esta rellenito...", "Es SHINY!!", "Esta feliz",
    "GRACIAS! Hasta siempre", "Se ha escapado...", "Adios! Se despide...",
    "HUEVO", "Huevo legendario!?", "Huevo raro!", "Toca el huevo...", "Se mueve!", "Esta a punto!",
    "POKEDEX %u/%u",
    "%s%s Nv.%u",
    "Soltar a %s?", "SI", "NO",
    "%u GOLPES", "FUERZA +%u", "NUEVO RECORD!", "RECORD: %u", "APORREA RAPIDO!",
    "PUNTOS: %u", "Que felicidad!", "+felicidad",
    "AJUSTAR HORA", "HORA", "MIN", "desliza arriba: cancelar", "Idioma",
    "MEDALLA!", "GENIAL!", "RACHA %u DIAS!",
    "RACHA %u  rec %u", "VIN", "BAYA ???", "BAYA ROJA", "BAYA AZUL", "BAYA VERDE",
    "%s   EDAD %lud", "toca el nombre: renombrar",
    "COMBATE", "FUE", "DEF", "VEL", "PES", "ENTRENAR FUERZA",
    "MEDALLAS %d/%d", "toca: volver",
    "NOMBRE:", "toca para volver",
    "COM", "FEL", "ENE", "LIM",
    "REC %u",
    "PROGRESO", "Nv.%u", "%u min para Nv.%u", "EVOLUCION", "Forma final",
    "Listo para evolucionar!", "Sube todo a 40 para evolucionar",
    "Evoluciona en %u niv.", "Descuidos: %u",
    "ON", "OFF",
    "EVOLUCIONAR", "%s quiere decirte algo...", "%s se siente abandonado...",
    "Evolucionar?", "Mantener forma", "Despedirse?", "Despedirse", "Quedaros juntos",
    "Elige tu inicial",
    "Sin sprites", "Cargalos en la SD",
    "PS", "IV %u",
    "MENU", "AJUSTES", "CERRAR", "EQUIPO %u/6", "- vacio -", "%s se une al equipo!", "Equipo lleno: elige a quien sustituir", "Dejarlo ir",
    "STATS", "ENTRENAR", "FUERZA", "VELOCIDAD", "DEFENSA", "Sube sola si esta a gusto",
    "MOVIMIENTOS", "- vacio -", "Elige movimiento", "Toca para cambiar", "POT %u", "ESTADO",
    "%s quiere aprender", "No aprender",
    "%s usa %s", "Es muy eficaz!", "No es muy eficaz...", "No le afecta...",
    "%s ha fallado!", "Golpe critico!", "%s se debilita!", "Se ha herido!",
    "%s: %s", "PARALISIS", "QUEMADURA", "VENENO", "DORMIDO", "CONGELADO", "CONFUSION",
    "Has ganado!", "Has perdido...",
    "%s saca a %s", "Adelante, %s!", "GIMNASIOS", "MEDALLAS %u/8", "ENTRENADOR", "VELOCIDAD +%u", "toca: cambiar avatar", "%u en total", "NORMAL", "DIFICIL",
 "ELEGIDOS %u/%u", "LUCHAR", "BLOQUEADO", "POKEMON", "%s derrotado!", "MEDALLA NUEVA!", "VOL %u", "CAJA %u/%u", "cambiar con %s: elige hueco", "CAJA", "TRAER", "solo con un huevo", "COMBATE LAN", "CREAR", "UNIRSE", "buscando...", "listo!", "version distinta", "crear o unirse", "rival: %u mons", "el rival se fue", "esperando al rival...", "OTRA VEZ", "HUIR", "de que region viene el huevo", "%s +%u", "ya no puede entrenar mas", "ELIGE TU REGION", "RETIRAR", "Retirarla ya?", "la siguiente evoluciona un dia mas tarde", "evoluciona un dia mas tarde",   "FALTA PACK", "SOLTAR", "se va para siempre", "AL EQUIPO", "no se unira a tu equipo", "VOLUMEN", "IDIOMA", "INFO", "BRILLO", "PRUEBA",
    // Fun Pack
    "IZQ/DER", "MAS/MENOS", "QUIEN ES?", "BAYAS", "TOPOS", "COMBA", "BAILE", "ESCONDITE", "Hacia donde mira?", "Mayor o menor?", "MAYOR", "MENOR", "Quien es ese Pokemon?", "Toca izq/der: mover", "Tocalos! No a Voltorb", "Toca para saltar!", "Toca al ritmo!", "PERFECTO", "BIEN", "FALLO", "Donde se esconde?", "ALEGRIA +%u", "DEFENSA +%u", "BIEN!", "NO...", "Le duele la tripa!", "Esta resfriado!", "Esta grunon...", "Se aburre...", "Zzz... apaga la luz!", "Esta malito!", "Ya esta mejor!", "Otra dosis luego", "Espera 10 min", "Ahora un bano!", "Puaj! No esta malo", "MAS JUEGOS", "MINIJUEGOS",
    // Fun Pack
    "SACO", "REFLEJOS", "PELOTA", "DIVERSION",
    // coins and daily missions
    "MONEDAS", "MISIONES DE HOY", "Dale de comer %u veces", "Su baya favorita", "Juega %u minijuegos", "Camina %u pasos", "Dale un bano", "Acaricialo %u veces", "Un juego de fuerza", "Un juego de velocidad", "Un juego de defensa", "Saca %u puntos", "Por jugar hoy: %u/%u", "Todas: +%u extra", "COMO GANAR", "Jugar y cuidarlo", "+%u cada 1000 pasos", "Primer cuidado del dia +%u", "Cada mision +%u", "Medalla nueva +%u", "A dormir sin luz +%u", "La tienda llega pronto!",
    // shop, bag and hats
    "TIENDA", "MOCHILA", "SOMBREROS", "GORRA", "LAZO", "FLOR", "GORRO", "DE FIESTA", "DE PAJA", "DE BRUJA", "DE NAVIDAD", "CHISTERA", "CORONA", "COMPRAR", "NO", "TUYO", "Faltan monedas", "Comprado!", "PUESTO", "QUITAR SOMBRERO", "MOVER SOMBRERO", "Aun no tienes nada", "LISTO",
    // toys and decorations
    "JUGUETES", "DECORACION", "PUESTO", "PELOTA", "PELUCHE", "PATITO", "BLOQUES", "TAMBOR", "COMETA", "PLANTA", "FLORES", "LAMPARA", "CAMA", "REGALO", "CALABAZA",
    // foods
    "COMIDA", "COMIDAS", "DULCES", "BEBIDAS", "MEDICINA", "SANDWICH", "CURRY", "SOPA", "BOLA DE ARROZ", "PASTEL", "GALLETA", "HELADO", "LECHE", "ZUMO", "No te queda: ve a la tienda", "x%u",
    // bath and sleep menus
    "BANO", "DUCHA RAPIDA", "BANO DE BURBUJAS", "FROTAR", "CEPILLAR", "Frota las manchas!", "Reluciente!", "Necesitas jabon", "Compra un cepillo", "DORMIR", "A DORMIR", "SIESTA 30 MIN", "NANA", "CUIDADO", "JABON", "CEPILLO",
    // places, garden, seasonal gifts
    "SALIR", "PARQUE", "CENTRO POKEMON", "HUERTO", "PASEAR", "ENTRAR", "VOLVER", "CURAR", "Gratis hoy", "Esta como nuevo!", "Encontro %u monedas!", "Encontro una baya!", "Un %s te saluda!", "Que paseo tan bonito", "Esta muy cansado", "PLANTAR", "COSECHAR", "Compra semillas", "Creciendo... %u%%", "SEMILLA", "Cosechaste: %s", "Regalo de Halloween!", "Regalo de Navidad!", "Feliz Ano Nuevo!", "Regalo de San Valentin!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "FONDOS", "PRADERA", "PLAYA", "BOSQUE", "VOLCAN", "MONTANA", "NIEVE", "CEMENTERIO", "TROFEOS", "Nuevo trofeo! +10", "100 comidas", "50 juegos", "30 banos", "25 paseos", "5 cosechas", "1000 monedas", "5 sombreros", "6 adornos", "7 dias seguidos", "Nivel 20", "10 tareas", "14 cepilladas", "Examen perfecto", "10 capsulas", "CAPSULA", "Quedan %u hoy", "Salio: %s!", "Salieron 2 jabones!", "Salieron 2 semillas!", "Salieron %u monedas!", "Vuelve manana", "LAVAR DIENTES", "Dientes limpios!", "Ya se lavo 2 veces hoy", "Toca para cepillar", "ESCUELA", "ESTUDIAR", "MATES", "Cuanto tiempo?", "%u min", "Estudiando...", "Buen trabajo! +%u", "Buen trabajo!", "PARAR", "%u de 10!", "PADRES", "Hora de dormir", "Juegos al dia", "NO", "Toca para cambiar", "Es hora de dormir", "Basta de juegos por hoy", "Tengo hambre!", "Quiero un bano", "Tengo sueno...", "Me siento mal...", "Juguemos!", "Esta lloviendo!", "Esta nevando!", "El huerto esta listo!", "Te quiero!", "Que buen dia!", "Estudiamos?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "PASTEL DE BAYAS", "GUISO", "PARFAIT", "LAGO", "AVENTURA", "COCINA", "PESCAR", "Esperando...", "Pica! Toca ya!", "Muy pronto...", "Se escapo...", "Pescaste: %s", "una bota vieja", "una concha", "una perla", "Premios hoy: %u", "Solo por diversion", "SENDERO DEL BOSQUE", "PLAYA SOLEADA", "CAMINO DE MONTANA", "CUEVA VOLCANICA", "ABRIR", "Faltan %u pasos", "Un cofre! %u monedas", "Encontro: %s", "Ruta completada! +10", "Camina con la bola!", "Faltan ingredientes", "Toca para remover!", "Listo: %s!", "TAREAS", "ALCANCIA", "DIARIO", "Hacer la cama", "Hacer los deberes", "Ordenar el cuarto", "Ayudar en casa", "Leer un rato", "Lavarse los dientes", "Gracias! +4", "Ahorrado", "SACAR", "+%u en %u dias", "Ahorra y gana intereses cada semana", "Hoy", "Ayer", "Comio %u veces", "Jugo %u juegos", "Se bano %u veces", "Paseo %u veces", "Pesco %u veces", "Estudio %u veces", "Hiciste %u tareas", "Gano %u monedas", "Cocinamos juntos", "Fuimos de aventura", "Gano un trofeo!", "Capsula rara!", "Un dia tranquilo", "ALARMA", "SI", "Buenos dias!", "Toca para parar", "Hora de ir a dormir!", "20 peces", "Primera ruta", "CENTRO",
    // phase 12 games
    "PAREJAS", "SIMON DICE", "VUELO", "LABERINTO", "Encuentra las parejas", "Repite la secuencia", "Mira bien...", "Toca para volar", "Inclina la bola o arrastra",
    // phase 13: hats and the room
    "GORRO DE CHEF", "SOMBRERO PIRATA", "OREJAS DE GATO", "OREJAS DE CONEJO", "AURICULARES", "BIRRETE", "CORONA DE FLORES", "CASCO DE OBRA", "BOINA", "CASCO VIKINGO", "MUEBLES", "MI CUARTO", "VENTANA", "RELOJ", "CUADRO", "ESTANTE", "SOFA", "TELE", "ACUARIO", "LIBRERIA", "CHIMENEA", "ALFOMBRA", "PAPEL AZUL", "PAPEL ROSA", "PAPEL MENTA",
    // phase 14
    "Quiero %s!", "Quiero jugar!", "Vamos al parque!", "Quiero un bano!", "Vamos a pescar!", "Acariciame!", "Deseo cumplido! +5", "Te echa de menos", "Que gran dia!", "Te extrane...", "Feliz cumpleanos! +20", "%u meses!", "Un huevo de Pascua! +5", "Festival de verano: helado gratis!", "Personalidad: %s", "juguetona", "perezosa", "glotona", "timida",
    // phase 15
    "JUEGOS", "MENTE", "ARCADE", "GLOBOS", "CARRERA", "ATRAPA", "EQUILIBRIO", "TIPOS", "PUZLE", "BURBUJAS", "DIBUJO", "Revienta globos, no bombas", "Agita la bola o toca rapido", "Atrapa, esquiva a Voltorb", "Inclina o toca a un lado", "Que tan efectivo es?", "Ordena del 1 al 8", "Toca grupos del mismo color", "LISTO",
    // phase 16
    "ROPA", "GAFAS DE SOL", "GAFAS ESTRELLA", "GAFAS CORAZON", "MONOCULO", "BUFANDA", "PAJARITA", "MEDALLA", "PANUELO", "CAPA", "MOCHILA", "ALAS DE ANGEL", "ALAS DE MURCIELAGO", "Que elegante!",
    // phase 17
    "PLAYA", "BIBLIOTECA", "MUSICA", "CONCURSO", "ARCADE", "TRABAJOS", "CASTILLO", "Una concha! +1", "No hay mas conchas hoy", "Un castillo de arena!", "Ya hizo uno hoy", "LEER", "Ya leyo bastante hoy", "Basta de musica hoy", "BATALLA", "VICTORIA", "NANA", "PARTICIPAR", "Vuelve la semana que viene", "Sin cinta esta vez... +2", "Cinta de bronce! +5", "Cinta de plata! +10", "Cinta de oro! +20", "Ropa, carino y alegria cuentan", "CAFETERIA", "FLORISTERIA", "CORREOS", "3 h   +15", "Esta trabajando", "Vuelve a las %u:%02u", "Volvio del trabajo! +15", "Manana mas trabajo", "CORREO", "No hay cartas", "Prof. Oak: Gran trabajo! Te envio 5 monedas.", "Meowth: Una galleta de regalo, miau!", "Chansey: Una semilla para tu huerto.", "Pikachu: Pika pika! (un jabon)", "La abuela: 8 monedas para tus ahorros.", "Un amigo: Te echo de menos, juguemos!",
    // phase 19
    "AVENTURA", "Pikachu perdio su gorro", "El almuerzo de Meowth", "El huevo perdido de Chansey", "Una cancion para Jigglypuff", "Busca en el parque", "Busca en la playa", "Pregunta en la biblioteca", "Pesca algo en el lago", "Cocina un plato", "Comed juntos algo casero", "Abre una parada de aventura", "Visita el Centro Pokemon", "Date un bano", "Pon una cancion", "Juega un juego", "Estudia en la escuela", "El siguiente paso, manana", "Aventura completada! Un gorro nuevo", "Truco o trato! Una galleta", "Chocolate caliente gratis!", "Un ano juntos! +50",
    // the photo album
    "ALBUM", "Aun no hay fotos", "Foto guardada en el album!", "FOTO",
    // the Nursery
    "GUARDERIA", "Un dia juntos: 40 monedas", "DEJAR DOS", "Elige dos amigos", "PAGAR 40", "Ese no puede ir", "Hasta manana!", "Vuelve la proxima semana", "Listo en", "TOMAR HUEVO", "Un huevo! Sera el siguiente", "Tu proximo huevo", "DEVOLVER", "Toca otra vez para devolverlo", "Su huevo ya esta en casa", "De la guarderia",
    // Wi-Fi
    "Sin Wi-Fi: configuralo en la pagina de instalacion", "Wi-Fi: conectando...", "Hora del Wi-Fi", "Wi-Fi: no se pudo conectar",
    // snow from New York
    "Esta nevando en Nueva York!", "Mira, nieve de Nueva York!", "Nieve de Nueva York: SI", "Nieve de Nueva York: NO",
    // restful nights
    "Durmiendo: sus necesidades descansan",
    // the try-on mirror
    "PROBAR", "Como me veo?",
    // the arcade
    "PREMIOS", "RECORDS", "Boletos", "Lo ganaste!", "Faltan boletos", "GORRA ARCADE", "VISERA ESTRELLA", "MAQUINITA", "SEMANA", "MEJOR", "Medalla de bronce!", "Medalla de plata!", "Medalla de oro!",
    // arcade trophies
    "10 medallas de oro", "Los 3 premios del arcade",
    // what it says
    "Tengo hambre!", "Mi pancita suena!", "Comemos algo?", "Una baya, por favor!", "Huele a comida rica...", "Necesito energia!",
    "Necesito un bano!", "Nos banamos?", "Estoy todo sucio...", "Burbujas, porfa!", "Quiero oler a flores", "Que sueno...",
    "Una siestita?", "Mis ojitos se cierran", "Estoy cansadito", "Me arropas?", "No me siento bien...", "Me duele la pancita",
    "Achis! Achis!", "Vamos al Centro?", "Una medicina, porfa", "Te extrane!", "Juegas conmigo?", "Estaba solito...",
    "No te vayas!", "Me haces un carino?", "Que aburrido...", "Jugamos un juego?", "Vamos al parque!", "Hagamos algo divertido",
    "Una carrera?", "Que bonito dia!", "Estoy feliz!", "Que bueno que estas aqui", "Mira lo que se hacer!", "Hoy voy a ser valiente",
    "La la la!", "Somos un gran equipo!", "Quiero aprender algo nuevo", "Me gusta mi casa", "Que haremos hoy?", "Cuentame algo!",
    "Estoy listo para todo!", "Te quiero mucho!", "Eres mi mejor amigo", "Amigos para siempre!", "Me cuidas muy bien", "Un abrazo gigante!",
    "Gracias por todo", "Contigo todo es mejor", "Eres especial", "Buenos dias!", "Dormiste bien?", "Hora del desayuno!",
    "Un nuevo dia!", "A cepillarse los dientes!", "Buenas tardes!", "Como te fue hoy?", "Hora de la merienda!", "Hiciste la tarea?",
    "Buenas noches!", "Ya casi es hora de dormir", "Que cenamos?", "Mira, salen las estrellas", "Fue un buen dia", "Dulces suenos...",
    "Ya es muy tarde!", "La luna esta bonita", "Que dia tan soleado!", "El sol se siente rico", "Salimos a jugar?", "Cielo azul!",
    "Cuantas nubes...", "Esa nube parece un Pokemon", "Esta nublado hoy", "Esta lloviendo!", "Plic, plic, plic...", "Saltamos en los charcos?",
    "No olvides el paraguas", "Truenos! Quedate cerca", "Que tormenta!", "Vi un rayo!", "No veo nada!", "Hay neblina hoy",
    "Que misterioso...", "Esta nevando!", "Hacemos un muneco?", "Copos de nieve!", "Guerra de bolas de nieve!", "Nieve desde Nueva York!",
    "En Nueva York hace frio!", "Que calor!", "Un helado, porfa!", "Necesito agua fresca", "Brrr, que frio!", "Necesito una bufanda",
    "Un chocolate caliente?", "El jardin esta listo!", "Crecio algo rico!", "Vamos a cosechar!", "Estudiamos un rato?", "Me encantan los numeros",
    "Juguemos al quiz!", "Vamos al arcade!", "Quiero mas boletos!", "Batimos un record?", "Quiero una medalla de oro!", "Hay un huevo en la guarderia!",
    "Que saldra del huevo?", "Llego una carta!", "Leemos el correo?", "Seguimos la aventura?", "Hoy toca un paso mas!", "Trabaje mucho hoy!",
    "Gane monedas para nosotros", "Es nuestro cumple-mes!", "Hay pastel?", "Gracias por otro mes juntos", "Dulce o truco!", "Buu! Te asuste?",
    "Me gustan las calabazas", "Felices fiestas!", "Habra regalos?", "Me encantan las luces", "Busquemos huevos!", "Un huevo de colores!",
    "Vamos a la playa!", "Viva el verano!", "Siempre estare contigo", "Confio en ti", "Juntos somos fuertes", "Eres el mejor entrenador",
    "Siento que voy a cambiar...", "Algo me esta pasando!",
    // phase 31
    "Un bano de burbujas", "Lavale los dientes", "Una siesta o una nana", "Come algo casero", "Gana %u tickets", "Gira la capsula", "Haz el quiz de tipos", "Entra al concurso", "Pasea por el parque", "Llega a una parada", "Pesca %u veces", "Juega en la playa", "Visita el Centro", "Cocina un plato", "Planta o cosecha", "Haz %u tareas", "Lee un libro", "Estudia un rato", "Pon una cancion", "Abre una carta", "Ahorra en la alcancia", "Compra comida", "Compra algo", "Decora su casa", "Ayuda con una tarea", "Compra en la tienda", "Gana tickets en el arcade", "Camina 3000 pasos", "El huerto de Bulbasaur", "A Psyduck le duele la cabeza", "La gran siesta de Snorlax", "La fiesta de Eevee", "Squirtle va a la playa", "Magikarp quiere volar", "La noche de luna de Clefairy", "Abra va a la escuela",
    // phase 32
    "Barrer la caca", "No hay nada que barrer", "Barre la caca tambien!", "+%u de limpieza", "Todo barrido!",
    // phase 32b
    "Todo barrido! +%u de limpieza",
    // phase 33
    "Borrar esta foto?", "BORRAR", "BORRAR TODAS", "Borrar todas menos las favoritas?", "Foto borrada", "Ya tienes %u favoritas",
    // phase 34
    "%s te reta!", "Medalla %u de 8", "Alto Mando %u de 4", "La gran final: el Campeon", "LUCHAR!", "LUEGO",
    // phase 35
    "PASOS", "Hoy", "Ultimos 7 dias", "Pasos para llegar al gimnasio", "Ruta de aventura", "Total: %u pasos", "Listo! A luchar!", "LMXJVSD", "Todos los gimnasios ganados!",
    // phase 36
    "Pasos listos! El reto llega en %u dias", "Pasos listos! El reto llega manana",
    // phase 36
    "Reto en %u dias", "Reto manana",
    // phase 40
    "%s! +%u monedas", "Premio a los %u pasos",
    // phase 43
    "2500 pasos! Un bocadillo", "5000 pasos! Gran paseo", "10000 pasos! Mas fuerte", "Aprendiz", "Paseante", "Caminante", "Excursionista", "Senderista", "Guia de ruta", "Trotamundos", "As del camino", "Leyenda",
    // phase 44
    "Hierba alta", "BALLS", "Poke Ball", "Super Ball", "Ultra Ball", "Un %s salvaje!", "Atrapaste a %s!", "Se fue a la caja.", "%s salio de la bola!", "Equipo y caja llenos", "Sin bolas: hay en la tienda", "Encontraste una Poke Ball!", "Algo se mueve en la hierba!", "Camina %u pasos mas", "Ahora esta durmiendo", "No hay nadie en la hierba", "Se usan en la hierba alta",
    // phase 45
    "Excursion", "De excursion", "De vuelta!", "%s se fue de excursion!", "Ya no hay excursiones hoy", "Es tarde: manana sera", "Llamar", "Llamarlo? Sin historia", "%s fue al rio", "y encontro %u monedas!", "%s busco entre los arbustos", "y encontro una Poke Ball!", "%s conocio a %s", "y compartieron un bocadillo!", "%s paseo por el pueblo", "y le dieron una galleta!", "%s jugo en el jardin", "y encontro una semilla!", "%s salto en los charcos", "y trajo un jabon!", "%s vio algo moverse", "en la hierba alta!", "%s subio a una colina", "y encontro una Super Ball!", },
  // ---------------- EN ----------------
  {
    "Evolving!", "Yum yum!", "It likes it!", "It's hungry!", "Needs a bath!",
    "Worn out...", "Feeling sad...", "A bit chubby...", "It's SHINY!!", "It's happy",
    "THANKS! Farewell", "It ran away...", "Bye! Waving goodbye...",
    "EGG", "Legendary egg!?", "Rare egg!", "Tap the egg...", "It moves!", "Almost there!",
    "POKEDEX %u/%u",
    "%s%s Lv.%u",
    "Release %s?", "YES", "NO",
    "%u HITS", "STR +%u", "NEW RECORD!", "BEST: %u", "HIT FAST!",
    "SCORE: %u", "So much fun!", "+happiness",
    "SET TIME", "HOUR", "MIN", "swipe up: cancel", "Lang",
    "MEDAL!", "AWESOME!", "%u DAY STREAK!",
    "STREAK %u  best %u", "BOND", "BERRY ???", "RED BERRY", "BLUE BERRY", "GREEN BERRY",
    "%s   AGE %lud", "tap name: rename",
    "BATTLE", "ATK", "DEF", "SPD", "WGT", "TRAIN STRENGTH",
    "MEDALS %d/%d", "tap: back",
    "NAME:", "tap to go back",
    "FOOD", "JOY", "ENE", "HYG",
    "BEST %u",
    "PROGRESS", "Lv.%u", "%u min to Lv.%u", "EVOLUTION", "Final form",
    "Ready to evolve!", "All needs >=40 to evolve",
    "Evolves in %u lv.", "Slip-ups: %u",
    "ON", "OFF",
    "EVOLVE!", "%s wants to tell you...", "%s feels abandoned...",
    "Evolve?", "Keep form", "Say goodbye?", "Goodbye", "Stay together",
    "Choose your starter",
    "No sprites", "Load them onto the SD",
    "HP", "IV %u",
    "MENU", "SETTINGS", "CLOSE", "PARTY %u/6", "- empty -", "%s joined the party!", "Party full: pick who to replace", "Let it go",
    "STATS", "TRAINING", "STRENGTH", "SPEED", "DEFENCE", "Grows on its own when happy",
    "MOVES", "- empty -", "Choose a move", "Tap a slot to change", "PWR %u", "STATUS",
    "%s wants to learn", "Do not learn",
    "%s used %s", "Super effective!", "Not very effective...", "It had no effect...",
    "%s missed!", "Critical hit!", "%s fainted!", "It hurt itself!",
    "%s: %s", "PARALYSED", "BURNED", "POISONED", "ASLEEP", "FROZEN", "CONFUSED",
    "You win!", "You lost...",
    "%s sends out %s", "Go, %s!", "GYMS", "BADGES %u/8", "TRAINER",
 "SPEED +%u", "tap: change avatar", "%u earned in all", "EASY", "HARD", "CHOSEN %u/%u", "FIGHT", "LOCKED", "POKEMON", "%s defeated!", "NEW BADGE!", "VOL %u", "BOX %u/%u", "swap with %s: pick a slot", "BOX", "BRING BACK", "only while an egg waits", "LAN BATTLE", "HOST", "JOIN", "searching...", "ready!", "different version", "host or join", "rival: %u mons", "the rival left", "waiting for the rival...", "AGAIN", "RUN", "where this egg comes from", "%s +%u", "trained as far as it can go", "CHOOSE A REGION", "RETIRE", "Retire it early?", "the next one evolves a day later", "evolves a day later",
   "NEEDS PACK", "RELEASE", "gone for good", "TO PARTY", "it will not join your party", "VOLUME", "LANGUAGE", "ABOUT", "BRIGHTNESS", "TEST",
    // Fun Pack
    "LEFT/RIGHT", "HIGH/LOW", "WHO IS IT?", "BERRY DROP", "MOLE POP", "JUMP ROPE", "DANCE", "HIDE+SEEK", "Which way will it look?", "Higher or lower?", "HIGHER", "LOWER", "Who's that Pokemon?", "Tap left/right to move", "Tap them! Not Voltorb!", "Tap to jump!", "Tap on the beat!", "PERFECT", "GOOD", "MISS", "Where is it hiding?", "JOY +%u", "DEFENCE +%u", "YES!", "NOPE", "Tummy ache!", "Caught a cold!", "Grumpy...", "Bored...", "Zzz... lights on!", "Too sick to play!", "Feeling better!", "One more dose later", "Wait 10 minutes", "Now a bath!", "Yuck! Not sick", "MORE GAMES", "MINIGAMES",
    // Fun Pack
    "PUNCH BAG", "REACTION", "BALL GAME", "FUN",
    // coins and daily missions
    "COINS", "TODAY'S MISSIONS", "Feed it %u times", "Its favourite berry", "Play %u minigames", "Walk %u steps", "Give it a bath", "Pet it %u times", "A strength game", "A speed game", "A defence game", "Score %u points", "From play today: %u/%u", "All of them: +%u extra", "HOW TO EARN", "Play and care", "+%u per 1000 steps", "First care of the day +%u", "Each mission +%u", "New badge +%u", "Bedtime, light off +%u", "The shop is coming soon!",
    // shop, bag and hats
    "SHOP", "BAG", "HATS", "CAP", "BOW", "FLOWER", "BEANIE", "PARTY HAT", "STRAW HAT", "WITCH HAT", "SANTA HAT", "TOP HAT", "CROWN", "BUY", "NO", "OWNED", "Not enough coins", "Bought!", "WEARING", "TAKE HAT OFF", "MOVE HAT", "Nothing here yet", "DONE",
    // toys and decorations
    "TOYS", "DECORATION", "OUT", "BEACH BALL", "PLUSH", "RUBBER DUCK", "BLOCKS", "DRUM", "KITE", "PLANT", "FLOWERS", "LAMP", "BED", "GIFT", "PUMPKIN",
    // foods
    "FOOD", "MEALS", "SWEETS", "DRINKS", "MEDICINE", "SANDWICH", "CURRY", "SOUP", "RICE BALL", "CAKE", "COOKIE", "ICE CREAM", "MILK", "JUICE", "None left: try the shop", "x%u",
    // bath and sleep menus
    "BATH", "QUICK WASH", "BUBBLE BATH", "SCRUB", "BRUSH", "Rub the dirt off!", "Squeaky clean!", "You need soap", "Buy a brush first", "SLEEP", "BEDTIME", "30 MIN NAP", "LULLABY", "CARE", "SOAP", "BRUSH",
    // places, garden, seasonal gifts
    "GO OUT", "PARK", "POKEMON CENTER", "GARDEN", "WALK", "GO IN", "BACK", "HEAL", "Free today", "Good as new!", "It found %u coins!", "It found a berry!", "A wild %s says hi!", "What a nice walk", "Too tired", "PLANT", "HARVEST", "Buy some seeds", "Growing... %u%%", "SEED", "You picked: %s", "A Halloween gift!", "A Christmas gift!", "Happy New Year!", "A Valentine gift!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "SCENERY", "MEADOW", "BEACH", "FOREST", "VOLCANO", "MOUNTAIN", "SNOW", "GRAVEYARD", "TROPHIES", "New trophy! +10", "100 meals", "50 games", "30 baths", "25 walks", "5 harvests", "1000 coins", "5 hats", "6 decorations", "7-day streak", "Level 20", "10 study times", "14 brushings", "Perfect quiz", "10 capsules", "CAPSULE", "%u left today", "You got: %s!", "2 soaps!", "2 seeds!", "%u coins!", "Come back tomorrow", "BRUSH TEETH", "Sparkly teeth!", "Already brushed twice today", "Tap to brush", "SCHOOL", "STUDY", "MATH", "How long?", "%u min", "Studying...", "Great work! +%u", "Great work!", "STOP", "%u out of 10!", "PARENTS", "Bedtime", "Games a day", "OFF", "Tap to change", "It's bedtime", "Enough games for today", "I'm hungry!", "I need a bath", "I'm sleepy...", "I don't feel well...", "Let's play!", "It's raining!", "It's snowing!", "The garden is ready!", "I love you!", "What a great day!", "Shall we study?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "BERRY PIE", "STEW", "PARFAIT", "LAKE", "ADVENTURE", "KITCHEN", "FISH", "Waiting...", "A bite! Tap now!", "Too soon...", "It got away...", "You caught: %s", "an old boot", "a shell", "a pearl", "Prizes left today: %u", "Just for fun now", "FOREST PATH", "SUNNY BEACH", "MOUNTAIN TRAIL", "VOLCANO CAVE", "OPEN", "%u steps to go", "A chest! %u coins", "It found: %s", "Route complete! +10", "Walk with the ball!", "Missing ingredients", "Tap to stir!", "Ready: %s!", "CHORES", "PIGGY BANK", "DIARY", "Make the bed", "Do homework", "Tidy the room", "Help at home", "Read a little", "Brush my teeth", "Thank you! +4", "Saved", "TAKE OUT", "+%u in %u days", "Save and earn interest every week", "Today", "Yesterday", "Ate %u times", "Played %u games", "Had %u baths", "Went for %u walks", "Went fishing %u times", "Studied %u times", "You did %u chores", "Earned %u coins", "We cooked together", "We went on an adventure", "Won a trophy!", "A rare capsule!", "A quiet day", "ALARM", "ON", "Good morning!", "Tap to stop", "Time for bed!", "20 fish", "First route", "CENTER",
    // phase 12 games
    "MEMORY", "SIMON SAYS", "FLAPPY FLIGHT", "TILT MAZE", "Find the pairs", "Repeat the sequence", "Watch...", "Tap to fly", "Tilt the ball or drag",
    // phase 13: hats and the room
    "CHEF HAT", "PIRATE HAT", "CAT EARS", "BUNNY EARS", "HEADPHONES", "GRAD CAP", "FLOWER CROWN", "HARD HAT", "BERET", "VIKING HELMET", "FURNITURE", "MY ROOM", "WINDOW", "CLOCK", "PAINTING", "SHELF", "SOFA", "TV", "AQUARIUM", "BOOKCASE", "FIREPLACE", "RUG", "BLUE WALLPAPER", "PINK WALLPAPER", "MINT WALLPAPER",
    // phase 14
    "I want %s!", "I want to play!", "Let's go to the park!", "I want a bath!", "Let's go fishing!", "Pet me!", "Wish granted! +5", "Feels lonely", "Having a great day!", "I missed you...", "Happy birthday! +20", "%u months old!", "An Easter egg! +5", "Summer festival: a free ice cream!", "Personality: %s", "playful", "lazy", "a big eater", "shy",
    // phase 15
    "GAMES", "BRAIN", "ARCADE", "BALLOON POP", "SHAKE RACE", "CATCH", "BALANCE", "TYPE QUIZ", "SLIDE PUZZLE", "BUBBLES", "DRAWING", "Pop balloons, not bombs", "Shake the ball or tap fast", "Catch them, dodge Voltorb", "Tilt, or touch a side", "How effective is it?", "Put 1 to 8 in order", "Tap groups of one colour", "DONE",
    // phase 16
    "OUTFITS", "SUNGLASSES", "STAR GLASSES", "HEART GLASSES", "MONOCLE", "SCARF", "BOW TIE", "MEDAL", "BANDANA", "CAPE", "BACKPACK", "ANGEL WINGS", "BAT WINGS", "Looking great!",
    // phase 17
    "BEACH", "LIBRARY", "MUSIC HALL", "CONTEST", "ARCADE", "JOBS", "CASTLE", "A shell! +1", "No more shells today", "A sandcastle!", "Already built one today", "READ", "Enough reading today", "Enough music today", "BATTLE", "VICTORY", "LULLABY", "ENTER", "Come back next week", "No ribbon this time... +2", "Bronze ribbon! +5", "Silver ribbon! +10", "Gold ribbon! +20", "Outfit, bond and joy count", "CAFE", "FLOWER SHOP", "POST OFFICE", "3 h   +15", "It's at work", "Back at %u:%02u", "Home from work! +15", "More work tomorrow", "MAIL", "No letters", "Prof. Oak: Great job! Here are 5 coins.", "Meowth: A free cookie for you, meow!", "Chansey: A seed for your garden.", "Pikachu: Pika pika! (a soap)", "Grandma: 8 coins for your savings.", "A friend: I miss you, let's play!",
    // phase 19
    "QUEST", "Pikachu lost its hat", "Meowth's lunch", "Chansey's lost egg", "A song for Jigglypuff", "Search the park", "Search the beach", "Ask at the library", "Catch something at the lake", "Cook a dish", "Eat something home-made", "Open an adventure stop", "Visit the Pokemon Center", "Have a bath", "Play a song", "Play a game", "Study at school", "The next step tomorrow", "Quest complete! A new hat", "Trick or treat! A cookie", "A free hot cocoa!", "One year together! +50",
    // the photo album
    "ALBUM", "No photos yet", "Photo saved to the album!", "PHOTO",
    // the Nursery
    "NURSERY", "A day together: 40 coins", "LEAVE TWO", "Pick two friends", "PAY 40", "That one can't go", "See you tomorrow!", "Come back next week", "Ready in", "TAKE EGG", "An egg! It will be your next", "Your next egg", "GIVE BACK", "Tap again to give it back", "Their egg is at home now", "From the Nursery",
    // Wi-Fi
    "No Wi-Fi yet: set it up on the install page", "Wi-Fi: connecting...", "Clock set by Wi-Fi", "Wi-Fi: could not connect",
    // snow from New York
    "It's snowing in New York!", "Look, snow from New York!", "Snow from New York: ON", "Snow from New York: OFF",
    // restful nights
    "Sleeping: its needs are resting",
    // the try-on mirror
    "TRY ON", "How do I look?",
    // the arcade
    "PRIZES", "RECORDS", "Tickets", "You won it!", "Not enough tickets", "ARCADE CAP", "STAR VISOR", "ARCADE CABINET", "WEEK", "BEST", "Bronze badge!", "Silver badge!", "Gold badge!",
    // arcade trophies
    "10 gold badges", "All 3 arcade prizes",
    // what it says
    "I'm hungry!", "My tummy is rumbling!", "Can we eat something?", "A berry, please!", "Something smells yummy...", "I need a snack!",
    "I need a bath!", "Can we have a bath?", "I'm all muddy...", "Bubbles, please!", "I want to smell nice", "Yawn... so sleepy.",
    "A little nap?", "My eyes are closing", "I'm a bit tired", "Will you tuck me in?", "I don't feel well...", "My tummy hurts",
    "Achoo! Achoo!", "Can we go to the Center?", "Some medicine, please", "I missed you!", "Will you play with me?", "I was all alone...",
    "Don't go away!", "Can I have a cuddle?", "I'm so bored...", "Let's play a game!", "Let's go to the park!", "Let's do something fun",
    "Race you!", "What a lovely day!", "I'm so happy!", "I'm glad you're here", "Look what I can do!", "Today I'll be brave",
    "La la la!", "We make a great team!", "I want to learn something", "I like our home", "What shall we do today?", "Tell me a story!",
    "I'm ready for anything!", "I love you lots!", "You're my best friend", "Best friends forever!", "You take good care of me", "A giant hug!",
    "Thank you for everything", "Everything's better with you", "You're special", "Good morning!", "Did you sleep well?", "Breakfast time!",
    "A brand new day!", "Time to brush our teeth!", "Good afternoon!", "How was your day?", "Snack time!", "Did you do your homework?",
    "Good evening!", "It's nearly bedtime", "What's for dinner?", "Look, the stars are out", "It was a good day", "Sweet dreams...",
    "It's very late!", "The moon is pretty", "What a sunny day!", "The sun feels nice", "Shall we play outside?", "Blue sky!",
    "So many clouds...", "That cloud looks like a Pokemon", "It's cloudy today", "It's raining!", "Pitter patter...", "Puddle jumping?",
    "Don't forget an umbrella", "Thunder! Stay close to me", "What a storm!", "I saw lightning!", "I can't see a thing!", "It's foggy today",
    "How mysterious...", "It's snowing!", "Let's build a snowman!", "Snowflakes!", "Snowball fight!", "Snow all the way from New York!",
    "It's cold in New York!", "It's so hot!", "An ice cream, please!", "I need cold water", "Brr, it's cold!", "I need a scarf",
    "Hot cocoa?", "The garden is ready to pick!", "Something yummy grew!", "Let's go harvest!", "Shall we study a bit?", "I love numbers",
    "Let's do the quiz!", "Let's go to the arcade!", "I want more tickets!", "Shall we beat a record?", "I want a gold badge!", "An egg is waiting at the Nursery!",
    "What will hatch?", "A letter came for us!", "Shall we read the mail?", "Shall we continue our quest?", "One more quest step today!", "I worked hard today!",
    "I earned coins for us", "It's our birthday month!", "Is there cake?", "Thanks for another month together", "Trick or treat!", "Boo! Did I scare you?",
    "I love pumpkins", "Happy holidays!", "Will there be presents?", "I love the lights", "Let's hunt for eggs!", "A colourful egg!",
    "Let's go to the beach!", "Hooray for summer!", "I'll always be with you", "I trust you", "Together we're strong", "You're the best trainer",
    "I feel like I'm about to change...", "Something is happening to me!",
    // phase 31
    "A bubble bath", "Brush its teeth", "A nap or a lullaby", "Eat a home-cooked dish", "Win %u tickets", "Spin the capsule", "Play the type quiz", "Enter the contest", "A walk in the park", "Reach an adventure stop", "Fish %u times", "Play at the beach", "Visit the Center", "Cook a dish", "Plant or harvest", "Do %u chores", "Read a book", "Study for a while", "Play a song", "Open a letter", "Save in the piggy bank", "Buy some food", "Buy something", "Decorate its home", "Help with a chore", "Buy at the shop", "Win arcade tickets", "Walk 3000 steps", "Bulbasaur's garden", "Psyduck's headache", "Snorlax's big nap", "Eevee's birthday party", "Squirtle's beach day", "Magikarp wants to fly", "Clefairy's moon night", "Abra's first school day",
    // phase 32
    "Sweep the poop", "Nothing to sweep", "Sweep the poop too!", "+%u clean", "All swept!",
    // phase 32b
    "All swept! +%u clean",
    // phase 33
    "Delete this photo?", "DELETE", "DELETE ALL", "Delete all but the favourites?", "Photo deleted", "You already have %u favourites",
    // phase 34
    "%s challenges you!", "Badge %u of 8", "Elite Four %u of 4", "The final: the Champion", "FIGHT!", "LATER",
    // phase 35
    "STEPS", "Today", "Last 7 days", "Steps to reach the gym", "Adventure route", "Total: %u steps", "Ready! Let's fight!", "MTWTFSS", "Every gym beaten!",
    // phase 36
    "Steps done! The challenge comes in %u days", "Steps done! The challenge comes tomorrow",
    // phase 36
    "Challenge in %u days", "Challenge tomorrow",
    // phase 40
    "%s! +%u coins", "Next prize at %u steps",
    // phase 43
    "2500 steps! A snack", "5000 steps! A great walk", "10000 steps! Stronger", "Beginner", "Stroller", "Walker", "Hiker", "Trekker", "Route Guide", "Globetrotter", "Trail Ace", "Legend",
    // phase 44
    "Tall Grass", "BALLS", "Poke Ball", "Great Ball", "Ultra Ball", "A wild %s!", "You caught %s!", "It went to the box.", "%s broke free!", "Team and box are full", "No balls: buy some in the shop", "You found a Poke Ball!", "Something stirs in the grass!", "Walk %u more steps", "It's asleep right now", "Nobody is in the grass", "Use them in the Tall Grass",
    // phase 45
    "Trip", "On a trip", "Back home!", "%s set off on a trip!", "No more trips today", "Too late: try tomorrow", "Call back", "Call it back? No story", "%s went to the river", "and found %u coins!", "%s looked under a bush", "and found a Poke Ball!", "%s met %s", "and they shared a snack!", "%s walked through town", "and was given a cookie!", "%s played in a garden", "and found a seed!", "%s splashed in puddles", "and brought home soap!", "%s saw something move", "in the tall grass!", "%s climbed a hill", "and found a Great Ball!", },
  // ---------------- FR ----------------
  {
    "Il evolue!", "Miam miam!", "Il aime ca!", "Il a faim!", "Besoin d'un bain!",
    "Epuise...", "Triste...", "Un peu rond...", "C'est SHINY!!", "Il est content",
    "MERCI! Adieu", "Il s'est enfui...", "Au revoir!",
    "OEUF", "Oeuf legendaire!?", "Oeuf rare!", "Touche l'oeuf...", "Il bouge!", "Presque la!",
    "POKEDEX %u/%u",
    "%s%s Niv.%u",
    "Relacher %s?", "OUI", "NON",
    "%u COUPS", "FORCE +%u", "NOUVEAU RECORD!", "RECORD: %u", "FRAPPE VITE!",
    "SCORE: %u", "Trop bien!", "+bonheur",
    "REGLER L'HEURE", "HEURE", "MIN", "glisse haut: annuler", "Langue",
    "MEDAILLE!", "SUPER!", "SERIE %u JOURS!",
    "SERIE %u  rec %u", "LIEN", "BAIE ???", "BAIE ROUGE", "BAIE BLEUE", "BAIE VERTE",
    "%s   AGE %lud", "touche le nom: renommer",
    "COMBAT", "ATQ", "DEF", "VIT", "PDS", "ENTRAINER FORCE",
    "MEDAILLES %d/%d", "touche: retour",
    "NOM:", "touche pour revenir",
    "NOUR", "JOIE", "ENE", "HYG",
    "REC %u",
    "PROGRES", "Niv.%u", "%u min pour Niv.%u", "EVOLUTION", "Forme finale",
    "Pret a evoluer!", "Tout a 40 pour evoluer",
    "Evolue dans %u niv.", "Negligences: %u",
    "ON", "OFF",
    "EVOLUER", "%s veut te parler...", "%s se sent abandonne...",
    "Evoluer?", "Garder forme", "Dire adieu?", "Adieu", "Rester ensemble",
    "Choisis ton starter",
    "Pas de sprites", "Charge-les sur la SD",
    "PV", "IV %u",
    "MENU", "REGLAGES", "FERMER", "EQUIPE %u/6", "- vide -", "%s rejoint l'equipe!", "Equipe pleine: qui remplacer?", "Le laisser partir",
    "STATS", "ENTRAINEMENT", "FORCE", "VITESSE", "DEFENSE", "Monte seule s'il est heureux",
    "CAPACITES", "- vide -", "Choisis une capacite", "Touche pour changer", "PUI %u", "STATUT",
    "%s veut apprendre", "Ne pas apprendre",
    "%s utilise %s", "Tres efficace!", "Peu efficace...", "Aucun effet...",
    "%s a rate!", "Coup critique!", "%s est K.O.!", "Il se blesse!",
    "%s: %s", "PARALYSIE", "BRULURE", "POISON", "ENDORMI", "GELE", "CONFUSION",
    "Gagne!", "Perdu...",
    "%s envoie %s", "Vas-y, %s!", "ARENES", "BADGES %u/8", "DRESSEUR", "VITESSE +%u", "touche: changer d'avatar", "%u au total", "NORMAL", "DIFFICILE", "CHOISIS %u/%u", "COMBATTRE", "VERROUILLE", "POKEMON", "%s vaincu!", "NOUVEAU BADGE!", "VOL %u", "BOITE %u/%u", "echanger avec %s: choisis", "BOITE", "RAMENER", "seulement avec un oeuf", "COMBAT LAN", "CREER", "REJOINDRE", "recherche...", "pret!", "version differente", "creer ou rejoindre", "rival: %u mons", "le rival est parti", "en attente du rival...", "ENCORE", "FUIR", "d ou vient cet oeuf", "%s +%u", "ne peut plus progresser", "CHOISIS TA REGION", "RETIRER", "Retirer maintenant?", "le suivant evolue un jour plus tard", "evolue un jour plus tard",
   "PACK REQUIS", "RELACHER", "parti pour de bon", "A L EQUIPE", "ne rejoindra pas l equipe", "VOLUME", "LANGUE", "INFOS", "LUMINOSITE", "TEST",
    // Fun Pack
    "GAUCHE/DR", "PLUS/MOINS", "QUI EST-CE", "BAIES", "TAUPES", "CORDE", "DANSE", "CACHE", "Quel cote ?", "Plus ou moins ?", "PLUS", "MOINS", "Quel est ce Pokemon ?", "Touche gauche/droite", "Touche-les! Pas Voltorbe", "Touche pour sauter !", "Touche en rythme !", "PARFAIT", "BIEN", "RATE", "Ou se cache-t-il ?", "JOIE +%u", "DEFENSE +%u", "OUI !", "RATE", "Mal au ventre !", "Il est enrhume !", "Grognon...", "S'ennuie...", "Zzz... la lumiere !", "Trop malade !", "Ca va mieux !", "Encore une dose", "Attends 10 min", "Un bain maintenant !", "Beurk ! Pas malade", "PLUS DE JEUX", "MINI-JEUX",
    // Fun Pack
    "SAC", "REFLEXES", "BALLE", "AMUSEMENT",
    // coins and daily missions
    "PIECES", "MISSIONS DU JOUR", "Nourris-le %u fois", "Sa baie preferee", "Joue a %u mini-jeux", "Marche %u pas", "Donne-lui un bain", "Caresse-le %u fois", "Un jeu de force", "Un jeu de vitesse", "Un jeu de defense", "Marque %u points", "Jeu aujourd hui: %u/%u", "Toutes: +%u en plus", "COMMENT GAGNER", "Jouer et soigner", "+%u par 1000 pas", "Premier soin du jour +%u", "Chaque mission +%u", "Nouveau badge +%u", "Au lit sans lumiere +%u", "La boutique arrive bientot !",
    // shop, bag and hats
    "BOUTIQUE", "SAC", "CHAPEAUX", "CASQUETTE", "NOEUD", "FLEUR", "BONNET", "CHAPEAU FETE", "DE PAILLE", "DE SORCIERE", "PERE NOEL", "HAUT-DE-FORME", "COURONNE", "ACHETER", "NON", "A TOI", "Pas assez de pieces", "Achete !", "PORTE", "ENLEVER", "DEPLACER", "Rien pour l instant", "FINI",
    // toys and decorations
    "JOUETS", "DECORATION", "SORTI", "BALLON", "PELUCHE", "CANARD", "CUBES", "TAMBOUR", "CERF-VOLANT", "PLANTE", "FLEURS", "LAMPE", "LIT", "CADEAU", "CITROUILLE",
    // foods
    "NOURRITURE", "REPAS", "DOUCEURS", "BOISSONS", "MEDICAMENT", "SANDWICH", "CURRY", "SOUPE", "ONIGIRI", "GATEAU", "BISCUIT", "GLACE", "LAIT", "JUS", "Plus rien: va a la boutique", "x%u",
    // bath and sleep menus
    "BAIN", "DOUCHE", "BAIN MOUSSANT", "FROTTER", "BROSSER", "Frotte les taches !", "Tout propre !", "Il faut du savon", "Achete une brosse", "DODO", "AU LIT", "SIESTE 30 MIN", "BERCEUSE", "SOINS", "SAVON", "BROSSE",
    // places, garden, seasonal gifts
    "SORTIR", "PARC", "CENTRE POKEMON", "POTAGER", "PROMENER", "ENTRER", "RETOUR", "SOIGNER", "Gratuit aujourd hui", "Comme neuf !", "Il a trouve %u pieces !", "Il a trouve une baie !", "Un %s dit bonjour !", "Belle promenade", "Trop fatigue", "PLANTER", "RECOLTER", "Achete des graines", "Ca pousse... %u%%", "GRAINE", "Recolte: %s", "Cadeau d Halloween !", "Cadeau de Noel !", "Bonne annee !", "Cadeau de la St-Valentin !",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "DECORS", "PRAIRIE", "PLAGE", "FORET", "VOLCAN", "MONTAGNE", "NEIGE", "CIMETIERE", "TROPHEES", "Nouveau trophee ! +10", "100 repas", "50 jeux", "30 bains", "25 promenades", "5 recoltes", "1000 pieces", "5 chapeaux", "6 decorations", "7 jours de suite", "Niveau 20", "10 devoirs", "14 brossages", "Quiz parfait", "10 capsules", "CAPSULE", "Encore %u aujourd hui", "Tu as eu : %s !", "2 savons !", "2 graines !", "%u pieces !", "Reviens demain", "BROSSER LES DENTS", "Dents toutes propres !", "Deja brosse 2 fois", "Touche pour brosser", "ECOLE", "ETUDIER", "MATHS", "Combien de temps ?", "%u min", "Au travail...", "Bravo ! +%u", "Bravo !", "ARRETER", "%u sur 10 !", "PARENTS", "Coucher", "Jeux par jour", "NON", "Touche pour changer", "C'est l'heure de dormir", "Assez joue aujourd hui", "J'ai faim !", "Je veux un bain", "J'ai sommeil...", "Je me sens mal...", "On joue ?", "Il pleut !", "Il neige !", "Le potager est pret !", "Je t'aime !", "Quelle belle journee !", "On etudie ?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "TARTE AUX BAIES", "RAGOUT", "PARFAIT", "LAC", "AVENTURE", "CUISINE", "PECHER", "On attend...", "Ca mord ! Touche !", "Trop tot...", "Il s'est echappe...", "Tu as peche : %s", "une vieille botte", "un coquillage", "une perle", "Prix restants : %u", "Juste pour le plaisir", "SENTIER FORESTIER", "PLAGE ENSOLEILLEE", "CHEMIN DE MONTAGNE", "GROTTE VOLCANIQUE", "OUVRIR", "Encore %u pas", "Un coffre ! %u pieces", "Trouve : %s", "Route terminee ! +10", "Marche avec la balle !", "Il manque des ingredients", "Touche pour remuer !", "Pret : %s !", "CORVEES", "TIRELIRE", "JOURNAL", "Faire le lit", "Faire les devoirs", "Ranger la chambre", "Aider a la maison", "Lire un peu", "Se brosser les dents", "Merci ! +4", "Economise", "RETIRER", "+%u dans %u jours", "Economise et gagne des interets", "Aujourd hui", "Hier", "A mange %u fois", "A joue %u parties", "A pris %u bains", "%u promenades", "A peche %u fois", "A etudie %u fois", "Tu as fait %u corvees", "A gagne %u pieces", "On a cuisine ensemble", "On est partis a l'aventure", "A gagne un trophee !", "Une capsule rare !", "Une journee calme", "REVEIL", "OUI", "Bonjour !", "Touche pour arreter", "Au lit !", "20 poissons", "Premiere route", "CENTRE",
    // phase 12 games
    "MEMOIRE", "JACQUES A DIT", "VOL", "LABYRINTHE", "Trouve les paires", "Repete la suite", "Regarde...", "Touche pour voler", "Penche la balle ou glisse",
    // phase 13: hats and the room
    "TOQUE", "CHAPEAU DE PIRATE", "OREILLES DE CHAT", "OREILLES DE LAPIN", "CASQUE AUDIO", "TOQUE DE DIPLOME", "COURONNE DE FLEURS", "CASQUE DE CHANTIER", "BERET", "CASQUE VIKING", "MEUBLES", "MA CHAMBRE", "FENETRE", "HORLOGE", "TABLEAU", "ETAGERE", "CANAPE", "TELE", "AQUARIUM", "BIBLIOTHEQUE", "CHEMINEE", "TAPIS", "PAPIER BLEU", "PAPIER ROSE", "PAPIER MENTHE",
    // phase 14
    "Je veux %s !", "Je veux jouer !", "Allons au parc !", "Je veux un bain !", "Allons pecher !", "Caresse-moi !", "Voeu exauce ! +5", "Se sent seul", "Super journee !", "Tu m'as manque...", "Joyeux anniversaire ! +20", "%u mois !", "Un oeuf de Paques ! +5", "Fete d'ete : une glace offerte !", "Caractere : %s", "joueur", "paresseux", "gourmand", "timide",
    // phase 15
    "JEUX", "CERVEAU", "ARCADE", "BALLONS", "COURSE", "ATTRAPE", "EQUILIBRE", "QUIZ DES TYPES", "TAQUIN", "BULLES", "DESSIN", "Eclate les ballons, pas les bombes", "Secoue la balle ou touche vite", "Attrape-les, evite Voltorbe", "Penche ou touche un cote", "Quelle efficacite ?", "Range de 1 a 8", "Touche les groupes", "FINI",
    // phase 16
    "TENUES", "LUNETTES DE SOLEIL", "LUNETTES ETOILES", "LUNETTES COEUR", "MONOCLE", "ECHARPE", "NOEUD PAPILLON", "MEDAILLE", "BANDANA", "CAPE", "SAC A DOS", "AILES D'ANGE", "AILES DE CHAUVE-SOURIS", "Trop chic !",
    // phase 17
    "PLAGE", "BIBLIOTHEQUE", "MUSIQUE", "CONCOURS", "ARCADE", "PETITS BOULOTS", "CHATEAU", "Un coquillage ! +1", "Plus de coquillages", "Un chateau de sable !", "Deja fait aujourd hui", "LIRE", "Assez lu aujourd hui", "Assez de musique", "COMBAT", "VICTOIRE", "BERCEUSE", "PARTICIPER", "Reviens la semaine prochaine", "Pas de ruban... +2", "Ruban de bronze ! +5", "Ruban d'argent ! +10", "Ruban d'or ! +20", "Tenue, lien et joie comptent", "CAFE", "FLEURISTE", "POSTE", "3 h   +15", "Il est au travail", "Retour a %u:%02u", "Retour du travail ! +15", "Demain encore", "COURRIER", "Pas de lettre", "Prof. Chen : Bravo ! Voici 5 pieces.", "Miaouss : Un biscuit offert, miaou !", "Leveinard : Une graine pour ton potager.", "Pikachu : Pika pika ! (un savon)", "Mamie : 8 pieces pour tes economies.", "Un ami : Tu me manques, on joue ?",
    // phase 19
    "QUETE", "Pikachu a perdu son chapeau", "Le dejeuner de Miaouss", "L'oeuf perdu de Leveinard", "Une chanson pour Rondoudou", "Cherche au parc", "Cherche a la plage", "Demande a la bibliotheque", "Peche au lac", "Cuisine un plat", "Mangez un plat maison", "Ouvre une etape d'aventure", "Va au Centre Pokemon", "Prends un bain", "Joue une chanson", "Joue a un jeu", "Etudie a l'ecole", "Prochaine etape demain", "Quete reussie ! Un chapeau", "Un bonbon ou un sort ! Un biscuit", "Un chocolat chaud offert !", "Un an ensemble ! +50",
    // the photo album
    "ALBUM", "Pas encore de photos", "Photo dans l'album !", "PHOTO",
    // the Nursery
    "PENSION", "Un jour ensemble : 40 pieces", "EN LAISSER 2", "Choisis deux amis", "PAYER 40", "Celui-la ne peut pas", "A demain !", "Reviens la semaine prochaine", "Pret dans", "PRENDRE", "Un oeuf ! Ce sera le prochain", "Ton prochain oeuf", "RENDRE", "Touche encore pour le rendre", "Leur oeuf est a la maison", "De la pension",
    // Wi-Fi
    "Pas de Wi-Fi : regle-le sur la page d'installation", "Wi-Fi : connexion...", "Heure par Wi-Fi", "Wi-Fi : connexion impossible",
    // snow from New York
    "Il neige a New York !", "Regarde, la neige de New York !", "Neige de New York : OUI", "Neige de New York : NON",
    // restful nights
    "Il dort : ses besoins se reposent",
    // the try-on mirror
    "ESSAYER", "Je suis comment ?",
    // the arcade
    "PRIX", "RECORDS", "Tickets", "Tu l'as gagne !", "Pas assez de tickets", "CASQUETTE ARCADE", "VISIERE ETOILE", "BORNE D'ARCADE", "SEMAINE", "MEILLEUR", "Badge de bronze !", "Badge d'argent !", "Badge d'or !",
    // arcade trophies
    "10 badges d'or", "Les 3 prix de l'arcade",
    // what it says
    "J'ai faim !", "Mon ventre gargouille !", "On mange quelque chose ?", "Une baie, s'il te plait !", "Ca sent bon...", "Il me faut un gouter !",
    "Il me faut un bain !", "On prend un bain ?", "Je suis tout sale...", "Des bulles, stp !", "Je veux sentir bon", "Bah... j'ai sommeil.",
    "Une petite sieste ?", "Mes yeux se ferment", "Je suis fatigue", "Tu me bordes ?", "Je ne me sens pas bien...", "J'ai mal au ventre",
    "Atchoum ! Atchoum !", "On va au Centre ?", "Un medicament, stp", "Tu m'as manque !", "Tu joues avec moi ?", "J'etais tout seul...",
    "Ne pars pas !", "Un calin ?", "Je m'ennuie...", "On fait un jeu ?", "Allons au parc !", "Faisons un truc drole",
    "On fait la course ?", "Quelle belle journee !", "Je suis tout content !", "Contente que tu sois la", "Regarde ce que je sais faire !", "Aujourd hui je serai brave",
    "La la la !", "On fait une super equipe !", "Je veux apprendre un truc", "J'aime notre maison", "On fait quoi aujourd hui ?", "Raconte-moi une histoire !",
    "Je suis pret a tout !", "Je t'aime fort !", "Tu es mon meilleur ami", "Amis pour toujours !", "Tu prends bien soin de moi", "Un calin geant !",
    "Merci pour tout", "Tout est mieux avec toi", "Tu es special", "Bonjour !", "Bien dormi ?", "C'est l'heure du petit-dej !",
    "Une nouvelle journee !", "On se brosse les dents !", "Bon apres-midi !", "Ta journee s'est bien passee ?", "L'heure du gouter !", "Tu as fait tes devoirs ?",
    "Bonsoir !", "C'est bientot l'heure du dodo", "On mange quoi ce soir ?", "Regarde, les etoiles", "C'etait une belle journee", "Fais de beaux reves...",
    "Il est tres tard !", "La lune est jolie", "Quel beau soleil !", "Le soleil fait du bien", "On joue dehors ?", "Ciel tout bleu !",
    "Que de nuages...", "Ce nuage ressemble a un Pokemon", "C'est nuageux aujourd hui", "Il pleut !", "Plic, ploc...", "On saute dans les flaques ?",
    "N'oublie pas ton parapluie", "Le tonnerre ! Reste pres de moi", "Quel orage !", "J'ai vu un eclair !", "Je ne vois rien !", "Il y a du brouillard",
    "Que c'est mysterieux...", "Il neige !", "On fait un bonhomme ?", "Des flocons !", "Bataille de boules de neige !", "De la neige de New York !",
    "Il fait froid a New York !", "Il fait chaud !", "Une glace, stp !", "Il me faut de l'eau fraiche", "Brrr, il fait froid !", "Il me faut une echarpe",
    "Un chocolat chaud ?", "Le jardin est pret !", "Quelque chose a pousse !", "Allons recolter !", "On etudie un peu ?", "J'adore les nombres",
    "On fait le quiz !", "Allons a l'arcade !", "Je veux plus de tickets !", "On bat un record ?", "Je veux un badge d'or !", "Un oeuf attend a la pension !",
    "Qu'est-ce qui va eclore ?", "Une lettre est arrivee !", "On lit le courrier ?", "On continue la quete ?", "Une etape de plus aujourd hui !", "J'ai bien travaille !",
    "J'ai gagne des pieces", "C'est notre anniversaire !", "Il y a du gateau ?", "Merci pour ce mois ensemble", "Des bonbons ou un sort !", "Bouh ! Je t'ai fait peur ?",
    "J'adore les citrouilles", "Joyeuses fetes !", "Il y aura des cadeaux ?", "J'adore les lumieres", "Cherchons des oeufs !", "Un oeuf colore !",
    "Allons a la plage !", "Vive l'ete !", "Je serai toujours avec toi", "J'ai confiance en toi", "Ensemble on est forts", "Tu es le meilleur dresseur",
    "Je sens que je vais changer...", "Il m'arrive quelque chose !",
    // phase 31
    "Un bain moussant", "Brosse-lui les dents", "Sieste ou berceuse", "Un plat fait maison", "Gagne %u tickets", "Tourne la capsule", "Fais le quiz des types", "Va au concours", "Promenade au parc", "Atteins une etape", "Peche %u fois", "Joue a la plage", "Va au Centre", "Cuisine un plat", "Plante ou recolte", "Fais %u taches", "Lis un livre", "Etudie un peu", "Mets une chanson", "Ouvre une lettre", "Epargne dans la tirelire", "Achete a manger", "Achete quelque chose", "Decore sa maison", "Aide a une tache", "Achete a la boutique", "Gagne des tickets", "Marche 3000 pas", "Le jardin de Bulbizarre", "La migraine de Psykokwak", "La grande sieste de Ronflex", "La fete d'Evoli", "Carapuce a la plage", "Magicarpe veut voler", "La nuit de lune de Melofee", "Abra va a l'ecole",
    // phase 32
    "Balayer le caca", "Rien a balayer", "Balaie le caca aussi !", "+%u de proprete", "Tout est balaye !",
    // phase 32b
    "Tout est balaye ! +%u",
    // phase 33
    "Effacer cette photo ?", "EFFACER", "TOUT EFFACER", "Tout effacer sauf les favorites ?", "Photo effacee", "Deja %u favorites",
    // phase 34
    "%s te defie !", "Badge %u sur 8", "Conseil 4 : %u sur 4", "La finale : le Maitre", "COMBAT !", "PLUS TARD",
    // phase 35
    "PAS", "Aujourd'hui", "Les 7 derniers jours", "Pas pour aller a l'arene", "Route d'aventure", "Total : %u pas", "Pret ! Au combat !", "LMMJVSD", "Toutes les arenes gagnees !",
    // phase 36
    "Pas faits ! Le defi arrive dans %u jours", "Pas faits ! Le defi arrive demain",
    // phase 36
    "Defi dans %u jours", "Defi demain",
    // phase 40
    "%s ! +%u pieces", "Prix suivant a %u pas",
    // phase 43
    "2500 pas ! Un en-cas", "5000 pas ! Belle balade", "10000 pas ! Plus fort", "Novice", "Promeneur", "Marcheur", "Randonneur", "Trekkeur", "Guide", "Globe-trotteur", "As du sentier", "Legende",
    // phase 44
    "Hautes herbes", "BALLS", "Poke Ball", "Super Ball", "Hyper Ball", "Un %s sauvage !", "%s est capture !", "Il va dans la boite.", "%s s'est libere !", "Equipe et boite pleines", "Plus de balls: voir la boutique", "Tu trouves une Poke Ball !", "Ca bouge dans les herbes !", "Encore %u pas", "Il dort pour l'instant", "Personne dans les herbes", "A utiliser dans les herbes",
    // phase 45
    "Balade", "En balade", "De retour !", "%s part en balade !", "Plus de balade aujourd'hui", "Trop tard: a demain", "Rappeler", "Rappeler ? Pas d'histoire", "%s est alle a la riviere", "et trouve %u pieces !", "%s a fouille un buisson", "et trouve une Poke Ball !", "%s a rencontre %s", "et ils ont goute ensemble !", "%s a traverse la ville", "et a recu un biscuit !", "%s a joue au jardin", "et trouve une graine !", "%s a saute dans les flaques", "et rapporte du savon !", "%s a vu bouger quelque chose", "dans les hautes herbes !", "%s a grimpe une colline", "et trouve une Super Ball !", },
  // ---------------- DE ----------------
  {
    "Entwickelt sich!", "Mampf mampf!", "Gefaellt ihm!", "Hat Hunger!", "Braucht ein Bad!",
    "Erschoepft...", "Traurig...", "Etwas rundlich...", "Es ist SHINY!!", "Es ist froh",
    "DANKE! Lebwohl", "Es ist weg...", "Tschuess! Winkt",
    "EI", "Legendaeres Ei!?", "Seltenes Ei!", "Beruehre das Ei...", "Es bewegt sich!", "Fast soweit!",
    "POKEDEX %u/%u",
    "%s%s Lv.%u",
    "%s freilassen?", "JA", "NEIN",
    "%u TREFFER", "KRAFT +%u", "NEUER REKORD!", "REKORD: %u", "SCHNELL HAUEN!",
    "PUNKTE: %u", "Wie schoen!", "+Freude",
    "ZEIT STELLEN", "STD", "MIN", "hoch wischen: abbruch", "Sprache",
    "MEDAILLE!", "TOLL!", "%u TAGE SERIE!",
    "SERIE %u  rek %u", "BND", "BEERE ???", "ROTE BEERE", "BLAUE BEERE", "GRUENE BEERE",
    "%s   ALTER %lud", "Name tippen: umbenennen",
    "KAMPF", "ANG", "VER", "INI", "GEW", "KRAFT TRAINIEREN",
    "MEDAILLEN %d/%d", "tippen: zurueck",
    "NAME:", "tippen zum zurueck",
    "ESS", "FRO", "ENE", "HYG",
    "REK %u",
    "FORTSCHRITT", "Lv.%u", "%u min bis Lv.%u", "ENTWICKLUNG", "Endform",
    "Bereit zur Entwicklung!", "Alles >=40 zur Entwicklung",
    "Entwickelt in %u Lv.", "Patzer: %u",
    "AN", "AUS",
    "ENTWICKELN", "%s will dir etwas sagen...", "%s fuehlt sich verlassen...",
    "Entwickeln?", "Form behalten", "Abschied?", "Lebwohl", "Zusammen bleiben",
    "Waehle dein Starter",
    "Keine Sprites", "Auf die SD laden",
    "KP", "IV %u",
    "MENU", "EINSTELLUNGEN", "SCHLIESSEN", "TEAM %u/6", "- leer -", "%s kommt ins Team!", "Team voll: wen ersetzen?", "Ziehen lassen",
    "WERTE", "TRAINING", "STAERKE", "TEMPO", "ABWEHR", "Steigt von selbst bei guter Laune",
    "ATTACKEN", "- leer -", "Attacke waehlen", "Tippen zum Aendern", "STK %u", "STATUS",
    "%s will lernen", "Nicht lernen",
    "%s setzt %s ein", "Sehr effektiv!", "Nicht sehr effektiv...", "Keine Wirkung...",
    "%s hat verfehlt!", "Volltreffer!", "%s wurde besiegt!", "Es verletzt sich!",
    "%s: %s", "PARALYSE", "VERBRANNT", "VERGIFTET", "SCHLAEFT", "GEFROREN", "VERWIRRT",
    "Gewonnen!", "Verloren...",
    "%s schickt %s", "Los, %s!", "ARENEN", "ORDEN %u/8", "TRAINER", "TEMPO +%u", "tippen: Avatar wechseln", "%u insgesamt", "NORMAL", "SCHWER", "GEWAEHLT %u/%u", "KAEMPFEN", "GESPERRT", "POKEMON", "%s besiegt!", "NEUER ORDEN!", "LAUT %u", "BOX %u/%u", "mit %s tauschen: waehle", "BOX", "ZURUECK", "nur mit einem Ei", "LAN KAMPF", "HOSTEN", "BEITRETEN", "suche...", "bereit!", "andere Version", "hosten oder beitreten", "Gegner: %u", "der Gegner ist weg", "warte auf den Gegner...", "NOCHMAL", "FLUCHT", "woher dieses Ei kommt", "%s +%u", "kann nicht weiter trainieren", "WAEHLE DEINE REGION", "VERABSCHIEDEN", "Jetzt verabschieden?", "das naechste entwickelt sich einen Tag spaeter", "entwickelt sich einen Tag spaeter",
   "PACK FEHLT", "FREILASSEN", "fuer immer weg", "INS TEAM", "kommt nicht ins team", "LAUTSTAERKE", "SPRACHE", "INFO", "HELLIGKEIT", "TEST",
    // Fun Pack
    "LINKS/RE", "HOCH/TIEF", "WER IST ES", "BEEREN", "MAULWURF", "SEILSPRUNG", "TANZ", "VERSTECK", "Wohin schaut es?", "Hoeher oder tiefer?", "HOEHER", "TIEFER", "Wer ist das Pokemon?", "Links/rechts tippen", "Tippen! Nicht Voltobal", "Tippen zum Springen!", "Im Takt tippen!", "PERFEKT", "GUT", "DANEBEN", "Wo versteckt es sich?", "FREUDE +%u", "VERTEID. +%u", "JA!", "NEIN", "Bauchweh!", "Erkaeltet!", "Grummelig...", "Langweilt sich...", "Zzz... Licht aus!", "Zu krank!", "Geht besser!", "Spaeter noch eine", "10 Min warten", "Jetzt baden!", "Igitt! Nicht krank", "MEHR SPIELE", "MINISPIELE",
    // Fun Pack
    "SANDSACK", "REAKTION", "BALLSPIEL", "SPASS",
    // coins and daily missions
    "MUENZEN", "HEUTIGE AUFGABEN", "%u mal fuettern", "Seine Lieblingsbeere", "Spiele %u Minispiele", "Gehe %u Schritte", "Bade es", "Streichle es %u mal", "Ein Kraftspiel", "Ein Tempospiel", "Ein Abwehrspiel", "Erziele %u Punkte", "Heute durch Spiel: %u/%u", "Alle: +%u extra", "SO VERDIENST DU", "Spielen und pflegen", "+%u je 1000 Schritte", "Erste Pflege am Tag +%u", "Jede Aufgabe +%u", "Neuer Orden +%u", "Ins Bett, Licht aus +%u", "Der Laden kommt bald!",
    // shop, bag and hats
    "LADEN", "BEUTEL", "HUETE", "KAPPE", "SCHLEIFE", "BLUME", "MUETZE", "PARTYHUT", "STROHHUT", "HEXENHUT", "NIKOLAUSHUT", "ZYLINDER", "KRONE", "KAUFEN", "NEIN", "DEINS", "Zu wenig Muenzen", "Gekauft!", "TRAEGT", "HUT AB", "HUT VERSCHIEBEN", "Noch nichts da", "FERTIG",
    // toys and decorations
    "SPIELZEUG", "DEKORATION", "DRAUSSEN", "WASSERBALL", "KUSCHELTIER", "QUIETSCHENTE", "BAUKLOETZE", "TROMMEL", "DRACHEN", "PFLANZE", "BLUMEN", "LAMPE", "BETT", "GESCHENK", "KUERBIS",
    // foods
    "ESSEN", "MAHLZEITEN", "SUESSES", "GETRAENKE", "MEDIZIN", "SANDWICH", "CURRY", "SUPPE", "REISBALL", "KUCHEN", "KEKS", "EIS", "MILCH", "SAFT", "Keins mehr: zum Laden", "x%u",
    // bath and sleep menus
    "BAD", "DUSCHE", "SCHAUMBAD", "SCHRUBBEN", "BUERSTEN", "Reib den Dreck weg!", "Blitzsauber!", "Du brauchst Seife", "Kauf eine Buerste", "SCHLAFEN", "INS BETT", "NICKERCHEN 30 MIN", "SCHLAFLIED", "PFLEGE", "SEIFE", "BUERSTE",
    // places, garden, seasonal gifts
    "AUSGEHEN", "PARK", "POKEMON-CENTER", "GARTEN", "SPAZIEREN", "REINGEHEN", "ZURUECK", "HEILEN", "Heute gratis", "Wie neu!", "%u Muenzen gefunden!", "Eine Beere gefunden!", "Ein %s sagt hallo!", "Schoener Spaziergang", "Zu muede", "PFLANZEN", "ERNTEN", "Kauf Samen", "Waechst... %u%%", "SAMEN", "Geerntet: %s", "Halloween-Geschenk!", "Weihnachtsgeschenk!", "Frohes neues Jahr!", "Valentinsgeschenk!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "KULISSEN", "WIESE", "STRAND", "WALD", "VULKAN", "BERG", "SCHNEE", "FRIEDHOF", "TROPHAEEN", "Neue Trophaee! +10", "100 Mahlzeiten", "50 Spiele", "30 Baeder", "25 Spaziergaenge", "5 Ernten", "1000 Muenzen", "5 Huete", "6 Dekos", "7 Tage am Stueck", "Level 20", "10x Lernen", "14x Zaehneputzen", "Perfektes Quiz", "10 Kapseln", "KAPSEL", "Heute noch %u", "Bekommen: %s!", "2 Seifen!", "2 Samen!", "%u Muenzen!", "Komm morgen wieder", "ZAEHNE PUTZEN", "Saubere Zaehne!", "Heute schon 2x geputzt", "Tippen zum Putzen", "SCHULE", "LERNEN", "MATHE", "Wie lange?", "%u Min", "Lernzeit...", "Super gemacht! +%u", "Super gemacht!", "STOPP", "%u von 10!", "ELTERN", "Schlafenszeit", "Spiele am Tag", "AUS", "Tippen zum Aendern", "Schlafenszeit!", "Genug gespielt heute", "Ich hab Hunger!", "Ich will baden", "Ich bin muede...", "Mir gehts nicht gut...", "Lass uns spielen!", "Es regnet!", "Es schneit!", "Der Garten ist reif!", "Hab dich lieb!", "So ein schoener Tag!", "Wollen wir lernen?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "BEERENKUCHEN", "EINTOPF", "PARFAIT", "SEE", "ABENTEUER", "KUECHE", "ANGELN", "Warten...", "Biss! Jetzt tippen!", "Zu frueh...", "Entwischt...", "Gefangen: %s", "einen alten Stiefel", "eine Muschel", "eine Perle", "Preise heute: %u", "Nur zum Spass", "WALDWEG", "SONNENSTRAND", "BERGPFAD", "VULKANHOEHLE", "OEFFNEN", "Noch %u Schritte", "Eine Truhe! %u Muenzen", "Gefunden: %s", "Route geschafft! +10", "Geh mit dem Ball!", "Zutaten fehlen", "Tippen zum Ruehren!", "Fertig: %s!", "AUFGABEN", "SPARSCHWEIN", "TAGEBUCH", "Bett machen", "Hausaufgaben", "Zimmer aufraeumen", "Zu Hause helfen", "Etwas lesen", "Zaehne putzen", "Danke! +4", "Gespart", "RAUSHOLEN", "+%u in %u Tagen", "Spare und bekomme Zinsen", "Heute", "Gestern", "%u Mal gegessen", "%u Spiele gespielt", "%u Mal gebadet", "%u Spaziergaenge", "%u Mal geangelt", "%u Mal gelernt", "%u Aufgaben erledigt", "%u Muenzen verdient", "Wir haben gekocht", "Ein Abenteuer erlebt", "Eine Trophaee!", "Seltene Kapsel!", "Ein ruhiger Tag", "WECKER", "AN", "Guten Morgen!", "Tippen zum Stoppen", "Ab ins Bett!", "20 Fische", "Erste Route", "ZENTRUM",
    // phase 12 games
    "MEMORY", "SIMON SAGT", "FLATTERFLUG", "LABYRINTH", "Finde die Paare", "Wiederhole die Folge", "Pass auf...", "Tippen zum Fliegen", "Kippen oder ziehen",
    // phase 13: hats and the room
    "KOCHMUETZE", "PIRATENHUT", "KATZENOHREN", "HASENOHREN", "KOPFHOERER", "DOKTORHUT", "BLUMENKRANZ", "BAUHELM", "BASKENMUETZE", "WIKINGERHELM", "MOEBEL", "MEIN ZIMMER", "FENSTER", "UHR", "BILD", "REGAL", "SOFA", "FERNSEHER", "AQUARIUM", "BUECHERREGAL", "KAMIN", "TEPPICH", "BLAUE TAPETE", "ROSA TAPETE", "MINZTAPETE",
    // phase 14
    "Ich will %s!", "Ich will spielen!", "Ab in den Park!", "Ich will baden!", "Lass uns angeln!", "Streichel mich!", "Wunsch erfuellt! +5", "Fuehlt sich einsam", "Ein toller Tag!", "Ich hab dich vermisst...", "Alles Gute! +20", "%u Monate!", "Ein Osterei! +5", "Sommerfest: ein Gratis-Eis!", "Charakter: %s", "verspielt", "faul", "verfressen", "schuechtern",
    // phase 15
    "SPIELE", "KOPF", "ARCADE", "BALLONS", "RENNEN", "FANGEN", "BALANCE", "TYPENQUIZ", "SCHIEBEPUZZLE", "BLASEN", "MALEN", "Ballons ja, Bomben nein", "Schuettel oder tippe schnell", "Fangen, Voltobal meiden", "Kippen oder Seite tippen", "Wie effektiv?", "Ordne 1 bis 8", "Tippe gleiche Gruppen", "FERTIG",
    // phase 16
    "OUTFITS", "SONNENBRILLE", "STERNBRILLE", "HERZBRILLE", "MONOKEL", "SCHAL", "FLIEGE", "MEDAILLE", "HALSTUCH", "UMHANG", "RUCKSACK", "ENGELSFLUEGEL", "FLEDERMAUSFLUEGEL", "Sieht toll aus!",
    // phase 17
    "STRAND", "BIBLIOTHEK", "MUSIK", "WETTBEWERB", "SPIELHALLE", "JOBS", "BURG", "Eine Muschel! +1", "Heute keine Muscheln mehr", "Eine Sandburg!", "Heute schon gebaut", "LESEN", "Genug gelesen heute", "Genug Musik heute", "KAMPF", "SIEG", "WIEGENLIED", "MITMACHEN", "Naechste Woche wieder", "Kein Band... +2", "Bronzeband! +5", "Silberband! +10", "Goldband! +20", "Outfit, Bindung, Freude", "CAFE", "BLUMENLADEN", "POST", "3 Std.   +15", "Es arbeitet", "Zurueck um %u:%02u", "Zurueck von der Arbeit! +15", "Morgen wieder", "POST", "Keine Briefe", "Prof. Eich: Super! Hier 5 Muenzen.", "Mauzi: Ein Keks fuer dich, miau!", "Chaneira: Ein Samen fuer deinen Garten.", "Pikachu: Pika pika! (eine Seife)", "Oma: 8 Muenzen zum Sparen.", "Ein Freund: Lass uns spielen!",
    // phase 19
    "QUEST", "Pikachu hat seinen Hut verloren", "Mauzis Mittagessen", "Chaneiras verlorenes Ei", "Ein Lied fuer Pummeluff", "Such im Park", "Such am Strand", "Frag in der Bibliothek", "Angle am See", "Koche ein Gericht", "Esst etwas Selbstgemachtes", "Oeffne einen Abenteuerhalt", "Besuch das Pokemon-Center", "Nimm ein Bad", "Spiel ein Lied", "Spiel ein Spiel", "Lern in der Schule", "Naechster Schritt morgen", "Quest geschafft! Ein Hut", "Suesses oder Saures! Ein Keks", "Ein Gratis-Kakao!", "Ein Jahr zusammen! +50",
    // the photo album
    "ALBUM", "Noch keine Fotos", "Foto im Album!", "FOTO",
    // the Nursery
    "PENSION", "Ein Tag zusammen: 40 Muenzen", "ZWEI DA LASSEN", "Waehle zwei Freunde", "40 ZAHLEN", "Der kann nicht mit", "Bis morgen!", "Komm naechste Woche wieder", "Fertig in", "EI NEHMEN", "Ein Ei! Es kommt als Naechstes", "Dein naechstes Ei", "ZURUECK", "Nochmal tippen zum Zurueckgeben", "Ihr Ei ist jetzt zu Hause", "Aus der Pension",
    // Wi-Fi
    "Kein WLAN: auf der Installationsseite einrichten", "WLAN: verbinde...", "Uhrzeit per WLAN", "WLAN: keine Verbindung",
    // snow from New York
    "In New York schneit es!", "Schau, Schnee aus New York!", "Schnee aus New York: AN", "Schnee aus New York: AUS",
    // restful nights
    "Schlaeft: seine Beduerfnisse ruhen",
    // the try-on mirror
    "ANPROBE", "Wie sehe ich aus?",
    // the arcade
    "PREISE", "REKORDE", "Tickets", "Gewonnen!", "Nicht genug Tickets", "ARCADE-KAPPE", "STERNVISIER", "SPIELAUTOMAT", "WOCHE", "BESTE", "Bronze-Abzeichen!", "Silber-Abzeichen!", "Gold-Abzeichen!",
    // arcade trophies
    "10 Gold-Abzeichen", "Alle 3 Arcade-Preise",
    // what it says
    "Ich habe Hunger!", "Mein Bauch knurrt!", "Essen wir etwas?", "Eine Beere, bitte!", "Da riecht was lecker...", "Ich brauche einen Snack!",
    "Ich brauche ein Bad!", "Baden wir?", "Ich bin ganz schmutzig...", "Seifenblasen, bitte!", "Ich will gut riechen", "Gaehn... so muede.",
    "Ein kleines Nickerchen?", "Mir fallen die Augen zu", "Ich bin muede", "Deckst du mich zu?", "Mir geht es nicht gut...", "Mein Bauch tut weh",
    "Hatschi! Hatschi!", "Gehen wir ins Center?", "Etwas Medizin, bitte", "Ich habe dich vermisst!", "Spielst du mit mir?", "Ich war ganz allein...",
    "Geh nicht weg!", "Kuscheln wir?", "Mir ist langweilig...", "Spielen wir ein Spiel?", "Gehen wir in den Park!", "Lass uns was Lustiges tun",
    "Wettrennen?", "Was fuer ein schoener Tag!", "Ich bin so froh!", "Schoen, dass du da bist", "Schau, was ich kann!", "Heute bin ich mutig",
    "La la la!", "Wir sind ein tolles Team!", "Ich will was lernen", "Ich mag unser Zuhause", "Was machen wir heute?", "Erzaehl mir was!",
    "Ich bin zu allem bereit!", "Ich hab dich lieb!", "Du bist mein bester Freund", "Freunde fuer immer!", "Du kuemmerst dich gut um mich", "Eine Riesenumarmung!",
    "Danke fuer alles", "Mit dir ist alles schoener", "Du bist besonders", "Guten Morgen!", "Gut geschlafen?", "Zeit fuers Fruehstueck!",
    "Ein ganz neuer Tag!", "Zaehne putzen!", "Guten Tag!", "Wie war dein Tag?", "Zeit fuer einen Snack!", "Hast du deine Hausaufgaben?",
    "Guten Abend!", "Bald ist Schlafenszeit", "Was gibt es zum Abendessen?", "Schau, die Sterne", "Das war ein guter Tag", "Traeum was Schoenes...",
    "Es ist schon spaet!", "Der Mond ist schoen", "So ein sonniger Tag!", "Die Sonne tut gut", "Spielen wir draussen?", "Blauer Himmel!",
    "So viele Wolken...", "Die Wolke sieht aus wie ein Pokemon", "Heute ist es wolkig", "Es regnet!", "Tropf, tropf...", "In Pfuetzen springen?",
    "Vergiss den Schirm nicht", "Donner! Bleib bei mir", "Was fuer ein Gewitter!", "Ich habe einen Blitz gesehen!", "Ich sehe nichts!", "Heute ist es neblig",
    "Wie geheimnisvoll...", "Es schneit!", "Bauen wir einen Schneemann!", "Schneeflocken!", "Schneeballschlacht!", "Schnee aus New York!",
    "In New York ist es kalt!", "Ist das heiss!", "Ein Eis, bitte!", "Ich brauche kaltes Wasser", "Brrr, ist das kalt!", "Ich brauche einen Schal",
    "Heisse Schokolade?", "Der Garten ist reif!", "Da ist was gewachsen!", "Lass uns ernten!", "Lernen wir ein bisschen?", "Ich liebe Zahlen",
    "Machen wir das Quiz!", "Ab in die Spielhalle!", "Ich will mehr Tickets!", "Brechen wir einen Rekord?", "Ich will ein Gold-Abzeichen!", "In der Pension wartet ein Ei!",
    "Was wohl schluepft?", "Ein Brief ist da!", "Lesen wir die Post?", "Machen wir mit der Mission weiter?", "Heute ein Schritt mehr!", "Ich habe fleissig gearbeitet!",
    "Ich habe Muenzen verdient", "Wir haben Geburtstag!", "Gibt es Kuchen?", "Danke fuer noch einen Monat", "Suesses oder Saures!", "Buh! Erschreckt?",
    "Ich liebe Kuerbisse", "Frohe Feiertage!", "Gibt es Geschenke?", "Ich liebe die Lichter", "Lass uns Eier suchen!", "Ein buntes Ei!",
    "Ab an den Strand!", "Hurra, Sommer!", "Ich bin immer bei dir", "Ich vertraue dir", "Zusammen sind wir stark", "Du bist der beste Trainer",
    "Ich glaube, ich veraendere mich...", "Mit mir passiert etwas!",
    // phase 31
    "Ein Schaumbad", "Zaehne putzen", "Nickerchen oder Schlaflied", "Etwas Selbstgekochtes", "Gewinne %u Tickets", "Dreh die Kapsel", "Mach das Typenquiz", "Mach beim Wettbewerb mit", "Spaziergang im Park", "Erreiche eine Station", "Angle %u mal", "Spiel am Strand", "Besuch das Center", "Koche ein Gericht", "Pflanzen oder ernten", "Mach %u Aufgaben", "Lies ein Buch", "Lerne ein bisschen", "Spiel ein Lied", "Oeffne einen Brief", "Spar im Sparschwein", "Kauf Essen", "Kauf etwas", "Schmueck sein Zuhause", "Hilf im Haushalt", "Kauf im Laden ein", "Gewinn Arcade-Tickets", "Geh 3000 Schritte", "Bisasams Garten", "Enton hat Kopfweh", "Relaxos grosses Nickerchen", "Evolis Geburtstag", "Schiggy am Strand", "Karpador will fliegen", "Piepis Mondnacht", "Abras erster Schultag",
    // phase 32
    "Kacka fegen", "Nichts zu fegen", "Feg auch die Kacka!", "+%u Sauberkeit", "Alles gefegt!",
    // phase 32b
    "Alles gefegt! +%u Sauberkeit",
    // phase 33
    "Dieses Foto loeschen?", "LOESCHEN", "ALLE LOESCHEN", "Alle ausser Favoriten loeschen?", "Foto geloescht", "Schon %u Favoriten",
    // phase 34
    "%s fordert dich!", "Orden %u von 8", "Top Vier %u von 4", "Das Finale: der Champion", "KAEMPFEN!", "SPAETER",
    // phase 35
    "SCHRITTE", "Heute", "Letzte 7 Tage", "Schritte bis zur Arena", "Abenteuerroute", "Gesamt: %u Schritte", "Bereit! Kampf!", "MDMDFSS", "Alle Arenen besiegt!",
    // phase 36
    "Schritte geschafft! Herausforderung in %u Tagen", "Schritte geschafft! Herausforderung morgen",
    // phase 36
    "Duell in %u Tagen", "Duell morgen",
    // phase 40
    "%s! +%u Muenzen", "Preis bei %u Schritten",
    // phase 43
    "2500 Schritte! Ein Snack", "5000 Schritte! Tolle Runde", "10000 Schritte! Staerker", "Neuling", "Bummler", "Geher", "Wanderer", "Trekker", "Routenguide", "Weltenbummler", "Pfad-Ass", "Legende",
    // phase 44
    "Hohes Gras", "BAELLE", "Pokeball", "Superball", "Hyperball", "Ein wildes %s!", "%s gefangen!", "Es kommt in die Box.", "%s hat sich befreit!", "Team und Box sind voll", "Keine Baelle: im Laden kaufen", "Du findest einen Pokeball!", "Es raschelt im Gras!", "Noch %u Schritte", "Es schlaeft gerade", "Niemand ist im Gras", "Fuer das hohe Gras",
    // phase 45
    "Ausflug", "Auf Ausflug", "Wieder da!", "%s macht einen Ausflug!", "Heute keine Ausfluege mehr", "Zu spaet: morgen wieder", "Zurueckrufen", "Zurueckrufen? Keine Story", "%s war am Fluss", "und fand %u Muenzen!", "%s suchte im Gebuesch", "und fand einen Pokeball!", "%s traf %s", "und sie teilten einen Snack!", "%s lief durch die Stadt", "und bekam einen Keks!", "%s spielte im Garten", "und fand einen Samen!", "%s sprang in Pfuetzen", "und brachte Seife mit!", "%s sah etwas sich bewegen", "im hohen Gras!", "%s stieg auf einen Huegel", "und fand einen Superball!", },
  // ---------------- IT ----------------
  {
    "Si evolve!", "Gnam gnam!", "Gli piace!", "Ha fame!", "Vuole un bagno!",
    "Esausto...", "Triste...", "Un po' cicciotto...", "E' SHINY!!", "E' felice",
    "GRAZIE! Addio", "E' scappato...", "Ciao! Saluta",
    "UOVO", "Uovo leggendario!?", "Uovo raro!", "Tocca l'uovo...", "Si muove!", "Ci siamo quasi!",
    "POKEDEX %u/%u",
    "%s%s Lv.%u",
    "Liberare %s?", "SI", "NO",
    "%u COLPI", "FORZA +%u", "NUOVO RECORD!", "RECORD: %u", "COLPISCI VELOCE!",
    "PUNTI: %u", "Che gioia!", "+felicita",
    "IMPOSTA ORA", "ORA", "MIN", "scorri su: annulla", "Lingua",
    "MEDAGLIA!", "GRANDE!", "SERIE %u GIORNI!",
    "SERIE %u  rec %u", "LEG", "BACCA ???", "BACCA ROSSA", "BACCA BLU", "BACCA VERDE",
    "%s   ETA %lud", "tocca il nome: rinomina",
    "LOTTA", "ATT", "DIF", "VEL", "PES", "ALLENA FORZA",
    "MEDAGLIE %d/%d", "tocca: indietro",
    "NOME:", "tocca per tornare",
    "CIB", "GIO", "ENE", "IGI",
    "REC %u",
    "PROGRESSI", "Lv.%u", "%u min per Lv.%u", "EVOLUZIONE", "Forma finale",
    "Pronto a evolvere!", "Tutto a 40 per evolvere",
    "Evolve tra %u liv.", "Disattenzioni: %u",
    "ON", "OFF",
    "EVOLVI", "%s vuole dirti qualcosa...", "%s si sente abbandonato...",
    "Evolvere?", "Mantieni forma", "Salutare?", "Addio", "Restare insieme",
    "Scegli l'iniziale",
    "Senza sprite", "Caricali sulla SD",
    "PS", "IV %u",
    "MENU", "IMPOSTAZIONI", "CHIUDI", "SQUADRA %u/6", "- vuoto -", "%s entra in squadra!", "Squadra piena: chi sostituire?", "Lasciarlo andare",
    "STATS", "ALLENAMENTO", "FORZA", "VELOCITA", "DIFESA", "Sale da sola se sta bene",
    "MOSSE", "- vuoto -", "Scegli una mossa", "Tocca per cambiare", "POT %u", "STATO",
    "%s vuole imparare", "Non imparare",
    "%s usa %s", "Superefficace!", "Non molto efficace...", "Nessun effetto...",
    "%s ha mancato!", "Brutto colpo!", "%s e\' esausto!", "Si e ferito!",
    "%s: %s", "PARALISI", "SCOTTATURA", "VELENO", "ADDORMENTATO", "CONGELATO", "CONFUSIONE",
    "Hai vinto!", "Hai perso...",
    "%s manda %s", "Vai, %s!", "PALESTRE", "MEDAGLIE %u/8", "ALLENATORE", "VELOCITA +%u", "tocca: cambia avatar", "%u in totale", "NORMALE", "DIFFICILE", "SCELTI %u/%u", "LOTTA", "BLOCCATO", "POKEMON", "%s sconfitto!", "NUOVA MEDAGLIA!", "VOL %u", "BOX %u/%u", "scambia con %s: scegli", "BOX", "RIPORTA", "solo con un uovo", "LOTTA LAN", "CREA", "ENTRA", "ricerca...", "pronto!", "versione diversa", "crea o entra", "rivale: %u mons", "il rivale se n' e andato", "in attesa del rivale...", "ANCORA", "FUGGI", "da quale regione viene l uovo", "%s +%u", "non puo allenarsi oltre", "SCEGLI LA REGIONE", "RITIRARE", "Ritirarla adesso?", "il prossimo evolve un giorno dopo", "evolve un giorno dopo",
   "MANCA PACK", "LIBERA", "via per sempre", "AL GRUPPO", "non entrera nel gruppo", "VOLUME", "LINGUA", "INFO", "LUMINOSITA", "PROVA",
    // Fun Pack
    "SIN/DES", "PIU/MENO", "CHI E?", "BACCHE", "TALPE", "CORDA", "BALLO", "NASCONDINO", "Dove guardera?", "Piu alto o basso?", "PIU", "MENO", "Chi e quel Pokemon?", "Tocca sin/des", "Toccali! Non Voltorb", "Tocca per saltare!", "Tocca a ritmo!", "PERFETTO", "BENE", "MANCATO", "Dove si nasconde?", "GIOIA +%u", "DIFESA +%u", "SI!", "NO...", "Mal di pancia!", "Ha il raffreddore!", "Brontolone...", "Si annoia...", "Zzz... spegni luce!", "Troppo malato!", "Sta meglio!", "Ancora una dose", "Aspetta 10 min", "Ora un bagno!", "Bleah! Non malato", "ALTRI GIOCHI", "MINIGIOCHI",
    // Fun Pack
    "SACCO", "RIFLESSI", "PALLA", "DIVERTIMENTO",
    // coins and daily missions
    "MONETE", "MISSIONI DI OGGI", "Dagli da mangiare %u volte", "La sua bacca preferita", "Gioca %u minigiochi", "Cammina %u passi", "Fagli il bagno", "Accarezzalo %u volte", "Un gioco di forza", "Un gioco di velocita", "Un gioco di difesa", "Fai %u punti", "Dal gioco oggi: %u/%u", "Tutte: +%u extra", "COME GUADAGNARE", "Giocare e curarlo", "+%u ogni 1000 passi", "Prima cura del giorno +%u", "Ogni missione +%u", "Nuova medaglia +%u", "A letto, luce spenta +%u", "Il negozio arriva presto!",
    // shop, bag and hats
    "NEGOZIO", "ZAINO", "CAPPELLI", "BERRETTO", "FIOCCO", "FIORE", "CUFFIA", "DA FESTA", "DI PAGLIA", "DA STREGA", "DI NATALE", "CILINDRO", "CORONA", "COMPRA", "NO", "TUO", "Monete insufficienti", "Comprato!", "INDOSSATO", "TOGLI CAPPELLO", "SPOSTA CAPPELLO", "Ancora niente", "FATTO",
    // toys and decorations
    "GIOCATTOLI", "DECORAZIONI", "FUORI", "PALLONE", "PELUCHE", "PAPERELLA", "CUBETTI", "TAMBURO", "AQUILONE", "PIANTA", "FIORI", "LAMPADA", "LETTO", "REGALO", "ZUCCA",
    // foods
    "CIBO", "PASTI", "DOLCI", "BEVANDE", "MEDICINA", "PANINO", "CURRY", "ZUPPA", "ONIGIRI", "TORTA", "BISCOTTO", "GELATO", "LATTE", "SUCCO", "Finito: vai al negozio", "x%u",
    // bath and sleep menus
    "BAGNO", "DOCCIA", "BAGNOSCHIUMA", "STROFINARE", "SPAZZOLARE", "Strofina le macchie!", "Pulitissimo!", "Serve sapone", "Compra una spazzola", "DORMIRE", "A NANNA", "PISOLINO 30 MIN", "NINNA NANNA", "CURA", "SAPONE", "SPAZZOLA",
    // places, garden, seasonal gifts
    "USCIRE", "PARCO", "CENTRO POKEMON", "ORTO", "PASSEGGIA", "ENTRA", "TORNA", "CURA", "Gratis oggi", "Come nuovo!", "Ha trovato %u monete!", "Ha trovato una bacca!", "Un %s ti saluta!", "Che bella passeggiata", "Troppo stanco", "PIANTA", "RACCOGLI", "Compra semi", "Cresce... %u%%", "SEME", "Raccolto: %s", "Regalo di Halloween!", "Regalo di Natale!", "Buon anno!", "Regalo di San Valentino!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "SFONDI", "PRATO", "SPIAGGIA", "FORESTA", "VULCANO", "MONTAGNA", "NEVE", "CIMITERO", "TROFEI", "Nuovo trofeo! +10", "100 pasti", "50 giochi", "30 bagni", "25 passeggiate", "5 raccolti", "1000 monete", "5 cappelli", "6 decorazioni", "7 giorni di fila", "Livello 20", "10 compiti", "14 spazzolate", "Quiz perfetto", "10 capsule", "CAPSULA", "Ancora %u oggi", "Uscito: %s!", "2 saponi!", "2 semi!", "%u monete!", "Torna domani", "LAVARE I DENTI", "Denti pulitissimi!", "Gia lavati 2 volte oggi", "Tocca per spazzolare", "SCUOLA", "STUDIA", "MATE", "Quanto tempo?", "%u min", "Si studia...", "Ottimo lavoro! +%u", "Ottimo lavoro!", "FERMA", "%u su 10!", "GENITORI", "Ora di dormire", "Giochi al giorno", "NO", "Tocca per cambiare", "E ora di dormire", "Basta giochi per oggi", "Ho fame!", "Voglio un bagno", "Ho sonno...", "Non sto bene...", "Giochiamo!", "Piove!", "Nevica!", "L'orto e pronto!", "Ti voglio bene!", "Che bella giornata!", "Studiamo?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "TORTA DI BACCHE", "STUFATO", "PARFAIT", "LAGO", "AVVENTURA", "CUCINA", "PESCA", "Aspettiamo...", "Abbocca! Tocca!", "Troppo presto...", "E scappato...", "Hai pescato: %s", "uno stivale vecchio", "una conchiglia", "una perla", "Premi oggi: %u", "Solo per divertimento", "SENTIERO DEL BOSCO", "SPIAGGIA SOLEGGIATA", "SENTIERO DI MONTAGNA", "GROTTA VULCANICA", "APRI", "Ancora %u passi", "Un forziere! %u monete", "Ha trovato: %s", "Percorso finito! +10", "Cammina con la palla!", "Mancano ingredienti", "Tocca per mescolare!", "Pronto: %s!", "FACCENDE", "SALVADANAIO", "DIARIO", "Rifare il letto", "Fare i compiti", "Riordinare la stanza", "Aiutare in casa", "Leggere un po", "Lavarsi i denti", "Grazie! +4", "Risparmiato", "PRELEVA", "+%u tra %u giorni", "Risparmia e guadagna interessi", "Oggi", "Ieri", "Ha mangiato %u volte", "Ha giocato %u partite", "Ha fatto %u bagni", "%u passeggiate", "Ha pescato %u volte", "Ha studiato %u volte", "Hai fatto %u faccende", "Ha guadagnato %u monete", "Abbiamo cucinato", "Siamo andati all'avventura", "Ha vinto un trofeo!", "Capsula rara!", "Una giornata tranquilla", "SVEGLIA", "SI", "Buongiorno!", "Tocca per fermare", "Ora di andare a letto!", "20 pesci", "Primo percorso", "CENTRO",
    // phase 12 games
    "MEMORY", "SIMON DICE", "VOLO", "LABIRINTO", "Trova le coppie", "Ripeti la sequenza", "Guarda...", "Tocca per volare", "Inclina o trascina",
    // phase 13: hats and the room
    "CAPPELLO DA CHEF", "CAPPELLO PIRATA", "ORECCHIE DA GATTO", "ORECCHIE DA CONIGLIO", "CUFFIE", "TOCCO", "CORONA DI FIORI", "CASCO DA CANTIERE", "BASCO", "ELMO VICHINGO", "MOBILI", "LA MIA STANZA", "FINESTRA", "OROLOGIO", "QUADRO", "MENSOLA", "DIVANO", "TV", "ACQUARIO", "LIBRERIA", "CAMINO", "TAPPETO", "CARTA BLU", "CARTA ROSA", "CARTA MENTA",
    // phase 14
    "Voglio %s!", "Voglio giocare!", "Andiamo al parco!", "Voglio un bagno!", "Andiamo a pescare!", "Accarezzami!", "Desiderio esaudito! +5", "Si sente solo", "Che giornata!", "Mi sei mancato...", "Buon compleanno! +20", "%u mesi!", "Un uovo di Pasqua! +5", "Festa d'estate: gelato gratis!", "Carattere: %s", "giocherellone", "pigrone", "golosone", "timido",
    // phase 15
    "GIOCHI", "MENTE", "ARCADE", "PALLONCINI", "CORSA", "PRENDI", "EQUILIBRIO", "QUIZ TIPI", "PUZZLE", "BOLLE", "DISEGNO", "Palloncini si, bombe no", "Scuoti o tocca veloce", "Prendili, evita Voltorb", "Inclina o tocca un lato", "Quanto e efficace?", "Ordina da 1 a 8", "Tocca gruppi uguali", "FATTO",
    // phase 16
    "VESTITI", "OCCHIALI DA SOLE", "OCCHIALI STELLA", "OCCHIALI CUORE", "MONOCOLO", "SCIARPA", "PAPILLON", "MEDAGLIA", "BANDANA", "MANTELLO", "ZAINO", "ALI D'ANGELO", "ALI DI PIPISTRELLO", "Che stile!",
    // phase 17
    "SPIAGGIA", "BIBLIOTECA", "MUSICA", "CONCORSO", "SALA GIOCHI", "LAVORETTI", "CASTELLO", "Una conchiglia! +1", "Niente piu conchiglie", "Un castello di sabbia!", "Gia fatto oggi", "LEGGI", "Basta leggere oggi", "Basta musica oggi", "LOTTA", "VITTORIA", "NINNA NANNA", "PARTECIPA", "Torna la prossima settimana", "Niente fiocco... +2", "Fiocco di bronzo! +5", "Fiocco d'argento! +10", "Fiocco d'oro! +20", "Contano vestiti, legame, gioia", "CAFFE", "FIORAIO", "POSTA", "3 h   +15", "E al lavoro", "Torna alle %u:%02u", "Tornato dal lavoro! +15", "Domani ancora", "POSTA", "Nessuna lettera", "Prof. Oak: Ottimo! Ecco 5 monete.", "Meowth: Un biscotto per te, miao!", "Chansey: Un seme per il tuo orto.", "Pikachu: Pika pika! (un sapone)", "La nonna: 8 monete per i risparmi.", "Un amico: Mi manchi, giochiamo!",
    // phase 19
    "MISSIONE", "Pikachu ha perso il cappello", "Il pranzo di Meowth", "L'uovo perso di Chansey", "Una canzone per Jigglypuff", "Cerca al parco", "Cerca in spiaggia", "Chiedi in biblioteca", "Pesca al lago", "Cucina un piatto", "Mangiate qualcosa fatto in casa", "Apri una tappa d'avventura", "Visita il Centro Pokemon", "Fai un bagno", "Metti una canzone", "Fai un gioco", "Studia a scuola", "Prossimo passo domani", "Missione compiuta! Un cappello", "Dolcetto o scherzetto! Un biscotto", "Una cioccolata calda gratis!", "Un anno insieme! +50",
    // the photo album
    "ALBUM", "Ancora nessuna foto", "Foto salvata nell'album!", "FOTO",
    // the Nursery
    "PENSIONE", "Un giorno insieme: 40 monete", "LASCIANE 2", "Scegli due amici", "PAGA 40", "Quello non puo andare", "A domani!", "Torna la prossima settimana", "Pronto tra", "PRENDI UOVO", "Un uovo! Sara il prossimo", "Il tuo prossimo uovo", "RESTITUISCI", "Tocca ancora per restituirlo", "Il loro uovo e a casa", "Dalla pensione",
    // Wi-Fi
    "Niente Wi-Fi: impostalo dalla pagina di installazione", "Wi-Fi: connessione...", "Ora dal Wi-Fi", "Wi-Fi: connessione non riuscita",
    // snow from New York
    "Nevica a New York!", "Guarda, neve da New York!", "Neve da New York: SI", "Neve da New York: NO",
    // restful nights
    "Dorme: i suoi bisogni riposano",
    // the try-on mirror
    "PROVA", "Come sto?",
    // the arcade
    "PREMI", "RECORD", "Biglietti", "L'hai vinto!", "Biglietti insufficienti", "BERRETTO ARCADE", "VISIERA STELLA", "CABINATO", "SETTIMANA", "MIGLIORE", "Medaglia di bronzo!", "Medaglia d'argento!", "Medaglia d'oro!",
    // arcade trophies
    "10 medaglie d'oro", "I 3 premi dell'arcade",
    // what it says
    "Ho fame!", "Mi brontola la pancia!", "Mangiamo qualcosa?", "Una bacca, per favore!", "Che profumino...", "Mi serve uno spuntino!",
    "Mi serve un bagno!", "Facciamo il bagno?", "Sono tutto sporco...", "Bolle, per favore!", "Voglio profumare", "Che sonno...",
    "Un pisolino?", "Mi si chiudono gli occhi", "Sono stanchino", "Mi rimbocchi le coperte?", "Non mi sento bene...", "Mi fa male la pancia",
    "Ecci! Ecci!", "Andiamo al Centro?", "Una medicina, per favore", "Mi sei mancato!", "Giochi con me?", "Ero tutto solo...",
    "Non andare via!", "Una coccola?", "Che noia...", "Facciamo un gioco?", "Andiamo al parco!", "Facciamo qualcosa di bello",
    "Facciamo una gara?", "Che bella giornata!", "Sono felicissimo!", "Che bello averti qui", "Guarda cosa so fare!", "Oggi saro coraggioso",
    "La la la!", "Siamo una bella squadra!", "Voglio imparare qualcosa", "Mi piace casa nostra", "Cosa facciamo oggi?", "Raccontami una storia!",
    "Sono pronto a tutto!", "Ti voglio tanto bene!", "Sei il mio migliore amico", "Amici per sempre!", "Ti prendi cura di me", "Un abbraccio gigante!",
    "Grazie di tutto", "Con te e tutto meglio", "Sei speciale", "Buongiorno!", "Hai dormito bene?", "Ora di colazione!",
    "Un giorno nuovo!", "Laviamoci i denti!", "Buon pomeriggio!", "Com'e andata oggi?", "Ora della merenda!", "Hai fatto i compiti?",
    "Buonasera!", "Quasi ora di dormire", "Cosa c'e per cena?", "Guarda, le stelle", "E stata una bella giornata", "Sogni d'oro...",
    "E molto tardi!", "La luna e bellissima", "Che giornata di sole!", "Il sole e piacevole", "Giochiamo fuori?", "Cielo azzurro!",
    "Quante nuvole...", "Quella nuvola sembra un Pokemon", "Oggi e nuvoloso", "Piove!", "Plic, plic...", "Saltiamo nelle pozzanghere?",
    "Non scordare l'ombrello", "Tuoni! Stammi vicino", "Che temporale!", "Ho visto un fulmine!", "Non vedo niente!", "C'e nebbia oggi",
    "Che mistero...", "Nevica!", "Facciamo un pupazzo?", "Fiocchi di neve!", "Battaglia di palle di neve!", "Neve da New York!",
    "A New York fa freddo!", "Che caldo!", "Un gelato, per favore!", "Mi serve acqua fresca", "Brrr, che freddo!", "Mi serve una sciarpa",
    "Una cioccolata calda?", "L'orto e pronto!", "E cresciuto qualcosa!", "Andiamo a raccogliere!", "Studiamo un po'?", "Adoro i numeri",
    "Facciamo il quiz!", "Andiamo in sala giochi!", "Voglio altri biglietti!", "Battiamo un record?", "Voglio una medaglia d'oro!", "Un uovo aspetta alla pensione!",
    "Cosa uscira dall'uovo?", "E arrivata una lettera!", "Leggiamo la posta?", "Continuiamo la missione?", "Oggi un passo in piu!", "Ho lavorato tanto oggi!",
    "Ho guadagnato monete", "E il nostro compleanno!", "C'e la torta?", "Grazie per un altro mese insieme", "Dolcetto o scherzetto!", "Bu! Ti ho spaventato?",
    "Adoro le zucche", "Buone feste!", "Ci saranno regali?", "Adoro le luci", "Cerchiamo le uova!", "Un uovo colorato!",
    "Andiamo al mare!", "Evviva l'estate!", "Saro sempre con te", "Mi fido di te", "Insieme siamo forti", "Sei il miglior allenatore",
    "Sento che sto per cambiare...", "Mi sta succedendo qualcosa!",
    // phase 31
    "Un bagno di bolle", "Lavagli i denti", "Pisolino o ninna nanna", "Un piatto fatto in casa", "Vinci %u biglietti", "Gira la capsula", "Fai il quiz dei tipi", "Partecipa alla gara", "Passeggia al parco", "Raggiungi una tappa", "Pesca %u volte", "Gioca in spiaggia", "Visita il Centro", "Cucina un piatto", "Pianta o raccogli", "Fai %u faccende", "Leggi un libro", "Studia un po", "Metti una canzone", "Apri una lettera", "Risparmia nel salvadanaio", "Compra da mangiare", "Compra qualcosa", "Decora la sua casa", "Aiuta in casa", "Compra al negozio", "Vinci biglietti", "Cammina 3000 passi", "L'orto di Bulbasaur", "Il mal di testa di Psyduck", "Il pisolino di Snorlax", "La festa di Eevee", "Squirtle va al mare", "Magikarp vuole volare", "La notte di luna di Clefairy", "Abra va a scuola",
    // phase 32
    "Spazza la cacca", "Niente da spazzare", "Spazza anche la cacca!", "+%u di pulizia", "Tutto spazzato!",
    // phase 32b
    "Tutto spazzato! +%u di pulizia",
    // phase 33
    "Cancellare questa foto?", "CANCELLA", "CANCELLA TUTTE", "Cancellare tutte tranne le preferite?", "Foto cancellata", "Hai gia %u preferite",
    // phase 34
    "%s ti sfida!", "Medaglia %u di 8", "Superquattro %u di 4", "La finale: il Campione", "LOTTA!", "DOPO",
    // phase 35
    "PASSI", "Oggi", "Ultimi 7 giorni", "Passi per la palestra", "Percorso avventura", "Totale: %u passi", "Pronto! Si lotta!", "LMMGVSD", "Tutte le palestre vinte!",
    // phase 36
    "Passi fatti! La sfida arriva tra %u giorni", "Passi fatti! La sfida arriva domani",
    // phase 36
    "Sfida tra %u giorni", "Sfida domani",
    // phase 40
    "%s! +%u monete", "Premio a %u passi",
    // phase 43
    "2500 passi! Uno spuntino", "5000 passi! Bella passeggiata", "10000 passi! Piu forte", "Principiante", "Passeggiatore", "Camminatore", "Escursionista", "Trekker", "Guida", "Giramondo", "Asso dei sentieri", "Leggenda",
    // phase 44
    "Erba alta", "BALL", "Poke Ball", "Mega Ball", "Ultra Ball", "Un %s selvatico!", "Hai preso %s!", "Va nel box.", "%s si e liberato!", "Squadra e box pieni", "Niente ball: vai al negozio", "Hai trovato una Poke Ball!", "Qualcosa si muove nell'erba!", "Ancora %u passi", "Ora sta dormendo", "Nessuno nell'erba", "Da usare nell'erba alta",
    // phase 45
    "Gita", "In gita", "Di ritorno!", "%s parte per una gita!", "Niente piu gite oggi", "Troppo tardi: a domani", "Richiama", "Richiamarlo? Niente storia", "%s e andato al fiume", "e ha trovato %u monete!", "%s ha frugato un cespuglio", "e ha trovato una Poke Ball!", "%s ha incontrato %s", "e hanno fatto merenda!", "%s ha girato in citta", "e ha avuto un biscotto!", "%s ha giocato in giardino", "e ha trovato un seme!", "%s ha saltato nelle pozzanghere", "e ha portato del sapone!", "%s ha visto muoversi qualcosa", "nell'erba alta!", "%s e salito su una collina", "e ha trovato una Mega Ball!", },
  // ---------------- PT ----------------
  {
    "Evoluindo!", "Nham nham!", "Ele gosta!", "Esta com fome!", "Precisa de banho!",
    "Exausto...", "Triste...", "Um pouco gordinho...", "E SHINY!!", "Esta feliz",
    "OBRIGADO! Adeus", "Fugiu...", "Tchau! Acena",
    "OVO", "Ovo lendario!?", "Ovo raro!", "Toque no ovo...", "Mexe-se!", "Quase la!",
    "POKEDEX %u/%u",
    "%s%s Niv.%u",
    "Soltar %s?", "SIM", "NAO",
    "%u GOLPES", "FORCA +%u", "NOVO RECORDE!", "RECORDE: %u", "BATA RAPIDO!",
    "PONTOS: %u", "Que alegria!", "+alegria",
    "AJUSTAR HORA", "HORA", "MIN", "deslize cima: cancelar", "Idioma",
    "MEDALHA!", "OTIMO!", "%u DIAS SEGUIDOS!",
    "SEQ %u  rec %u", "LACO", "BAGA ???", "BAGA VERMELHA", "BAGA AZUL", "BAGA VERDE",
    "%s   IDADE %lud", "toque no nome: renomear",
    "COMBATE", "ATQ", "DEF", "VEL", "PES", "TREINAR FORCA",
    "MEDALHAS %d/%d", "toque: voltar",
    "NOME:", "toque para voltar",
    "COM", "ALE", "ENE", "HIG",
    "REC %u",
    "PROGRESSO", "Niv.%u", "%u min para Niv.%u", "EVOLUCAO", "Forma final",
    "Pronto a evoluir!", "Tudo a 40 para evoluir",
    "Evolui em %u niv.", "Descuidos: %u",
    "ON", "OFF",
    "EVOLUIR", "%s quer dizer-te algo...", "%s sente-se abandonado...",
    "Evoluir?", "Manter forma", "Despedir?", "Adeus", "Ficar juntos",
    "Escolhe o inicial",
    "Sem sprites", "Carrega-os no SD",
    "PS", "IV %u",
    "MENU", "AJUSTES", "FECHAR", "EQUIPA %u/6", "- vazio -", "%s junta-se a equipa!", "Equipa cheia: quem substituir?", "Deixa-lo ir",
    "STATS", "TREINO", "FORCA", "VELOCIDADE", "DEFESA", "Sobe sozinha se estiver bem",
    "GOLPES", "- vazio -", "Escolhe um golpe", "Toca para mudar", "POT %u", "ESTADO",
    "%s quer aprender", "Nao aprender",
    "%s usa %s", "Super eficaz!", "Pouco eficaz...", "Nao teve efeito...",
    "%s falhou!", "Acerto critico!", "%s desmaiou!", "Feriu-se!",
    "%s: %s", "PARALISIA", "QUEIMADURA", "VENENO", "A DORMIR", "CONGELADO", "CONFUSAO",
    "Ganhaste!", "Perdeste...",
    "%s envia %s", "Vai, %s!", "GINASIOS", "MEDALHAS %u/8", "TREINADOR", "VELOCIDADE +%u", "toca: mudar avatar", "%u no total", "NORMAL", "DIFICIL", "ESCOLHIDOS %u/%u", "LUTAR", "BLOQUEADO", "POKEMON", "%s derrotado!", "NOVA MEDALHA!", "VOL %u", "CAIXA %u/%u", "trocar com %s: escolhe", "CAIXA", "TRAZER", "so com um ovo", "COMBATE LAN", "CRIAR", "ENTRAR", "a procurar...", "pronto!", "versao diferente", "criar ou entrar", "rival: %u mons", "o rival saiu", "a esperar pelo rival...", "OUTRA VEZ", "FUGIR", "de que regiao vem o ovo", "%s +%u", "ja nao pode treinar mais", "ESCOLHE A REGIAO", "REFORMAR", "Reformar agora?", "o proximo evolui um dia depois", "evolui um dia depois",
   "FALTA PACK", "SOLTAR", "vai para sempre", "A EQUIPA", "nao entrara na equipa", "VOLUME", "IDIOMA", "INFO", "BRILHO", "TESTE",
    // Fun Pack
    "ESQ/DIR", "MAIS/MENOS", "QUEM E?", "BAGAS", "TOUPEIRAS", "CORDA", "DANCA", "ESCONDE", "Para onde olha?", "Maior ou menor?", "MAIOR", "MENOR", "Quem e esse Pokemon?", "Toque esq/dir", "Toque! Nao o Voltorb", "Toque para pular!", "Toque no ritmo!", "PERFEITO", "BOM", "ERROU", "Onde se escondeu?", "ALEGRIA +%u", "DEFESA +%u", "ISSO!", "NAO...", "Dor de barriga!", "Esta resfriado!", "Rabugento...", "Entediado...", "Zzz... apague a luz!", "Doente demais!", "Melhorou!", "Mais uma dose", "Espere 10 min", "Agora um banho!", "Eca! Nao esta doente", "MAIS JOGOS", "MINIJOGOS",
    // Fun Pack
    "SACO", "REFLEXOS", "BOLA", "DIVERSAO",
    // coins and daily missions
    "MOEDAS", "MISSOES DE HOJE", "Alimente %u vezes", "Sua baga favorita", "Jogue %u minijogos", "Ande %u passos", "De um banho", "Faca carinho %u vezes", "Um jogo de forca", "Um jogo de velocidade", "Um jogo de defesa", "Faca %u pontos", "Jogando hoje: %u/%u", "Todas: +%u extra", "COMO GANHAR", "Brincar e cuidar", "+%u a cada 1000 passos", "Primeiro cuidado do dia +%u", "Cada missao +%u", "Insignia nova +%u", "Dormir sem luz +%u", "A loja chega em breve!",
    // shop, bag and hats
    "LOJA", "MOCHILA", "CHAPEUS", "BONE", "LACO", "FLOR", "GORRO", "DE FESTA", "DE PALHA", "DE BRUXA", "DE NATAL", "CARTOLA", "COROA", "COMPRAR", "NAO", "SEU", "Moedas insuficientes", "Comprado!", "USANDO", "TIRAR CHAPEU", "MOVER CHAPEU", "Nada ainda", "PRONTO",
    // toys and decorations
    "BRINQUEDOS", "DECORACAO", "FORA", "BOLA", "PELUCIA", "PATINHO", "BLOCOS", "TAMBOR", "PIPA", "PLANTA", "FLORES", "LUMINARIA", "CAMA", "PRESENTE", "ABOBORA",
    // foods
    "COMIDA", "REFEICOES", "DOCES", "BEBIDAS", "REMEDIO", "SANDUICHE", "CURRY", "SOPA", "BOLINHO", "BOLO", "BISCOITO", "SORVETE", "LEITE", "SUCO", "Acabou: va a loja", "x%u",
    // bath and sleep menus
    "BANHO", "DUCHA RAPIDA", "BANHO DE ESPUMA", "ESFREGAR", "ESCOVAR", "Esfregue a sujeira!", "Limpinho!", "Precisa de sabao", "Compre uma escova", "DORMIR", "HORA DE DORMIR", "SONECA 30 MIN", "CANCAO DE NINAR", "CUIDADOS", "SABAO", "ESCOVA",
    // places, garden, seasonal gifts
    "SAIR", "PARQUE", "CENTRO POKEMON", "HORTA", "PASSEAR", "ENTRAR", "VOLTAR", "CURAR", "Gratis hoje", "Novinho em folha!", "Achou %u moedas!", "Achou uma baga!", "Um %s diz oi!", "Que passeio bom", "Cansado demais", "PLANTAR", "COLHER", "Compre sementes", "Crescendo... %u%%", "SEMENTE", "Colheu: %s", "Presente de Halloween!", "Presente de Natal!", "Feliz Ano Novo!", "Presente de Namorados!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "CENARIOS", "CAMPINA", "PRAIA", "FLORESTA", "VULCAO", "MONTANHA", "NEVE", "CEMITERIO", "TROFEUS", "Novo trofeu! +10", "100 refeicoes", "50 jogos", "30 banhos", "25 passeios", "5 colheitas", "1000 moedas", "5 chapeus", "6 enfeites", "7 dias seguidos", "Nivel 20", "10 licoes", "14 escovadas", "Quiz perfeito", "10 capsulas", "CAPSULA", "Restam %u hoje", "Saiu: %s!", "2 sabonetes!", "2 sementes!", "%u moedas!", "Volte amanha", "ESCOVAR DENTES", "Dentes limpinhos!", "Ja escovou 2 vezes hoje", "Toque para escovar", "ESCOLA", "ESTUDAR", "CONTAS", "Quanto tempo?", "%u min", "Estudando...", "Bom trabalho! +%u", "Bom trabalho!", "PARAR", "%u de 10!", "PAIS", "Hora de dormir", "Jogos por dia", "NAO", "Toque para mudar", "Hora de dormir", "Chega de jogos hoje", "Estou com fome!", "Quero um banho", "Estou com sono...", "Nao me sinto bem...", "Vamos brincar!", "Esta chovendo!", "Esta nevando!", "A horta esta pronta!", "Te amo!", "Que dia bom!", "Vamos estudar?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "TORTA DE BAGAS", "ENSOPADO", "PARFAIT", "LAGO", "AVENTURA", "COZINHA", "PESCAR", "Esperando...", "Mordeu! Toque ja!", "Cedo demais...", "Escapou...", "Pescou: %s", "uma bota velha", "uma concha", "uma perola", "Premios hoje: %u", "So por diversao", "TRILHA DA FLORESTA", "PRAIA ENSOLARADA", "TRILHA DA MONTANHA", "CAVERNA VULCANICA", "ABRIR", "Faltam %u passos", "Um bau! %u moedas", "Achou: %s", "Rota completa! +10", "Ande com a bola!", "Faltam ingredientes", "Toque para mexer!", "Pronto: %s!", "TAREFAS", "COFRINHO", "DIARIO", "Arrumar a cama", "Fazer a licao", "Arrumar o quarto", "Ajudar em casa", "Ler um pouco", "Escovar os dentes", "Obrigado! +4", "Guardado", "TIRAR", "+%u em %u dias", "Guarde e ganhe juros", "Hoje", "Ontem", "Comeu %u vezes", "Jogou %u jogos", "Tomou %u banhos", "Passeou %u vezes", "Pescou %u vezes", "Estudou %u vezes", "Voce fez %u tarefas", "Ganhou %u moedas", "Cozinhamos juntos", "Fomos numa aventura", "Ganhou um trofeu!", "Capsula rara!", "Um dia tranquilo", "ALARME", "SIM", "Bom dia!", "Toque para parar", "Hora de dormir!", "20 peixes", "Primeira rota", "CENTRO",
    // phase 12 games
    "MEMORIA", "SIMON DIZ", "VOO", "LABIRINTO", "Ache os pares", "Repita a sequencia", "Olhe...", "Toque para voar", "Incline ou arraste",
    // phase 13: hats and the room
    "CHAPEU DE CHEF", "CHAPEU PIRATA", "ORELHAS DE GATO", "ORELHAS DE COELHO", "FONES", "CAPELO", "COROA DE FLORES", "CAPACETE", "BOINA", "CAPACETE VIKING", "MOVEIS", "MEU QUARTO", "JANELA", "RELOGIO", "QUADRO", "PRATELEIRA", "SOFA", "TV", "AQUARIO", "ESTANTE DE LIVROS", "LAREIRA", "TAPETE", "PAPEL AZUL", "PAPEL ROSA", "PAPEL MENTA",
    // phase 14
    "Quero %s!", "Quero brincar!", "Vamos ao parque!", "Quero um banho!", "Vamos pescar!", "Faz carinho!", "Desejo realizado! +5", "Sente saudade", "Que dia otimo!", "Senti sua falta...", "Feliz aniversario! +20", "%u meses!", "Um ovo de Pascoa! +5", "Festa de verao: sorvete gratis!", "Personalidade: %s", "brincalhona", "preguicosa", "comilona", "timida",
    // phase 15
    "JOGOS", "MENTE", "ARCADE", "BALOES", "CORRIDA", "PEGA", "EQUILIBRIO", "QUIZ DE TIPOS", "QUEBRA-CABECA", "BOLHAS", "DESENHO", "Estoure baloes, nao bombas", "Sacuda ou toque rapido", "Pegue, desvie de Voltorb", "Incline ou toque um lado", "Quao eficaz e?", "Ordene de 1 a 8", "Toque grupos iguais", "PRONTO",
    // phase 16
    "ROUPAS", "OCULOS DE SOL", "OCULOS ESTRELA", "OCULOS CORACAO", "MONOCULO", "CACHECOL", "GRAVATA BORBOLETA", "MEDALHA", "BANDANA", "CAPA", "MOCHILA", "ASAS DE ANJO", "ASAS DE MORCEGO", "Que estilo!",
    // phase 17
    "PRAIA", "BIBLIOTECA", "MUSICA", "CONCURSO", "FLIPERAMA", "TRABALHOS", "CASTELO", "Uma concha! +1", "Sem mais conchas hoje", "Um castelo de areia!", "Ja fez um hoje", "LER", "Chega de leitura hoje", "Chega de musica hoje", "BATALHA", "VITORIA", "CANCAO DE NINAR", "PARTICIPAR", "Volte semana que vem", "Sem fita desta vez... +2", "Fita de bronze! +5", "Fita de prata! +10", "Fita de ouro! +20", "Roupa, laco e alegria contam", "CAFETERIA", "FLORICULTURA", "CORREIO", "3 h   +15", "Esta trabalhando", "Volta as %u:%02u", "Voltou do trabalho! +15", "Amanha tem mais", "CORREIO", "Sem cartas", "Prof. Carvalho: Muito bem! 5 moedas.", "Meowth: Um biscoito pra voce, miau!", "Chansey: Uma semente para sua horta.", "Pikachu: Pika pika! (um sabonete)", "Vovo: 8 moedas para guardar.", "Um amigo: Saudade, vamos brincar!",
    // phase 19
    "MISSAO", "Pikachu perdeu o chapeu", "O almoco de Meowth", "O ovo perdido de Chansey", "Uma cancao para Jigglypuff", "Procure no parque", "Procure na praia", "Pergunte na biblioteca", "Pesque no lago", "Cozinhe um prato", "Comam algo caseiro", "Abra uma parada de aventura", "Visite o Centro Pokemon", "Tome um banho", "Toque uma musica", "Jogue um jogo", "Estude na escola", "Proximo passo amanha", "Missao completa! Um chapeu", "Gostosuras! Um biscoito", "Chocolate quente gratis!", "Um ano juntos! +50",
    // the photo album
    "ALBUM", "Ainda sem fotos", "Foto salva no album!", "FOTO",
    // the Nursery
    "CRECHE", "Um dia juntos: 40 moedas", "DEIXAR DOIS", "Escolha dois amigos", "PAGAR 40", "Esse nao pode ir", "Ate amanha!", "Volte na proxima semana", "Pronto em", "PEGAR OVO", "Um ovo! Sera o proximo", "Seu proximo ovo", "DEVOLVER", "Toque de novo para devolver", "O ovo deles esta em casa", "Da creche",
    // Wi-Fi
    "Sem Wi-Fi: configure na pagina de instalacao", "Wi-Fi: conectando...", "Hora pelo Wi-Fi", "Wi-Fi: nao conectou",
    // snow from New York
    "Esta nevando em Nova York!", "Olha, neve de Nova York!", "Neve de Nova York: SIM", "Neve de Nova York: NAO",
    // restful nights
    "Dormindo: as necessidades descansam",
    // the try-on mirror
    "PROVAR", "Como estou?",
    // the arcade
    "PREMIOS", "RECORDES", "Bilhetes", "Voce ganhou!", "Faltam bilhetes", "BONE ARCADE", "VISEIRA ESTRELA", "FLIPERAMA", "SEMANA", "MELHOR", "Medalha de bronze!", "Medalha de prata!", "Medalha de ouro!",
    // arcade trophies
    "10 medalhas de ouro", "Os 3 premios do arcade",
    // what it says
    "Estou com fome!", "Minha barriga ronca!", "Vamos comer algo?", "Uma fruta, por favor!", "Que cheirinho bom...", "Preciso de um lanche!",
    "Preciso de um banho!", "Vamos tomar banho?", "Estou todo sujo...", "Bolhas, por favor!", "Quero ficar cheiroso", "Que sono...",
    "Um cochilo?", "Meus olhos estao fechando", "Estou cansadinho", "Me cobre?", "Nao me sinto bem...", "Minha barriga doi",
    "Atchim! Atchim!", "Vamos ao Centro?", "Um remedio, por favor", "Senti sua falta!", "Brinca comigo?", "Eu estava sozinho...",
    "Nao va embora!", "Me da um carinho?", "Que tedio...", "Vamos jogar?", "Vamos ao parque!", "Vamos fazer algo legal",
    "Apostamos corrida?", "Que dia lindo!", "Estou tao feliz!", "Que bom que voce esta aqui", "Olha o que eu sei fazer!", "Hoje vou ser corajoso",
    "La la la!", "Somos um otimo time!", "Quero aprender algo", "Gosto da nossa casa", "O que vamos fazer hoje?", "Me conta uma historia!",
    "Estou pronto pra tudo!", "Te amo muito!", "Voce e meu melhor amigo", "Amigos para sempre!", "Voce cuida bem de mim", "Um abraco gigante!",
    "Obrigado por tudo", "Com voce tudo e melhor", "Voce e especial", "Bom dia!", "Dormiu bem?", "Hora do cafe!",
    "Um novo dia!", "Vamos escovar os dentes!", "Boa tarde!", "Como foi seu dia?", "Hora do lanche!", "Fez a licao de casa?",
    "Boa noite!", "Quase hora de dormir", "O que tem pro jantar?", "Olha, as estrelas", "Foi um bom dia", "Bons sonhos...",
    "Ja esta tarde!", "A lua esta linda", "Que dia de sol!", "O sol esta gostoso", "Vamos brincar la fora?", "Ceu azul!",
    "Quantas nuvens...", "Aquela nuvem parece um Pokemon", "Esta nublado hoje", "Esta chovendo!", "Ping, ping...", "Pular nas pocas?",
    "Nao esqueca o guarda-chuva", "Trovao! Fica perto de mim", "Que tempestade!", "Vi um raio!", "Nao vejo nada!", "Tem neblina hoje",
    "Que misterioso...", "Esta nevando!", "Vamos fazer um boneco?", "Flocos de neve!", "Guerra de bolas de neve!", "Neve de Nova York!",
    "Esta frio em Nova York!", "Que calor!", "Um sorvete, por favor!", "Preciso de agua gelada", "Brrr, que frio!", "Preciso de um cachecol",
    "Um chocolate quente?", "A horta esta pronta!", "Algo gostoso cresceu!", "Vamos colher!", "Vamos estudar um pouco?", "Adoro numeros",
    "Vamos fazer o quiz!", "Vamos ao fliperama!", "Quero mais bilhetes!", "Vamos bater um recorde?", "Quero uma medalha de ouro!", "Um ovo espera na creche!",
    "O que vai nascer?", "Chegou uma carta!", "Vamos ler a carta?", "Vamos continuar a missao?", "Mais um passo hoje!", "Trabalhei muito hoje!",
    "Ganhei moedas pra gente", "E nosso aniversario!", "Tem bolo?", "Obrigado por mais um mes juntos", "Gostosuras ou travessuras!", "Bu! Te assustei?",
    "Adoro aboboras", "Boas festas!", "Vai ter presentes?", "Adoro as luzes", "Vamos cacar ovos!", "Um ovo colorido!",
    "Vamos a praia!", "Viva o verao!", "Sempre estarei com voce", "Confio em voce", "Juntos somos fortes", "Voce e o melhor treinador",
    "Sinto que vou mudar...", "Algo esta acontecendo comigo!",
    // phase 31
    "Um banho de espuma", "Escove os dentes", "Soneca ou cancao de ninar", "Coma algo caseiro", "Ganhe %u tickets", "Gire a capsula", "Faca o quiz de tipos", "Entre no concurso", "Passeie no parque", "Chegue a uma parada", "Pesque %u vezes", "Brinque na praia", "Visite o Centro", "Cozinhe um prato", "Plante ou colha", "Faca %u tarefas", "Leia um livro", "Estude um pouco", "Toque uma musica", "Abra uma carta", "Guarde no cofrinho", "Compre comida", "Compre algo", "Decore a casa", "Ajude numa tarefa", "Compre na loja", "Ganhe tickets no arcade", "Ande 3000 passos", "A horta do Bulbasaur", "A dor de cabeca do Psyduck", "A grande soneca do Snorlax", "A festa do Eevee", "Squirtle vai a praia", "Magikarp quer voar", "A noite de lua da Clefairy", "Abra vai a escola",
    // phase 32
    "Varrer o coco", "Nada para varrer", "Varra o coco tambem!", "+%u de limpeza", "Tudo varrido!",
    // phase 32b
    "Tudo varrido! +%u de limpeza",
    // phase 33
    "Apagar esta foto?", "APAGAR", "APAGAR TODAS", "Apagar todas menos as favoritas?", "Foto apagada", "Ja tem %u favoritas",
    // phase 34
    "%s te desafia!", "Insignia %u de 8", "Elite Quatro %u de 4", "A final: o Campeao", "LUTAR!", "DEPOIS",
    // phase 35
    "PASSOS", "Hoje", "Ultimos 7 dias", "Passos ate o ginasio", "Rota de aventura", "Total: %u passos", "Pronto! A lutar!", "STQQSSD", "Todos os ginasios vencidos!",
    // phase 36
    "Passos prontos! O desafio chega em %u dias", "Passos prontos! O desafio chega amanha",
    // phase 36
    "Desafio em %u dias", "Desafio amanha",
    // phase 40
    "%s! +%u moedas", "Premio aos %u passos",
    // phase 43
    "2500 passos! Um lanche", "5000 passos! Belo passeio", "10000 passos! Mais forte", "Aprendiz", "Passeante", "Caminhante", "Excursionista", "Trilheiro", "Guia de rota", "Globetrotter", "As da trilha", "Lenda",
    // phase 44
    "Mato alto", "BOLAS", "Poke Bola", "Grande Bola", "Ultra Bola", "Um %s selvagem!", "Pegou %s!", "Foi para a caixa.", "%s escapou!", "Equipe e caixa cheias", "Sem bolas: compre na loja", "Achou uma Poke Bola!", "Algo se mexe no mato!", "Ande mais %u passos", "Esta dormindo agora", "Ninguem no mato", "Use no mato alto",
    // phase 45
    "Passeio", "Em passeio", "De volta!", "%s saiu em passeio!", "Sem passeios hoje", "Tarde: so amanha", "Chamar", "Chamar? Sem historia", "%s foi ao rio", "e achou %u moedas!", "%s olhou num arbusto", "e achou uma Poke Bola!", "%s conheceu %s", "e dividiram um lanche!", "%s andou pela cidade", "e ganhou um biscoito!", "%s brincou no jardim", "e achou uma semente!", "%s pulou em pocas", "e trouxe sabao!", "%s viu algo se mexer", "no mato alto!", "%s subiu um morro", "e achou uma Grande Bola!", },
  // ---------------- ZH (simplified) ----------------
  // The only rows allowed non-ASCII: glyphs come from cjkfont.h (tools/gen_cjk.py).
  {
    "进化中!", "好好吃!", "它很喜欢!", "它饿了!", "需要洗澡!",
    "累坏了...", "好难过...", "有点胖...", "是闪光的!!", "很开心",
    "谢谢! 再见", "它跑掉了...", "再见! 挥手告别...",
    "蛋", "传说之蛋!?", "稀有蛋!", "点一点蛋...", "它在动!", "快好了!",
    "图鉴 %u/%u",
    "%s%s Lv.%u",
    "放生 %s?", "是", "否",
    "%u 次", "力量 +%u", "新纪录!", "最高: %u", "快打!",
    "得分: %u", "太好玩了!", "+开心",
    "设置时间", "时", "分", "上滑: 取消", "语言",
    "奖章!", "太棒了!", "连续 %u 天!",
    "连续 %u  最高 %u", "羁绊", "树果 ???", "红树果", "蓝树果", "绿树果",
    "%s   年龄 %lu天", "点名字: 改名",
    "战斗", "攻击", "防御", "速度", "体重", "训练力量",
    "奖章 %d/%d", "点击: 返回",
    "名字:", "点击返回",
    "饱食", "快乐", "精力", "清洁",
    "最高 %u",
    "进度", "Lv.%u", "%u 分钟到 Lv.%u", "进化", "最终形态",
    "可以进化了!", "所有需求>=40才能进化",
    "%u 级后进化", "失误: %u",
    "开", "关",
    "进化!", "%s 有话想说...", "%s 觉得被抛弃了...",
    "进化吗?", "保持原样", "告别吗?", "告别", "留下来",
    "选择你的伙伴",
    "没有图像", "请放入SD卡",
    "体力", "IV %u",
    "菜单", "设置", "关闭", "队伍 %u/6", "- 空 -", "%s 加入了队伍!", "队伍已满: 选择替换对象", "算了",
    "能力", "训练", "力量", "速度", "防御", "开心时会自己成长",
    "招式", "- 空 -", "选择招式", "点击格子更换", "威力 %u", "变化",
    "%s 想学习", "不学习",
    "%s 使用了 %s", "效果拔群!", "效果不太好...", "没有效果...",
    "%s 没打中!", "击中要害!", "%s 倒下了!", "它伤到了自己!",
    "%s: %s", "麻痹", "灼伤", "中毒", "睡眠", "冰冻", "混乱",
    "你赢了!", "你输了...",
    "%s 派出了 %s", "上吧, %s!", "道馆", "徽章 %u/8", "训练家",
    "速度 +%u", "点击: 换头像", "共获得 %u", "简单", "困难", "已选 %u/%u", "战斗", "未解锁", "宝可梦", "%s 被打败了!", "新徽章!", "音量 %u", "箱子 %u/%u", "与 %s 交换: 选择格子", "箱子", "带回来", "仅在有蛋时可用", "局域网对战", "主机", "加入", "搜索中...", "准备好了!", "版本不同", "主机或加入", "对手: %u 只", "对手离开了", "等待对手...", "再来", "逃跑", "这颗蛋的来源", "%s +%u", "已训练到极限", "选择地区", "退役", "要提前退役吗?", "下一只晚一天进化", "晚一天进化",
    "需要图包", "放生", "永远离开", "入队", "它不会加入你的队伍", "音量", "语言", "关于", "亮度", "测试",
    // Fun Pack
    "左还是右", "大还是小", "我是谁", "接树果", "打地鼠", "跳绳", "跳舞", "捉迷藏", "它会看哪边?", "更大还是更小?", "更大", "更小", "这是哪只宝可梦?", "点左右移动", "快点! 别碰霹雳电球", "点击跳跃!", "跟着节拍点击!", "完美", "不错", "失误", "它躲在哪里?", "快乐 +%u", "防御 +%u", "答对了!", "错了...", "肚子疼!", "感冒了!", "心情不好...", "好无聊...", "Zzz... 请关灯!", "生病了不能玩!", "好多了!", "稍后再吃一次", "等10分钟", "现在洗个澡!", "好苦! 它没生病", "更多游戏", "小游戏",
    // Fun Pack
    "沙袋", "反应测试", "顶球", "娱乐",
    // coins and daily missions
    "金币", "今日任务", "喂食%u次", "喂它最爱的树果", "玩%u个小游戏", "走%u步", "给它洗澡", "抚摸%u次", "玩一个力量游戏", "玩一个速度游戏", "玩一个防御游戏", "得到%u分", "今日游戏: %u/%u", "全部完成: 额外+%u", "如何赚取", "玩耍和照顾", "每1000步 +%u", "每天第一次照顾 +%u", "每个任务 +%u", "新徽章 +%u", "关灯睡觉 +%u", "商店即将开放!",
    // shop, bag and hats
    "商店", "背包", "帽子", "鸭舌帽", "蝴蝶结", "花朵", "毛线帽", "派对帽", "草帽", "女巫帽", "圣诞帽", "礼帽", "皇冠", "购买", "不要", "已拥有", "金币不够", "买到了!", "戴着", "摘下帽子", "调整帽子", "还什么都没有", "完成",
    // toys and decorations
    "玩具", "装饰", "已摆放", "沙滩球", "毛绒玩具", "小黄鸭", "积木", "鼓", "风筝", "盆栽", "花束", "台灯", "小床", "礼物", "南瓜",
    // foods
    "食物", "正餐", "甜点", "饮料", "药", "三明治", "咖喱饭", "汤", "饭团", "蛋糕", "饼干", "冰淇淋", "牛奶", "果汁", "没有了: 去商店买", "x%u",
    // bath and sleep menus
    "洗澡", "冲澡", "泡泡浴", "搓洗", "梳毛", "把脏东西搓掉!", "干干净净!", "需要肥皂", "先买梳子", "睡觉", "去睡觉", "午睡30分钟", "摇篮曲", "护理", "肥皂", "梳子",
    // places, garden, seasonal gifts
    "出门", "公园", "宝可梦中心", "菜园", "散步", "进去", "回家", "治疗", "今天免费", "完全恢复了!", "找到了%u枚金币!", "找到了树果!", "%s 打招呼!", "散步真开心", "太累了", "种下", "收获", "去买种子", "生长中... %u%%", "种子", "收获了: %s", "万圣节礼物!", "圣诞礼物!", "新年快乐!", "情人节礼物!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "场景", "草原", "海滩", "森林", "火山", "高山", "雪原", "墓地", "奖杯", "新奖杯! +10", "吃饭100次", "游戏50次", "洗澡30次", "散步25次", "收获5次", "1000枚金币", "5顶帽子", "6件装饰", "连续7天", "等级20", "学习10次", "刷牙14次", "测验满分", "扭蛋10次", "扭蛋", "今天还剩%u个", "抽到了: %s!", "2块肥皂!", "2颗种子!", "%u枚金币!", "明天再来", "刷牙", "牙齿亮晶晶!", "今天刷过两次了", "点击刷牙", "学校", "学习", "算术", "学多久?", "%u分钟", "学习中...", "做得好! +%u", "做得好!", "停止", "答对%u题!", "家长", "睡觉时间", "每天游戏", "关", "点击更改", "该睡觉了", "今天玩够了", "我饿了!", "我想洗澡", "我困了...", "我不舒服...", "一起玩吧!", "下雨了!", "下雪了!", "菜园可以收获了!", "我爱你!", "今天真开心!", "我们学习吧?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "树果派", "炖菜", "芭菲", "湖", "冒险", "厨房", "钓鱼", "等待中...", "上钩了! 快点!", "太早了...", "跑掉了...", "钓到了: %s", "旧靴子", "贝壳", "珍珠", "今天还有%u次奖励", "现在只是玩玩", "森林小路", "阳光海滩", "山间小道", "火山洞穴", "打开", "还差%u步", "宝箱! %u枚金币", "找到了: %s", "路线完成! +10", "带着球走路吧!", "缺少材料", "点击搅拌!", "做好了: %s!", "家务", "存钱罐", "日记", "整理床铺", "做作业", "整理房间", "帮忙做家务", "读书", "刷牙", "谢谢! +4", "已存", "取出", "+%u (%u天后)", "存钱每周有利息", "今天", "昨天", "吃了%u次", "玩了%u个游戏", "洗了%u次澡", "散步%u次", "钓鱼%u次", "学习%u次", "做了%u件家务", "赚了%u枚金币", "一起做饭了", "去冒险了", "赢得了奖杯!", "稀有扭蛋!", "平静的一天", "闹钟", "开", "早上好!", "点击停止", "该上床睡觉了!", "钓鱼20次", "第一条路线", "中心",
    // phase 12 games
    "记忆配对", "跟我做", "飞行", "迷宫", "找出配对", "重复顺序", "看好...", "点击飞行", "倾斜或拖动",
    // phase 13: hats and the room
    "厨师帽", "海盗帽", "猫耳", "兔耳", "耳机", "学士帽", "花冠", "安全帽", "贝雷帽", "维京头盔", "家具", "我的房间", "窗户", "挂钟", "画", "架子", "沙发", "电视", "鱼缸", "书架", "壁炉", "地毯", "蓝色壁纸", "粉色壁纸", "薄荷壁纸",
    // phase 14
    "我想要%s!", "我想玩!", "去公园吧!", "我想洗澡!", "去钓鱼吧!", "摸摸我!", "愿望实现! +5", "有点寂寞", "今天超棒!", "我想你了...", "生日快乐! +20", "%u个月啦!", "复活节彩蛋! +5", "夏日祭: 免费冰淇淋!", "性格: %s", "爱玩", "懒洋洋", "贪吃", "害羞",
    // phase 15
    "游戏", "头脑", "街机", "气球", "赛跑", "接住", "平衡", "属性测验", "滑块拼图", "泡泡", "画画", "戳气球, 别碰炸弹", "摇动或快速点击", "接住, 躲开霹雳电球", "倾斜或按住一边", "效果如何?", "按1到8排好", "点击同色泡泡", "完成",
    // phase 16
    "服装", "墨镜", "星星眼镜", "爱心眼镜", "单片眼镜", "围巾", "领结", "奖牌", "头巾", "斗篷", "背包", "天使翅膀", "蝙蝠翅膀", "真时髦!",
    // phase 17
    "海滩", "图书馆", "音乐厅", "比赛", "游戏厅", "打工", "城堡", "贝壳! +1", "今天没有贝壳了", "沙堡!", "今天已经堆过了", "阅读", "今天读够了", "今天听够了", "战斗", "胜利", "摇篮曲", "参加", "下周再来", "这次没有缎带... +2", "铜缎带! +5", "银缎带! +10", "金缎带! +20", "服装, 亲密度和心情", "咖啡馆", "花店", "邮局", "3小时 +15", "在打工", "%u:%02u 回来", "下班回来了! +15", "明天再打工", "信件", "没有信", "大木博士: 做得好! 送你5枚金币。", "喵喵: 送你一块饼干, 喵!", "吉利蛋: 送你一颗种子。", "皮卡丘: 皮卡皮卡! (肥皂)", "奶奶: 8枚金币给你存起来。", "朋友: 想你了, 一起玩吧!",
    // phase 19
    "任务", "皮卡丘的帽子丢了", "喵喵的午饭", "吉利蛋丢失的蛋", "给胖丁的歌", "去公园找找", "去海滩找找", "去图书馆问问", "在湖边钓鱼", "做一道菜", "一起吃自制的菜", "打开一个冒险站点", "去宝可梦中心", "洗个澡", "放一首歌", "玩个游戏", "在学校学习", "下一步明天继续", "任务完成! 新帽子", "不给糖就捣蛋! 饼干", "免费热可可!", "在一起一年了! +50",
    // the photo album
    "相册", "还没有照片", "照片已存入相册!", "拍照",
    // the Nursery
    "托儿所", "一起待一天:40金币", "留下两只", "选两个朋友", "付40", "它不能去", "明天见!", "下周再来", "还要", "拿蛋", "一颗蛋!下一个就是它", "你的下一颗蛋", "退回", "再按一次退回", "它们的蛋已经在家了", "来自托儿所",
    // Wi-Fi
    "未设Wi-Fi:请在安装页面设置", "Wi-Fi:连接中...", "Wi-Fi已校时", "Wi-Fi:无法连接",
    // snow from New York
    "纽约下雪啦!", "看,纽约的雪!", "纽约的雪:开", "纽约的雪:关",
    // restful nights
    "睡觉中:需求也在休息",
    // the try-on mirror
    "试穿", "我看起来怎么样?",
    // the arcade
    "奖品", "纪录", "彩票", "你赢到了!", "彩票不够", "街机帽", "星星遮阳帽", "街机", "本周", "最佳", "铜牌!", "银牌!", "金牌!",
    // arcade trophies
    "10枚金牌", "3件街机奖品",
    // what it says
    "我饿了!", "肚子咕咕叫!", "我们吃点东西吧?", "请给我一个果子!", "好香啊...", "我需要点心!",
    "我要洗澡!", "我们洗澡吧?", "我好脏啊...", "想要泡泡!", "我想香香的", "哈欠...好困。",
    "睡个午觉?", "眼睛快闭上了", "我有点累", "帮我盖被子?", "我不太舒服...", "我肚子疼",
    "阿嚏!阿嚏!", "去宝可梦中心吧?", "请给我药", "我想你了!", "陪我玩好吗?", "我一个人好孤单...",
    "别走!", "抱抱我?", "好无聊...", "我们玩游戏吧!", "去公园吧!", "做点好玩的吧",
    "比赛跑步吧?", "多美好的一天!", "我好开心!", "很高兴你在这里", "看我会做什么!", "今天我要勇敢",
    "啦啦啦!", "我们是最棒的组合!", "我想学点新东西", "我喜欢我们的家", "今天做什么呢?", "给我讲个故事!",
    "我什么都准备好了!", "我好爱你!", "你是我最好的朋友", "永远是好朋友!", "你把我照顾得很好", "一个大大的拥抱!",
    "谢谢你做的一切", "有你一切都更好", "你很特别", "早上好!", "睡得好吗?", "早餐时间!",
    "新的一天!", "刷牙时间到!", "下午好!", "你今天过得怎么样?", "点心时间!", "作业做完了吗?",
    "晚上好!", "快到睡觉时间了", "晚饭吃什么?", "看,星星出来了", "今天真不错", "做个好梦...",
    "已经很晚了!", "月亮真美", "阳光真好!", "晒太阳真舒服", "出去玩吧?", "蓝天!",
    "好多云...", "那朵云像宝可梦", "今天多云", "下雨了!", "滴答滴答...", "去踩水坑吗?",
    "别忘了带伞", "打雷了!待在我身边", "好大的暴风雨!", "我看到闪电了!", "什么都看不见!", "今天有雾",
    "好神秘...", "下雪了!", "堆雪人吧!", "雪花!", "打雪仗!", "纽约来的雪!",
    "纽约好冷!", "好热啊!", "请给我冰淇淋!", "我需要冰水", "好冷啊!", "我需要围巾",
    "来杯热可可?", "花园可以收获了!", "长出好吃的了!", "去收获吧!", "我们学习一会儿吧?", "我喜欢数字",
    "来做测验吧!", "去游戏厅吧!", "我想要更多彩票!", "我们破纪录吧?", "我想要金牌!", "托儿所有一颗蛋!",
    "会孵出什么呢?", "来信了!", "我们看看信吧?", "继续冒险吧?", "今天再完成一步!", "我今天好努力工作!",
    "我赚了金币", "是我们的生日!", "有蛋糕吗?", "谢谢又陪我一个月", "不给糖就捣蛋!", "哇!吓到你了吗?",
    "我喜欢南瓜", "节日快乐!", "会有礼物吗?", "我喜欢彩灯", "去找彩蛋吧!", "彩色的蛋!",
    "去海边吧!", "夏天万岁!", "我会一直陪着你", "我相信你", "在一起我们很强", "你是最棒的训练家",
    "我觉得我要变了...", "我身上发生了什么!",
    // phase 31
    "泡泡浴", "刷牙", "午睡或摇篮曲", "吃一道家常菜", "赢得%u张奖券", "转一次扭蛋", "玩属性测验", "参加比赛", "去公园散步", "到达冒险站点", "钓鱼%u次", "去海滩玩", "去宝可梦中心", "做一道菜", "种植或收获", "做%u件家务", "读一本书", "学习一会儿", "放一首歌", "打开一封信", "存进存钱罐", "买食物", "买点东西", "装饰小屋", "帮忙做家务", "去商店买东西", "在街机赢奖券", "走3000步", "妙蛙种子的菜园", "可达鸭头疼了", "卡比兽的大午觉", "伊布的生日派对", "杰尼龟的海滩日", "鲤鱼王想飞", "皮皮的月亮之夜", "凯西去上学",
    // phase 32
    "扫掉便便", "没有要扫的", "也把便便扫掉吧!", "清洁+%u", "扫干净了!",
    // phase 32b
    "扫干净了! 清洁+%u",
    // phase 33
    "删除这张照片?", "删除", "全部删除", "删除除收藏外的所有照片?", "照片已删除", "已经有%u张收藏了",
    // phase 34
    "%s向你挑战!", "徽章 %u/8", "四天王 %u/4", "决赛: 冠军", "战斗!", "稍后",
    // phase 35
    "步数", "今天", "最近7天", "到道馆的步数", "冒险路线", "总计: %u步", "准备好了! 开战!", "一二三四五六日", "所有道馆都赢了!",
    // phase 36
    "步数够了! %u天后迎来挑战", "步数够了! 明天迎来挑战",
    // phase 36
    "%u天后挑战", "明天挑战",
    // phase 40
    "%s! +%u 金币", "%u 步有奖励",
    // phase 43
    "2500 步! 吃点心", "5000 步! 走得真棒", "10000 步! 变强了", "新手", "散步者", "步行者", "远足者", "徒步者", "领路人", "环球旅人", "步道高手", "传奇",
    // phase 44
    "草丛", "精灵球", "精灵球", "超级球", "高级球", "野生的 %s!", "抓到了 %s!", "送进了箱子.", "%s 挣脱了!", "队伍和箱子都满了", "没有球了: 去商店买", "找到了精灵球!", "草丛里有动静!", "再走 %u 步", "它正在睡觉", "草丛里没有人", "在草丛里使用",
    // phase 45
    "旅行", "旅行中", "回来了!", "%s 出发旅行了!", "今天不能再旅行了", "太晚了: 明天吧", "叫回来", "叫回来? 没有故事", "%s 去了河边", "找到了 %u 枚金币!", "%s 在灌木下找东西", "找到了精灵球!", "%s 遇见了 %s", "一起吃了点心!", "%s 逛了小镇", "得到了一块饼干!", "%s 在花园里玩", "找到了一颗种子!", "%s 踩了水坑", "带回了肥皂!", "%s 看到有东西在动", "在草丛里!", "%s 爬上了小山", "找到了超级球!", },
  // ---------------- KO ----------------
  {
    "진화 중!", "냠냠!", "좋아해요!", "배고파요!", "목욕이 필요해요!",
    "지쳤어요...", "슬퍼요...", "조금 통통해요...", "이로치예요!!", "행복해요",
    "고마워요! 안녕", "도망갔어요...", "안녕! 손을 흔들어요...",
    "알", "전설의 알!?", "희귀한 알!", "알을 눌러요...", "움직여요!", "거의 다 됐어요!",
    "도감 %u/%u",
    "%s%s Lv.%u",
    "%s 놓아줄까요?", "예", "아니오",
    "%u 회", "힘 +%u", "신기록!", "최고: %u", "빨리 쳐요!",
    "점수: %u", "너무 재밌어요!", "+행복",
    "시간 설정", "시", "분", "위로 밀기: 취소", "언어",
    "메달!", "멋져요!", "%u 일 연속!",
    "연속 %u  최고 %u", "유대", "열매 ???", "빨간 열매", "파란 열매", "초록 열매",
    "%s   나이 %lu일", "이름 누르기: 이름 변경",
    "배틀", "공격", "방어", "속도", "무게", "힘 훈련",
    "메달 %d/%d", "누르기: 뒤로",
    "이름:", "눌러서 뒤로",
    "식사", "기쁨", "기력", "청결",
    "최고 %u",
    "진행", "Lv.%u", "%u 분 후 Lv.%u", "진화", "최종 형태",
    "진화 준비 완료!", "모든 욕구 >=40 필요",
    "%u 레벨 후 진화", "실수: %u",
    "켬", "끔",
    "진화!", "%s: 할 말이 있어요...", "%s: 버려진 기분이에요...",
    "진화할까요?", "그대로", "작별할까요?", "작별", "함께 있기",
    "첫 파트너를 골라요",
    "스프라이트 없음", "SD에 넣어 주세요",
    "체력", "IV %u",
    "메뉴", "설정", "닫기", "파티 %u/6", "- 비어 있음 -", "%s 파티에 합류!", "파티 가득: 교체할 대상 선택", "그만두기",
    "능력치", "훈련", "힘", "속도", "방어", "행복하면 저절로 늘어요",
    "기술", "- 비어 있음 -", "기술 선택", "칸을 눌러 변경", "위력 %u", "변화",
    "%s: 배우고 싶어요", "배우지 않기",
    "%s 의 %s!", "효과가 굉장했다!", "효과가 별로인 듯하다...", "효과가 없었다...",
    "%s 빗나갔다!", "급소에 맞았다!", "%s 쓰러졌다!", "스스로를 다치게 했다!",
    "%s: %s", "마비", "화상", "독", "잠듦", "얼음", "혼란",
    "승리!", "패배...",
    "%s 의 %s 등장!", "가라, %s!", "체육관", "배지 %u/8", "트레이너",
    "속도 +%u", "누르기: 아바타 변경", "총 %u 획득", "쉬움", "어려움", "선택 %u/%u", "싸우기", "잠김", "포켓몬", "%s 쓰러뜨림!", "새 배지!", "볼륨 %u", "박스 %u/%u", "%s 와 교체: 칸 선택", "박스", "데려오기", "알이 있을 때만", "LAN 배틀", "호스트", "참가", "검색 중...", "준비 완료!", "버전 다름", "호스트 또는 참가", "상대: %u 마리", "상대가 나갔어요", "상대를 기다리는 중...", "다시", "도망", "이 알의 출신지", "%s +%u", "더 이상 훈련할 수 없어요", "지역 선택", "은퇴", "일찍 은퇴할까요?", "다음 아이는 하루 늦게 진화", "하루 늦게 진화",
    "팩 필요", "놓아주기", "영영 이별", "파티로", "파티에 들어가지 않아요", "볼륨", "언어", "정보", "밝기", "테스트",
    // Fun Pack
    "왼쪽/오른쪽", "높게/낮게", "누구일까?", "열매 받기", "두더지", "줄넘기", "댄스", "숨바꼭질", "어느 쪽을 볼까?", "더 클까 작을까?", "크다", "작다", "이 포켓몬은 누구?", "좌우를 눌러 이동", "눌러! 찌리리공은 안돼", "눌러서 점프!", "박자에 맞춰 눌러!", "완벽", "좋아", "실패", "어디에 숨었을까?", "기쁨 +%u", "방어 +%u", "정답!", "아쉽다...", "배가 아파!", "감기에 걸렸어!", "기분이 나빠...", "심심해...", "Zzz... 불 꺼줘!", "아파서 못 놀아!", "다 나았어!", "나중에 한 번 더", "10분 기다려", "이제 목욕!", "으웩! 안 아파", "더 많은 게임", "미니게임",
    // Fun Pack
    "샌드백", "반응 속도", "공놀이", "놀이",
    // coins and daily missions
    "코인", "오늘의 미션", "먹이 %u번 주기", "좋아하는 열매 주기", "미니게임 %u번", "%u걸음 걷기", "목욕시키기", "%u번 쓰다듬기", "힘 게임 하기", "속도 게임 하기", "방어 게임 하기", "%u점 얻기", "오늘 놀이: %u/%u", "모두 완료: +%u 추가", "모으는 방법", "놀아주고 돌보기", "1000걸음마다 +%u", "하루 첫 돌봄 +%u", "미션마다 +%u", "새 배지 +%u", "불 끄고 재우기 +%u", "상점이 곧 열려요!",
    // shop, bag and hats
    "상점", "가방", "모자", "야구모자", "리본", "꽃", "비니", "파티 모자", "밀짚모자", "마녀 모자", "산타 모자", "실크해트", "왕관", "구매", "아니요", "보유", "코인이 부족해요", "구매 완료!", "착용 중", "모자 벗기", "모자 옮기기", "아직 아무것도 없어요", "완료",
    // toys and decorations
    "장난감", "장식", "배치됨", "비치볼", "인형", "고무오리", "블록", "북", "연", "화분", "꽃병", "램프", "침대", "선물", "호박",
    // foods
    "음식", "식사", "간식", "음료", "약", "샌드위치", "카레", "수프", "주먹밥", "케이크", "쿠키", "아이스크림", "우유", "주스", "다 떨어졌어요: 상점으로", "x%u",
    // bath and sleep menus
    "목욕", "샤워", "거품 목욕", "문지르기", "빗질", "얼룩을 문질러!", "반짝반짝!", "비누가 필요해요", "빗을 사세요", "잠자기", "재우기", "낮잠 30분", "자장가", "관리", "비누", "빗",
    // places, garden, seasonal gifts
    "외출", "공원", "포켓몬센터", "텃밭", "산책", "들어가기", "돌아가기", "치료", "오늘 무료", "완전히 회복했어요!", "코인 %u개를 찾았어요!", "열매를 찾았어요!", "%s 인사해요!", "즐거운 산책", "너무 피곤해요", "심기", "수확", "씨앗을 사세요", "자라는 중... %u%%", "씨앗", "수확: %s", "할로윈 선물!", "크리스마스 선물!", "새해 복 많이!", "발렌타인 선물!",
    // capsules, scenery, trophies, teeth, school, quiz, parents, talk
    "배경", "초원", "해변", "숲", "화산", "산", "설원", "묘지", "트로피", "새 트로피! +10", "식사 100번", "게임 50번", "목욕 30번", "산책 25번", "수확 5번", "코인 1000개", "모자 5개", "장식 6개", "7일 연속", "레벨 20", "공부 10번", "양치 14번", "퀴즈 만점", "캡슐 10개", "캡슐", "오늘 %u개 남음", "나왔다: %s!", "비누 2개!", "씨앗 2개!", "코인 %u개!", "내일 다시 와요", "양치하기", "이가 반짝반짝!", "오늘 이미 2번 했어요", "눌러서 양치", "학교", "공부", "수학", "얼마나 할까요?", "%u분", "공부 중...", "잘했어요! +%u", "잘했어요!", "그만", "10개 중 %u개!", "부모님", "잠잘 시간", "하루 게임", "끔", "눌러서 변경", "잘 시간이에요", "오늘은 충분히 놀았어요", "배고파요!", "목욕하고 싶어요", "졸려요...", "몸이 안 좋아요...", "놀자!", "비가 와요!", "눈이 와요!", "텃밭이 다 자랐어요!", "사랑해요!", "정말 좋은 날!", "공부할까요?",
    // adventures, lake, kitchen, chores, piggy bank, diary, alarm
    "열매 파이", "스튜", "파르페", "호수", "모험", "부엌", "낚시", "기다리는 중...", "입질! 지금 눌러!", "너무 빨라요...", "놓쳤어요...", "낚았다: %s", "낡은 장화", "조개껍데기", "진주", "오늘 남은 보상: %u", "이제 재미로만", "숲길", "햇살 해변", "산길", "화산 동굴", "열기", "%u걸음 남음", "보물상자! 코인 %u개", "찾았다: %s", "루트 완주! +10", "공을 들고 걸어요!", "재료가 부족해요", "눌러서 저어요!", "완성: %s!", "집안일", "저금통", "일기", "침대 정리", "숙제하기", "방 정리", "집안일 돕기", "책 읽기", "양치하기", "고마워요! +4", "저금", "꺼내기", "+%u (%u일 후)", "저금하면 매주 이자", "오늘", "어제", "%u번 먹었어요", "게임 %u번", "목욕 %u번", "산책 %u번", "낚시 %u번", "공부 %u번", "집안일 %u개", "코인 %u개 벌었어요", "같이 요리했어요", "모험을 떠났어요", "트로피를 받았어요!", "희귀 캡슐!", "조용한 하루", "알람", "켬", "좋은 아침!", "눌러서 끄기", "잘 시간이에요!", "물고기 20마리", "첫 루트", "센터",
    // phase 12 games
    "기억력", "따라하기", "비행", "미로", "짝을 찾아요", "순서를 따라해요", "잘 봐요...", "눌러서 날아요", "기울이거나 끌어요",
    // phase 13: hats and the room
    "요리사 모자", "해적 모자", "고양이 귀", "토끼 귀", "헤드폰", "학사모", "꽃관", "안전모", "베레모", "바이킹 투구", "가구", "내 방", "창문", "시계", "그림", "선반", "소파", "TV", "어항", "책장", "벽난로", "러그", "파란 벽지", "분홍 벽지", "민트 벽지",
    // phase 14
    "%s 먹고 싶어요!", "놀고 싶어요!", "공원 가요!", "목욕하고 싶어요!", "낚시 가요!", "쓰다듬어 줘요!", "소원 성취! +5", "외로워해요", "최고의 하루!", "보고 싶었어요...", "생일 축하해요! +20", "%u개월!", "부활절 달걀! +5", "여름 축제: 아이스크림 공짜!", "성격: %s", "장난꾸러기", "게으름뱅이", "먹보", "수줍음",
    // phase 15
    "게임", "두뇌", "아케이드", "풍선 터뜨리기", "흔들기 경주", "받기", "균형", "타입 퀴즈", "슬라이드 퍼즐", "버블", "그림 그리기", "폭탄은 피해요", "흔들거나 빠르게 눌러요", "받고, 찌리리공은 피해요", "기울이거나 한쪽을 눌러요", "효과는 얼마나?", "1부터 8까지 정렬", "같은 색 묶음을 눌러요", "완성",
    // phase 16
    "옷", "선글라스", "별 안경", "하트 안경", "외알 안경", "목도리", "나비넥타이", "메달", "반다나", "망토", "배낭", "천사 날개", "박쥐 날개", "멋쟁이!",
    // phase 17
    "해변", "도서관", "음악당", "콘테스트", "오락실", "아르바이트", "모래성", "조개! +1", "오늘은 조개가 없어요", "모래성 완성!", "오늘 이미 만들었어요", "읽기", "오늘은 충분히 읽었어요", "오늘은 음악 그만", "배틀", "승리", "자장가", "참가", "다음 주에 와요", "이번엔 리본 없음... +2", "동 리본! +5", "은 리본! +10", "금 리본! +20", "옷, 친밀도, 기쁨이 중요해요", "카페", "꽃집", "우체국", "3시간 +15", "일하는 중", "%u:%02u 돌아와요", "퇴근했어요! +15", "내일 또 해요", "우편", "편지가 없어요", "오박사: 잘했어! 코인 5개 줄게.", "나옹: 쿠키 선물이다옹!", "럭키: 텃밭에 심을 씨앗이야.", "피카츄: 피카피카! (비누)", "할머니: 저금하렴, 코인 8개.", "친구: 보고 싶어, 같이 놀자!",
    // phase 19
    "퀘스트", "피카츄가 모자를 잃어버렸어요", "나옹의 점심", "럭키의 잃어버린 알", "푸린을 위한 노래", "공원을 찾아봐요", "해변을 찾아봐요", "도서관에 물어봐요", "호수에서 낚시해요", "요리를 해요", "직접 만든 음식을 먹어요", "모험 정류장을 열어요", "포켓몬센터에 가요", "목욕해요", "노래를 틀어요", "게임을 해요", "학교에서 공부해요", "다음 단계는 내일", "퀘스트 완료! 새 모자", "트릭 오어 트릿! 쿠키", "핫초코 공짜!", "함께한 지 1년! +50",
    // the photo album
    "앨범", "아직 사진이 없어요", "앨범에 사진 저장!", "사진",
    // the Nursery
    "보육원", "하루 함께: 40코인", "두 마리 맡기기", "친구 두 명 고르기", "40 내기", "그 친구는 안 돼요", "내일 봐요!", "다음 주에 다시 와요", "남은 시간", "알 받기", "알이다! 다음 알이 될 거예요", "다음 알", "돌려주기", "한 번 더 누르면 돌려줘요", "알은 이미 집에 있어요", "보육원에서 온 알",
    // Wi-Fi
    "Wi-Fi 없음: 설치 페이지에서 설정하세요", "Wi-Fi: 연결 중...", "Wi-Fi로 시간 맞춤", "Wi-Fi: 연결 실패",
    // snow from New York
    "뉴욕에 눈이 와요!", "봐, 뉴욕에서 온 눈이야!", "뉴욕의 눈: 켜짐", "뉴욕의 눈: 꺼짐",
    // restful nights
    "자는 중: 필요도 쉬고 있어요",
    // the try-on mirror
    "입어보기", "나 어때?",
    // the arcade
    "경품", "기록", "티켓", "받았어요!", "티켓이 부족해요", "아케이드 모자", "별 썬캡", "오락기", "이번 주", "최고", "동메달!", "은메달!", "금메달!",
    // arcade trophies
    "금메달 10개", "아케이드 경품 3개",
    // what it says
    "배고파!", "배에서 꼬르륵!", "뭐 좀 먹을까?", "열매 하나만 줘!", "맛있는 냄새...", "간식이 필요해!",
    "목욕하고 싶어!", "목욕할까?", "온몸이 지저분해...", "거품 목욕 해줘!", "좋은 냄새 나고 싶어", "하암... 졸려.",
    "낮잠 잘까?", "눈이 감겨", "조금 피곤해", "이불 덮어줄래?", "몸이 안 좋아...", "배가 아파",
    "에취! 에취!", "센터에 갈까?", "약 좀 줘", "보고 싶었어!", "나랑 놀아줄래?", "혼자 있었어...",
    "가지 마!", "안아줄래?", "심심해...", "게임하자!", "공원 가자!", "재미있는 거 하자",
    "달리기 시합?", "정말 좋은 하루야!", "너무 행복해!", "네가 있어서 좋아", "나 이거 할 수 있어!", "오늘은 용감해질 거야",
    "랄라라!", "우리는 최고의 팀이야!", "새로운 걸 배우고 싶어", "우리 집이 좋아", "오늘 뭐 할까?", "이야기 해줘!",
    "뭐든 준비됐어!", "많이 사랑해!", "넌 내 최고의 친구야", "영원한 친구!", "날 잘 돌봐줘서 고마워", "꼭 안아줘!",
    "모든 게 고마워", "너랑 있으면 다 좋아", "넌 특별해", "좋은 아침!", "잘 잤어?", "아침 먹을 시간!",
    "새로운 하루!", "양치하자!", "좋은 오후!", "오늘 하루 어땠어?", "간식 시간!", "숙제 했어?",
    "좋은 저녁!", "곧 잘 시간이야", "저녁 뭐 먹어?", "봐, 별이 떴어", "좋은 하루였어", "좋은 꿈 꿔...",
    "벌써 늦었어!", "달이 예뻐", "햇살 좋은 날!", "햇볕이 따뜻해", "밖에서 놀까?", "파란 하늘!",
    "구름이 많아...", "저 구름 포켓몬 같아", "오늘은 흐려", "비가 와!", "똑똑똑...", "물웅덩이 뛰기?",
    "우산 챙겨", "천둥이야! 곁에 있어줘", "폭풍우가 대단해!", "번개를 봤어!", "아무것도 안 보여!", "오늘 안개가 꼈어",
    "신비로워...", "눈이 와!", "눈사람 만들자!", "눈송이!", "눈싸움!", "뉴욕에서 온 눈!",
    "뉴욕은 추워!", "너무 더워!", "아이스크림 줘!", "시원한 물이 필요해", "으으, 추워!", "목도리가 필요해",
    "핫초코 마실까?", "정원 수확할 시간!", "맛있는 게 자랐어!", "수확하러 가자!", "조금 공부할까?", "숫자가 좋아",
    "퀴즈 하자!", "오락실 가자!", "티켓 더 모을래!", "기록 깰까?", "금메달 갖고 싶어!", "보육원에 알이 있어!",
    "뭐가 태어날까?", "편지가 왔어!", "편지 읽을까?", "모험 계속할까?", "오늘 한 단계 더!", "오늘 열심히 일했어!",
    "코인 벌었어", "우리 생일이야!", "케이크 있어?", "함께한 한 달 고마워", "과자 안 주면 장난칠 거야!", "왁! 놀랐지?",
    "호박이 좋아", "즐거운 연휴!", "선물 있을까?", "불빛이 좋아", "달걀 찾으러 가자!", "알록달록 달걀!",
    "해변 가자!", "여름 최고!", "항상 곁에 있을게", "널 믿어", "함께라면 강해", "넌 최고의 트레이너야",
    "곧 변할 것 같아...", "뭔가 일어나고 있어!",
    // phase 31
    "거품 목욕", "양치하기", "낮잠이나 자장가", "집밥 먹기", "티켓 %u장 얻기", "캡슐 뽑기", "타입 퀴즈 하기", "콘테스트 참가", "공원 산책", "모험 지점 도착", "낚시 %u번", "해변에서 놀기", "포켓몬센터 방문", "요리하기", "심기 또는 수확", "집안일 %u개", "책 읽기", "공부하기", "노래 틀기", "편지 열기", "저금통에 저금", "음식 사기", "무언가 사기", "집 꾸미기", "집안일 돕기", "상점에서 사기", "오락실 티켓 얻기", "3000걸음 걷기", "이상해씨의 텃밭", "고라파덕의 두통", "잠만보의 긴 낮잠", "이브이의 생일 파티", "꼬부기의 해변 나들이", "잉어킹은 날고 싶어", "삐삐의 달밤", "캐이시의 첫 등교",
    // phase 32
    "응가 쓸기", "쓸 게 없어요", "응가도 쓸어 주세요!", "청결 +%u", "깨끗하게 쓸었어요!",
    // phase 32b
    "깨끗하게 쓸었어요! 청결 +%u",
    // phase 33
    "이 사진을 지울까요?", "지우기", "모두 지우기", "즐겨찾기 빼고 모두 지울까요?", "사진을 지웠어요", "이미 즐겨찾기가 %u개예요",
    // phase 34
    "%s의 도전!", "배지 %u/8", "사천왕 %u/4", "결승: 챔피언", "싸우자!", "나중에",
    // phase 35
    "걸음", "오늘", "지난 7일", "체육관까지 걸음", "모험 루트", "총 %u걸음", "준비 완료! 싸우자!", "월화수목금토일", "모든 체육관 승리!",
    // phase 36
    "걸음 완료! %u일 후 도전이 와요", "걸음 완료! 내일 도전이 와요",
    // phase 36
    "%u일 후 도전", "내일 도전",
    // phase 40
    "%s! +%u 코인", "%u 걸음에 보상",
    // phase 43
    "2500 걸음! 간식 시간", "5000 걸음! 멋진 산책", "10000 걸음! 더 강해졌어", "초보", "산책가", "걷기꾼", "등산가", "트레커", "길잡이", "세계 여행자", "길의 고수", "전설",
    // phase 44
    "풀숲", "볼", "몬스터볼", "수퍼볼", "하이퍼볼", "야생의 %s!", "%s 잡았다!", "상자로 보냈어.", "%s 빠져나왔어!", "팀과 상자가 꽉 찼어", "볼이 없어: 상점에서 사자", "몬스터볼을 찾았어!", "풀숲에서 뭔가 움직여!", "%u 걸음 더 걷자", "지금 자고 있어", "풀숲에 아무도 없어", "풀숲에서 쓰자",
    // phase 45
    "여행", "여행 중", "돌아왔어!", "%s 여행을 떠났어!", "오늘은 여행 끝", "늦었어: 내일 가자", "부르기", "부를까? 이야기 없어", "%s 강에 갔어", "코인 %u개를 찾았어!", "%s 덤불을 뒤졌어", "몬스터볼을 찾았어!", "%s, %s 만났어", "간식을 나눠 먹었어!", "%s 마을을 걸었어", "쿠키를 받았어!", "%s 정원에서 놀았어", "씨앗을 찾았어!", "%s 물웅덩이에서 놀았어", "비누를 가져왔어!", "%s 뭔가 움직이는 걸 봤어", "풀숲에서!", "%s 언덕에 올랐어", "수퍼볼을 찾았어!", },
};

// Medal names in their three lengths [language][medal].
static const char *const MED_NAME[LANG_COUNT][MED_COUNT] = {
  { "Nv.10", "Nv.25", "Nv.50", "BAYA", "RACHA 7", "VINCULO", "FORMA TOPE", "EN FORMA" },
  { "Lv.10", "Lv.25", "Lv.50", "BERRY", "7 STREAK", "BOND", "TOP FORM", "IN SHAPE" },
  { "Niv.10", "Niv.25", "Niv.50", "BAIE", "SERIE 7", "LIEN", "FORME MAX", "EN FORME" },
  { "Lv.10", "Lv.25", "Lv.50", "BEERE", "7 SERIE", "BINDUNG", "ENDFORM", "FIT" },
  { "Lv.10", "Lv.25", "Lv.50", "BACCA", "SERIE 7", "LEGAME", "FORMA MAX", "IN FORMA" },
  { "Niv.10", "Niv.25", "Niv.50", "BAGA", "SEQ 7", "LACO", "FORMA MAX", "EM FORMA" },
  { "Lv.10", "Lv.25", "Lv.50", "树果", "连续7天", "羁绊", "最终形态", "健康" },
  { "Lv.10", "Lv.25", "Lv.50", "열매", "7일 연속", "유대", "최종 형태", "건강" },
};
static const char *const MED_LBL[LANG_COUNT][MED_COUNT] = {
  { "Nv10", "Nv25", "Nv50", "BAYA", "7DIAS", "VINC", "TOPE", "SANO" },
  { "Lv10", "Lv25", "Lv50", "BERRY", "7DAYS", "BOND", "TOP", "FIT" },
  { "Niv10", "Niv25", "Niv50", "BAIE", "7JRS", "LIEN", "MAX", "FORME" },
  { "Lv10", "Lv25", "Lv50", "BEERE", "7TAGE", "BND", "END", "FIT" },
  { "Lv10", "Lv25", "Lv50", "BACCA", "7GG", "LEG", "MAX", "FIT" },
  { "Niv10", "Niv25", "Niv50", "BAGA", "7DIAS", "LACO", "MAX", "FIT" },
  { "Lv10", "Lv25", "Lv50", "树果", "7天", "羁绊", "终极", "健康" },
  { "Lv10", "Lv25", "Lv50", "열매", "7일", "유대", "최종", "건강" },
};
static const char *const MED_DSC[LANG_COUNT][MED_COUNT] = {
  { "NIVEL 10", "NIVEL 25", "NIVEL 50", "BAYA HALLADA",
    "RACHA 7 DIAS", "VINCULO MAX", "FORMA FINAL", "EN FORMA" },
  { "LEVEL 10", "LEVEL 25", "LEVEL 50", "BERRY FOUND",
    "7 DAY STREAK", "MAX BOND", "FINAL FORM", "IN SHAPE" },
  { "NIVEAU 10", "NIVEAU 25", "NIVEAU 50", "BAIE TROUVEE",
    "SERIE 7 JOURS", "LIEN MAX", "FORME FINALE", "EN FORME" },
  { "LEVEL 10", "LEVEL 25", "LEVEL 50", "BEERE GEFUNDEN",
    "7 TAGE SERIE", "MAX BINDUNG", "ENDFORM", "FIT" },
  { "LIVELLO 10", "LIVELLO 25", "LIVELLO 50", "BACCA TROVATA",
    "SERIE 7 GIORNI", "LEGAME MAX", "FORMA FINALE", "IN FORMA" },
  { "NIVEL 10", "NIVEL 25", "NIVEL 50", "BAGA ACHADA",
    "SEQ 7 DIAS", "LACO MAX", "FORMA FINAL", "EM FORMA" },
  { "等级 10", "等级 25", "等级 50", "发现树果",
    "连续 7 天", "羁绊满值", "最终形态", "健康" },
  { "레벨 10", "레벨 25", "레벨 50", "열매 발견",
    "7일 연속", "최대 유대", "최종 형태", "건강" },
};

const char *T(StrId id) { return STRINGS[gLang][id]; }
const char *medalName(int i)  { return MED_NAME[gLang][i]; }
const char *medalLabel(int i) { return MED_LBL[gLang][i]; }
const char *medalDesc(int i)  { return MED_DSC[gLang][i]; }

void loadLang() {
  Preferences p;
  p.begin("tamapoke", true);  // read-only
  uint8_t v = p.getUChar("lang", LANG_DEFAULT);
  p.end();
  gLang = (v < LANG_COUNT) ? (Lang)v : LANG_DEFAULT;
}

void setLang(Lang l) {
  if (l >= LANG_COUNT) return;
  gLang = l;
  Preferences p;
  p.begin("tamapoke", false);
  p.putUChar("lang", (uint8_t)l);
  p.end();
}
