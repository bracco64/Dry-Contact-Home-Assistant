# Dry-Contact-Home-Assistant
progetti fai da te
**********************************
Legge la tensione della batteria ed attiva / disattiva un relè a delle soglie di tensione preimpostate. 
Il progetto si basa sul controllore ESP8266 e un convertitore A/D a 16 bit per una maggiore precisione
nella lettura rispetto all'ingresso A/D del controllore (10 bit).
Il relè utilizzato è a stato solido per ridurre al minimo l'assorbimento del circuito.
La tensione letta, lo stato del relè, le soglie impostate sono visibili sul sul display da 1.3 pollici.
I due led segnalano la connessione al WiFi (Blu) e lo stato del relè (Giallo).
Il Dry contact può essere impostato collegandosi al suo web server con qualsiasi dispositivo connesso
alla stessa rete con un comunissimo browser. Una volta impostato i dati rimarranno persistenti sul
dispositivo.

Integrazione in Home Assistant:
Installare un broker MQTT in HA (mosquito)
Il dispositivo pubblica i dati con il protocollo MQTT e dall'interfaccia di HA è possibile visualizzare
e modificare le impostazioni.


Alla prima accensione il dispositivo entrerà in modalita AP. Connettersi alla wifi generata
(DryContactAP), selezionare la wifi alla quale si deve connettere, inserite i dati del server MQTT e salvare.

![Screenshot 2025-01-30 151114](https://github.com/user-attachments/assets/af58bd3d-ebaa-4d13-8f47-f0ffa753abf7)
![Screenshot 2025-01-30 151140](https://github.com/user-attachments/assets/678acf27-c2c5-4430-88f3-971afd9bd079)


Al riavvio il dispositivo si connetterà alla Vs. rete Wifi e sul display apparirà l'indirizzo IP assegnato dal router.
Collegarsi con un browser e settare i parametri "Tensione Alta, Tensione Bassa, Calibrazione, Tensione CutOff"

![Screenshot_2025-01-09-14-38-27-692_com android chrome 2](https://github.com/user-attachments/assets/43f43b96-98fb-4c04-8b76-686de75a5c4c)

![Screenshot 2025-01-30 162652](https://github.com/user-attachments/assets/9b3f14d4-0ef5-4323-a48c-7bc6f2c0b2a7)

Funzionamento:  Il contatto del relè è normalmente aperto e quando la tensione della batteria raggiunge la Tensione Alta il Relè si attiva, quando la batteria scende e la tensione raggiunge la Tensione Bassa il relè si disattiva.
E' possibile attivare il relè manualmente da web oppure HA anche se la tensione della batteria non ha ancora raggiunto la
soglia alta (funzionamento manuale). In questo stato la soglia bassa non interviene. Quando la batteria raggiunge la soglia alta lo stato manuale viene resettato e le soglie vengono riattivate per il normale funzionamento. Se la batteria non dovesse raggiungere nello stato manuale la soglia alta, sarà la TENSIONE DI CUTOFF ad intervenire e sesettare lo stato manuale e riportarlo al funzionamento automatico.

Calibrazione: Per allineare la lettura della tensione del Dry Contact rispetto alla tensione letta sul display della batteria
agire sul parametro. il valore 100 aumenta la lettura di 100 mV. sono ammessi anche valori negativi (-10).

La connessione Wi-Fi e quindi eventuali automazioni ad esso collegate potrebbero non funzionare nel caso di assenza di connettività, per questo è stato previsto
in tal caso il funzionamento stand alone del dispositivo con le impostazioni memorizzate.

LISTA COMPONENTI: 
N.1 Wemos D1 R1 esp8266
N.1 ADS1115 I2C (Convertitore A/D 16bit)
N.1 AZ-delivery 1,3 I2C OLED DISPLAY
N.1 SSR-41FDA relè stato solido
N.1 Modulo Step-Down USB DC convertitore Buck da 9-85V a 5V
N.2 Led
N.1 Switch Reset
N.3 Resistenze 1.5K
N.1 Resistenza 100K
N.1 Resistenza 2.2K
N.1 Condensatore 0.1


