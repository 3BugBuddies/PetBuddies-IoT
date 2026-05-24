# PetBuddies - IoT

Projeto de IoT do grupo **BugBuddies** para o ecossistema **PetBuddies**. A proposta é simular dispositivos conectados para apoiar uma clínica veterinária, registrando eventos de check-in por RFID e leituras de temperatura associadas ao pet.

## Integrantes:

| Nome | RM |
| --- | --- |
| Felipe Yuiti Ishii | RM 565339 |
| Gabriel Nogueira | RM 563925 |
| Giovanna Neri dos Santos | RM 566154 |
| Mariana Inoue | RM 565834 |


## Por Que Montamos Este Circuito

O PetBuddies é um sistema voltado ao acompanhamento de pets em ambiente veterinário. Dentro desse contexto, o circuito IoT foi pensado para automatizar duas situações comuns da rotina de atendimento:

- identificar a chegada de um pet na recepção por meio de uma tag RFID;
- registrar a temperatura do pet associada ao UID da tag RFID.

Com isso, o sistema reduz registros manuais, evita perda de informações e cria uma base de dados que pode ser consumida por outras partes da aplicação PetBuddies.

## Como o Sistema Funciona

O projeto é dividido em dois protótipos simulados no Wokwi.

O primeiro protótipo é o **Check-in PetBuddies**. Ele usa um ESP32 conectado a um leitor RFID MFRC522. Quando uma tag é aproximada, o ESP32 lê o UID da tag e publica esse dado em um canal privado do ThingSpeak usando MQTT.

O segundo protótipo é o **Temperature PetBuddies**. Ele usa o mesmo conceito de identificação por RFID, mas também lê a temperatura pelo sensor DS18B20. O ESP32 publica o UID e a temperatura em outro canal privado do ThingSpeak.

O Node-RED assina os canais MQTT do ThingSpeak, trata os dados recebidos e envia as informações para um banco MySQL hospedado no Railway.

```mermaid
flowchart LR
  A["Tag RFID"] --> B["ESP32 + MFRC522"]
  C["Sensor DS18B20"] --> D["ESP32 + MFRC522 + DS18B20"]
  B --> E["ThingSpeak MQTT - Canal Check-in"]
  D --> F["ThingSpeak MQTT - Canal Temperatura"]
  E --> G["Node-RED"]
  F --> G
  G --> H["MySQL no Railway"]
  H --> I["Sistema PetBuddies"]
```

## Estrutura do Repositório

```txt
.
├── CheckIn PetBuddies/
│   ├── sketch.ino
│   └── libraries.txt
├── Temperature PetBuddies/
│   ├── sketch.ino
│   └── libraries.txt
├── node-red/
│   └── flows.json 
└── README.md
```

## Tecnologias Utilizadas

### Bibliotecas Arduino

- `WiFi.h`
- `PubSubClient.h`
- `SPI.h`
- `MFRC522.h`
- `OneWire.h`
- `DallasTemperature.h`

### Plataformas

- **Wokwi**: simulação dos circuitos e execução dos sketches.
- **ThingSpeak**: servidor MQTT privado e visualização dos dados por canais.
- **Node-RED**: integração entre ThingSpeak e banco de dados.
- **Railway**: hospedagem do banco MySQL.
- **MySQL**: persistência dos registros.
- **GitHub**: versionamento e documentação do projeto.

## Montagem Física do Circuito

### Check-in PetBuddies

Componentes:

- ESP32
- Leitor RFID MFRC522
- Tag RFID

Pinagem utilizada:

| MFRC522 | ESP32 | Função |
| --- | --- | --- |
| SDA/SS | GPIO 5 | Seleção SPI |
| SCK | GPIO 18 | Clock SPI |
| MOSI | GPIO 23 | Dados SPI |
| MISO | GPIO 19 | Dados SPI |
| RST | GPIO 22 | Reset do módulo |
| 3.3V | 3.3V | Alimentação |
| GND | GND | Terra |

Funcionamento: quando a tag RFID é aproximada do leitor, o ESP32 captura o UID e envia ao ThingSpeak:

```txt
field1 = UID
field2 = recepcao
```

### Temperature PetBuddies

Componentes:

- ESP32
- Leitor RFID MFRC522
- Tag RFID
- Sensor DS18B20
- Resistor de 4.7 kOhm para o barramento OneWire do DS18B20

Pinagem adicional do DS18B20:

| DS18B20 | ESP32 | Função |
| --- | --- | --- |
| DATA | GPIO 4 | Barramento OneWire |
| VCC | 3.3V | Alimentação |
| GND | GND | Terra |

O resistor de 4.7 kOhm deve ficar entre o pino DATA do DS18B20 (`GPIO 4`) e o 3.3V. Sem esse resistor, o sensor pode retornar erro de leitura.

Funcionamento: quando a tag RFID é lida, o ESP32 também lê a temperatura e envia ao ThingSpeak:

```txt
field1 = UID
field2 = temperatura
```

## Configuração Para Fazer Funcionar

### 1. Montagem do circuito

Seguir a montagem do circuito seguindo os componentes e pinagem conforme listado acima

### 2. Configurar os Sketches

Os arquivos `sketch.ino` dos 2 projetos estarão as credenciais do servidor privado.
Foi disponibilizado com a entrega o `CREDENCIAIS.md` para preencher.

No arquivo `CheckIn PetBuddies/sketch.ino`, altere:

```cpp
const char* THINGSPEAK_CHANNEL_ID = "SEU_CHANNEL_ID_CHECKIN";
const char* MQTT_CLIENT_ID = "SEU_CLIENT_ID_MQTT";
const char* MQTT_USER      = "SEU_USERNAME_MQTT";
const char* MQTT_PASS      = "SUA_SENHA_MQTT";
```

No arquivo `Temperature PetBuddies/sketch.ino`, altere:

```cpp
const char* THINGSPEAK_CHANNEL_ID = "SEU_CHANNEL_ID_TEMPERATURA";
const char* MQTT_CLIENT_ID = "SEU_CLIENT_ID_MQTT";
const char* MQTT_USER      = "SEU_USERNAME_MQTT";
const char* MQTT_PASS      = "SUA_SENHA_MQTT";
```

O broker MQTT usado pelos dois sketches é:

```txt
mqtt3.thingspeak.com
porta 1883
```

## Resultados Parciais

- O ESP32 conectou ao Wi-Fi simulado do Wokwi.
- O leitor MFRC522 conseguiu capturar o UID das tags RFID.
- O sensor DS18B20 retornou leituras de temperatura em Celsius.
- Os dois sketches publicaram dados em canais privados do ThingSpeak via MQTT.
- O Node-RED conseguiu assinar os canais do ThingSpeak.
- O fluxo do Node-RED preparou os dados para gravação no MySQL do Railway.

## Link do Vídeo do Youtube

`https://www.youtube.com/watch?v=nKNVvrJg3zI`