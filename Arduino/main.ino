#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <DHT.h>

//---------------DHT11 SENSOR---------------------------------
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp_value;
//------------------------------------------------------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 1, 153);
IPAddress myDns(192, 168, 0, 1);

EthernetClient client;

int HTTP_PORT = 80;
String HTTP_METHOD = "GET";
char ip_server[] = "ip-api.com";
const char time_server[] = "worldtimeapi.org";

String PATH_NAME = "/insert_temp.php";
String tempValueQueryString = "?temp_value=";
String tempDateQueryString = "&temp_date=";
String tempLatQueryString = "&lat=";
String tempLonQueryString = "&lon=";

const unsigned long HTTP_TIMEOUT = 10000;  // max response time from server
const size_t MAX_CONTENT_SIZE = 500;      // max size of the HTTP response

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10 * 1000;

struct clientData {
	String datetime;
	float lat;
	float lon;
};

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
		Serial.print(F(" DHCP assigned IP "));
		Serial.println(Ethernet.localIP());
	}
	dht.begin();
	delay(1000);
}

void loop() {
	clientData data;
	temp_value = dht.readTemperature();
	getTime(&data);
	getLocation(&data);
	sendToDb(temp_value, &data);
	wait();
}
void sendToDb(float temp_value, struct clientData *data) {
	Serial.println(F("Attempting to send to DB:"));
	//DEBUG
	Serial.println("From sendToDb: ");
	Serial.println(temp_value);
	Serial.println(data->datetime);
	Serial.println(data->lat);
	Serial.println(data->lon);
	//
	if (client.connect(ip, HTTP_PORT)) {
		// if connected:
		Serial.println(F("Connected to server"));
		// make a HTTP request:
		// Send HTTP request
		String encodedTempDate = urlEncode(data->datetime);
		//DEBUG
		Serial.println("Query to DB:");
		Serial.println(
				HTTP_METHOD + " " + PATH_NAME + tempValueQueryString
						+ String(temp_value) + "&temp_date=" + encodedTempDate
						+ tempLatQueryString + data->lat + tempLonQueryString
						+ data->lon + " HTTP/1.1");
		//
		client.println(
				HTTP_METHOD + " " + PATH_NAME + tempValueQueryString
						+ String(temp_value) + "&temp_date=" + encodedTempDate
						+ tempLatQueryString + data->lat + tempLonQueryString
						+ data->lon + " HTTP/1.1");
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
void getTime(struct clientData *data) {
	if (client.connect(time_server, 80)) {
		Serial.println(F("We have connected to Time Server"));
		if (sendRequestTime(time_server) && skipResponseHeaders()) {  //
			Serial.println(F("We got something"));
			if (readResponseContentTime(data)) {
				Serial.println("We've read time");
			}
		}
		disconnect();
	}
}

void getLocation(struct clientData *data) {
	if (client.connect(ip_server, 80)) {
		Serial.println(F("We have connected"));
		if (sendRequestLocation(ip_server) && skipResponseHeaders()) { //
			Serial.println("We got something");
			if (readResponseContentLocation(data)) {
//				printClientData(&data);
			}
		}

		sendToDb(temp_value, data);
	}
	disconnect();
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
	delay(200);
	return true;
}

bool sendRequestLocation(const char *host) {
	client.println("GET /json HTTP/1.1");
	client.print("Host: ");
	client.println(host);
	client.println("User-Agent: arduino-ethernet");
	client.println("Connection: close");
	client.println();
	delay(200);
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
//	else {
//		Serial.println("Response received from server:");
//		// Print the response content to see what the server sent (optional)
//		while (client.available()) {
//			Serial.write(client.read());
//		}
	return ok;
}

bool readResponseContentTime(struct clientData *data) {
	DynamicJsonDocument jsonTime(MAX_CONTENT_SIZE);
	DeserializationError error = deserializeJson(jsonTime, client);
	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		client.stop();
		return false;
	}
	data->datetime = jsonTime["datetime"].as<String>();
	//DEBUG
//	Serial.println("readResponseContentTime()");
//	Serial.println(data->datetime);
	//
	return true;
}

bool readResponseContentLocation(struct clientData *data) {
	DynamicJsonDocument jsonLocation(MAX_CONTENT_SIZE);
	DeserializationError error = deserializeJson(jsonLocation, client);

	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		client.stop();
		return false;
	}

	// Parse the JSON and extract the data
	data->lat = jsonLocation["lat"].as<float>();
	data->lon = jsonLocation["lon"].as<float>();

	return true;
}
void printClientData(const float temp_value, struct clientData *data) {
	//DEBUG
	Serial.println("From printClientData: ");
	//
	Serial.println(temp_value);
	Serial.println(data->datetime);
	Serial.println(data->lat);
	Serial.println(data->lon);
}

void disconnect() {
	Serial.println("Disconnect");
	client.stop();
}

void wait() {
	Serial.println("Wait 2min");
	delay(120000);
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
