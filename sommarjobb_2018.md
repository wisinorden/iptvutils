# TODO
* Går för tillfället inte att stoppa när man spelar upp ts-filer.
* För tillfället är "producer" direkt kopplad till "consumer" (i tsfilenetworkplayer.cpp). 
Funktionalitet för "middleware" måste inplementeras för att fungera som för pcap-filer.
* För ts-filer med fler än ett program: göra det valbart i GUI:t vilket program man vill läsa PCR ifrån. 
(funktionalitet finns förberett i tsnetworkconsumer.cpp (`playFromQNetwork()`)).
* Avgöra om det är VBR eller CBR och välja rätt funktion (i tsnetworkconsumer.cpp (`run()`)). 
För tillfället är det `playVBRFromQNetwork()` ,för VBR, och `playFromQNetwork()` ,för CBR, som gäller. 

# DONE
* Implementerat funktionalitet för uppspelning av ts-filer med CBR och VBR.
* Gjort abstraktioner för att stödja inläsning/uppspelning av ts-filer.

I och med att allt som har med pcap i programmet använder sig av biblioteket `pcap.h` så har det inte gått att använda sig 
av samma klasser som för pcap-filer när vi vill arbeta med ts-filer. Jag har därför försökt göra abstraktioner och introducera
ts-klasser som beter sig på liknande sätt som respektive pcap-klasser. 
