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
Al riavvio il dispositivo si connetterà alla Vs. rete Wifi e sul display apparirà l'indirizzo IP assegnato dal router.
Collegarsi con un browser e settare i parametri "Tensione Alta, Tensione Bassa, Calibrazione, Tensione CutOff"

![Screenshot_2025-01-09-14-38-27-692_com android chrome 2](https://github.com/user-attachments/assets/43f43b96-98fb-4c04-8b76-686de75a5c4c)

Funzionamento:  

