#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <DHT.h>

//---------------DHT11 SENSOR---------------------------------
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//------------------------------------------------------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 1, 153);
IPAddress myDns(192, 168, 0, 1);

EthernetClient client;

int HTTP_PORT = 80;
String HTTP_METHOD = "GET";
const char time_server[] = "worldtimeapi.org";

String PATH_NAME = "/insert_temp.php";
String tempValueQueryString = "?temp_value=";
String tempDateQueryString = "&temp_date=";

const unsigned long HTTP_TIMEOUT = 10000;  // max response time from server
const size_t MAX_CONTENT_SIZE = 500;      // max size of the HTTP response

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10 * 1000;

struct clientData {
	String datetime;
};

DynamicJsonDocument json(MAX_CONTENT_SIZE);

float init_num = 1;

void setup() {
	Serial.begin(9600);
	while (!Serial) {
		;
	}

	// start the Ethernet connection:
	Serial.println("Initialize Ethernet with DHCP:");
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// Check for Ethernet hardware present
		if (Ethernet.hardwareStatus() == EthernetNoHardware) {
			Serial.println(
					"Ethernet shield was not found. Sorry, can't run without hardware. :(");
			while (true) {
				delay(100);  // do nothing
			}
		}
		if (Ethernet.linkStatus() == LinkOFF) {
			Serial.println("Ethernet cable is not connected.");
		}
		Ethernet.begin(mac, ip, myDns);
		Serial.print(F("My IP address: "));
		Serial.println(Ethernet.localIP());
	} else {
		Serial.print(F("  DHCP assigned IP "));
		Serial.println(Ethernet.localIP());
	}
	dht.begin();
	delay(1000);
}

void loop() {

	getTime(); // @suppress("Invalid arguments")
	float temp_value = dht.readTemperature();
	Serial.print(F("We have read this temperature: "));
	Serial.println(temp_value);
	sendToDb(temp_value, json["datetime"].as<String>());

	wait(); // @suppress("Invalid arguments")
}
void sendToDb(float temp_value, String datetime) {
	Serial.println(F("Attempting to send to DB"));
	if (client.connect(ip, HTTP_PORT)) {
		// if connected:
		Serial.println(F("Connected to server"));
		// make a HTTP request:
		// Send HTTP request
		String encodedTempDate = urlEncode(datetime);
		client.println(
				HTTP_METHOD + " " + PATH_NAME + tempValueQueryString
						+ String(temp_value) + "&temp_date=" + encodedTempDate
						+ " HTTP/1.1");
		client.println(F("Host: arduino-ethernet"));

		client.println(F("Connection: close"));
		client.println();  // end HTTP header

		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.print(c);
			}
		}

		// the server's disconnected, stop the client:
		client.stop();
		Serial.println();
		Serial.println(F("disconnected"));
	}

	else {  // if not connected:
		Serial.println(F("connection failed"));
	}
}
void getTime() {
	Serial.println(F("Attempting to get TIME"));
	if (client.connect(time_server, 80)) {
		Serial.println(F("We have connected to Time Server"));
		if (sendRequestTime(time_server) && skipResponseHeaders()) {  //
			Serial.println(F("We got something"));
			clientData data;
			if (readResponseContentTime(&data)) { // @suppress("Invalid arguments")
				printClientData(&data);
			}
		}
		disconnect();
	}
}

bool sendRequestTime(const char *host) {
	Serial.println("Sending Time request");
	char request[100];
	sprintf(request, "GET /api/ip HTTP/1.1");
	client.println(request);
	client.print("Host: ");
	client.println(host);
	client.println("Connection: close");
	client.println();
	return true;
}

bool skipResponseHeaders() {
	// HTTP headers
	char endOfHeaders[] = "\r\n\r\n";
	client.setTimeout(HTTP_TIMEOUT);
	bool ok = client.find(endOfHeaders);

	if (!ok) {
		Serial.println("No response or invalid response!");
	}
	return ok;
}

bool readResponseContentTime(struct clientData *data) {
	DeserializationError error = deserializeJson(json, client);
	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		client.stop();
		return false;
	}
	data->datetime = json["datetime"].as<String>();
	return true;
}

void printClientData(const struct clientData *data) {
	Serial.print("Time = ");
	Serial.println(data->datetime);
}

void disconnect() {
	Serial.println("Disconnect");
	client.stop();
}

void wait() {
	Serial.println("Wait 30 seconds");
	delay(50000);
}

String urlEncode(const String &str) {
	String encodedString = "";
	char c;
	char code[4];

	for (uint8_t i = 0; i < str.length(); i++) {
		c = str.charAt(i);
		if (isalnum(c)) {
			encodedString += c;
		} else if (c == ' ') {
			encodedString += '+';
		} else {
			sprintf(code, "%%%02X", c);
			encodedString += code;
		}
	}

	return encodedString;
}
