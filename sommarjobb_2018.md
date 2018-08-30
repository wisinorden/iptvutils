# TODO
* Går för tillfället inte att stoppa när man spelar upp ts-filer.
* För tillfället är "producer" direkt kopplad till "consumer" (i tsfilenetworkplayer.cpp). 
Funktionalitet för "middleware" måste inplementeras för att fungera som för pcap-filer.
* För ts-filer med fler än ett program: göra det valbart i GUI:t vilket program man vill läsa PCR ifrån. 
(funktionalitet finns förberett i tsnetworkconsumer.cpp (playFromQNetwork())).

# DONE
* Implementerat funktionalitet för uppspelning av ts-filer med CBR och VBR.
* Gjort abstraktioner för att stödja inläsning/uppspelning av ts-filer.
