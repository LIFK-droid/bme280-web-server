//На данном этапе подключаем библиотеки для работы bme280 с esp32
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; 

//Ставим свои данные по WiFi
const char* ssid     = "Введите название сети";
const char* password = "Введите пароль сети";

// Устанавливаем номер веб-порта на 80
WiFiServer server(80);

// Переменная для хранения HTTP-запроса
String header;

// В данный момент времени
unsigned long currentTime = millis();
// Прошлый раз
unsigned long previousTime = 0; 
// Определяем время ожидания в миллисекундах (пример: 2000мс = 2с)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  bool status;

 
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Подключение к сети WiFi с логином и паролем
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Здесь выводится локальный IP-адрес для запуска веб-сервера
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   

  if (client) {                             // Если подключается новый клиент
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // Выводит сообщение в Serial Port
    String currentLine = "";                // Создаём строку для хранения входящих данных от клиента
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // Цикл, если клиент подключился
      currentTime = millis();
      if (client.available()) {             // Если есть байты для чтения с клиента,
        char c = client.read();             // Читает байт, тогда
        Serial.write(c);                    // Выводит это в serial monitor
        header += c;
        if (c == '\n') {                    // Если байт является символом новой строки
          // Если текущая строка пуста, то вы получили два символа новой строки подряд.
          if (currentLine.length() == 0) {
            // Выводится содержимое, чтобы пользователь знал, что его ждёт, а потом пустая строка:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Выводит HTML веб-страницу
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Вводим CSS для оформления таблицы
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }");
            
            // Вводим код для заголовка веб-страницы
            client.println("</style></head><body><h1>ESP32 with BME280</h1>");
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(bme.readTemperature());
            client.println(" *C</span></td></tr>");  
            client.println("<tr><td>Temp. Fahrenheit</td><td><span class=\"sensor\">");
            client.println(1.8 * bme.readTemperature() + 32);
            client.println(" *F</span></td></tr>");       
            client.println("<tr><td>Pressure</td><td><span class=\"sensor\">");
            client.println(bme.readPressure() / 100.0F);
            client.println(" hPa</span></td></tr>");
            client.println("<tr><td>Approx. Altitude</td><td><span class=\"sensor\">");
            client.println(bme.readAltitude(SEALEVELPRESSURE_HPA));
            client.println(" m</span></td></tr>"); 
            client.println("<tr><td>Humidity</td><td><span class=\"sensor\">");
            client.println(bme.readHumidity());
            client.println(" %</span></td></tr>"); 
            client.println("</body></html>");
            
            // Ответ HTTP заканчивается пустой строкой
            client.println();
            // Выходим из цикла while
            break;
          } else { // При выведении новой строки очистит текущую
            currentLine = "";
          }
        } else if (c != '\r') {  // Если у вас есть что-нибудь еще,
          currentLine += c;      // Добавьте его в конец текущей строки
        }
      }
    }
    // Очищаем переменную заголовка
    header = "";
    // Закрываем соединение
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
