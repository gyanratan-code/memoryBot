#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <string>
#include <Vector>
Vector<int> timestamp;
Vector<String> movement_record;
//rotation time must be in milliseconds
String message_readable[]={"forward","backward","right","left","reverse operation processing"};
int client_id_stored;
int rotation_time= 78;
int time_start=0; 
int time_end=0;
int buffer=0;
String data234="hello";
const int pin1=D5;    //D7
const int pin2=D6;    //D8
const int enable_pin1=D7; //D5
const int enable_pin2=D1;
const int pin3= D2;
const int pin4=D3;
IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
//webserver setup
AsyncWebServer server(80);
//websocket setup
AsyncWebSocket ws("/ws");
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    //client connected
    os_printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client_id_stored=client->id();
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    os_printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    os_printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    os_printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      os_printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        os_printf("%s\n", (char*)data);
        // hecker
        data234=(char*)data;
        loop();
      } else {
        for(size_t i=0; i < info->len; i++){
          os_printf("%02x ", data[i]);
        }
        os_printf("\n");
      }
      if(info->opcode == WS_TEXT){
        client->text("I got your message");// i editedhere.
      }
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          os_printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        os_printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      os_printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
      if(info->message_opcode == WS_TEXT){
        data[len] = 0;
        os_printf("%s\n", (char*)data);
      } else {
        for(size_t i=0; i < len; i++){
          os_printf("%02x ", data[i]);
        }
        os_printf("\n");
      }

      if((info->index + len) == info->len){
        os_printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          os_printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}
const char index_html[] PROGMEM = R"rawliteral("
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>To and Fro robot control</title>
    <style>
        /* write stylesheet code here */
        body{
            margin: 0px;
        }
        nav{
            width: 100%;
            position: fixed;
            align-items: center;
            font-size: 40px;
            font-weight: bolder;
        }
        header{
            width: 100%;
            padding: 10px;
            background-color: aliceblue;
        }
        main{
            display: flex;
            padding-top: 90px;
        }
        .control-element{
            width: 100%;
            height: 60%;
            text-align: center;
        }
        .highlight{
            outline: 2px solid blue;
        }
        area{
            cursor: pointer;
        }
        #text-output{
            border: 2px solid grey;widows: 100%;
            margin-bottom: 5px;
        }
        footer{
            width: 100%;
            position: fixed;
            bottom: 0;
            align-content: right;
            background-color: #CFC9C8;
        }
        #server_response{
            text-align: center;
            outline: 2px solid grey;
        }
        .cursor{
            display: none;
        }
        input{
            margin-top: 18px;
            width: 90%;
            padding: 5px;
            margin-bottom: 5px;
        }
        canvas{
            border: 1px solid #000;
/*            width: 90%;
            height: 350px;
*/            width: 373px;
            height: 352px;
            /*margin-bottom: 60px;*/
        }
    </style>
</head>
<body>
    <nav>
        <header>Robot Controlling</header>
    </nav>
    <main>
        <div class='control-element'>
            <img id='con' usemap='#arrow' src='data:image/png;base64,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' height='300'>
            <map name='arrow'>
                <area shape='rect' coords='0,120,96,182'  onclick='left()' alt='Left'>
                <area shape='rect' coords='112,0,170,100'  onclick='forward()' alt='Forward'>
                <area shape='rect' coords='196,118,293,181'  onclick='right()' alt='Right'>
                <area shape='rect' coords='114,200,178,298'  onclick='backward()' alt='Backward'>
                <area shape='circle' coords='145,153,32' onclick='stop()' alt='Stop'>
            </map>
        <div> <input id="reverse_button" type='button' name='button' value='Reverse' onclick="reverse()"> <div>
        <div id='text-output'>Info about movement will appear here</div>
        <div id='visual-output'>
            <canvas width="373px" height="352px"></canvas>
        </div>
    </main>
    <footer>
        <div id="server_response">Server Response will appear here.</div>
        <span style="font-size: x-small;">Gyan Software</span>
    </footer>
    <script async>
        text_output= document.getElementById('text-output');
        server_response= document.getElementById('server_response');
        // image= document.getElementById('con');
        image= document.getElementById('con');
        reverse_button= document.getElementById('reverse_button');
        function left(){
            text_output.innerHTML='Going Left';
            socket.send(3);
        }
        function forward(){
            text_output.innerHTML='Going Forward';
            socket.send(0);
        }
        function right(){
            text_output.innerHTML='Going Right';
            socket.send(2);
        }
        function backward(){
            text_output.innerHTML='Going Backward';
            socket.send(1);
        }
        function reverse(event){
            text_output.innerHTML="Reversing the path, Controls disabled.";
            reverse_button.setAttribute('disabled','true');
            disable_movement();
            socket.send(4);
        }
        function stop(){
          text_output.innerHTML='Stopping';
          socket.send(5);
        }
        function disable_movement(){
            image.classList.add('cursor');

            console.log("I am called.")
        }
        //websocket connection handling
        const socket= new WebSocket('ws://192.168.4.22/ws');
        socket.addEventListener('open',function(){
            console.log('Connected to web server.')
        })
        socket.addEventListener('message',function(event){
            // text_output.innerHTML('Task Completed');
            console.log(event.data);
            server_response.innerHTML= event.data;
            if(event.data=='Path retrace done.'){
                image.classList.remove('cursor');
                reverse_button.removeAttribute('disabled');
            }
        })
        socket.addEventListener('close',function(){
            console.log('Connected closed');
        })
    </script>
</body>
</html>
")rawliteral";
void showError(AsyncWebServerRequest *request){
  //Handle Unknown Request
  request->send(404,"text/html","<html><head></head><body><h1 style='text-align:center'>404! Not found</h1></body></html>");
}
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN,OUTPUT); //setting built_in led as indicator
  pinMode(pin1,OUTPUT);
  pinMode(pin2,OUTPUT);
  pinMode(enable_pin1,OUTPUT);
  pinMode(enable_pin2,OUTPUT);
  pinMode(pin3,OUTPUT);
  pinMode(pin4,OUTPUT);
  Serial.begin(115200); //baud rate for communication
  Serial.println();
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("FDP_Group:15","xxxxxxxx") ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  //web socket server handling
  server.addHandler(&ws);
  ws.onEvent(onEvent);
  //request handling
  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send_P(200,"text/html",index_html);
  });
  server.on("/execute",HTTP_GET,[](AsyncWebServerRequest *request){
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){ //p->isPost() is also true
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
    request->send_P(200,"text/html","Content reading");
  });
  server.onNotFound(showError);
  server.begin();
  digitalWrite(LED_BUILTIN,HIGH);
}
void stop(){
    digitalWrite(enable_pin1,LOW);
    digitalWrite(enable_pin2,LOW);
}
void forward(){
    digitalWrite(enable_pin1,HIGH);
    digitalWrite(enable_pin2,HIGH);
    time_start= millis();
    buffer=1;
    
    digitalWrite(pin1,HIGH);
    digitalWrite(pin2,LOW);
    digitalWrite(pin3,HIGH);
    digitalWrite(pin4,LOW);
}
void backward(){
    digitalWrite(enable_pin1,HIGH);
    digitalWrite(enable_pin2,HIGH);
    time_start= millis();
    buffer=1;
    
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,HIGH);
    digitalWrite(pin3,LOW);
    digitalWrite(pin4,HIGH);
}
void left(){
    digitalWrite(enable_pin1,HIGH);
    digitalWrite(enable_pin2,HIGH);
    //time_start= millis();
    
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,HIGH);
    digitalWrite(pin3,HIGH);
    digitalWrite(pin4,LOW);
    delay(rotation_time);
    timestamp.push_back(-1);
}
void right(){
    digitalWrite(enable_pin1,HIGH);
    digitalWrite(enable_pin2,HIGH);
    //time_start= millis();
    
    digitalWrite(pin1,HIGH);
    digitalWrite(pin2,LOW);
    digitalWrite(pin3,LOW);
    digitalWrite(pin4,HIGH);
    delay(rotation_time);
    timestamp.push_back(-1);
}
void reverse(){
  //retraces the path
  digitalWrite(enable_pin1,HIGH);
  digitalWrite(enable_pin2,HIGH);
  digitalWrite(pin1,LOW);
  digitalWrite(pin2,HIGH);
  digitalWrite(pin3,HIGH);
  digitalWrite(pin4,LOW);
  delay(rotation_time*2);
  for(int i=movement_record.size()-1;i>=0;i--){
    if(movement_record[i]=="0"){
        forward();
        delay(timestamp[i]);
    }
    else if(movement_record[i]=="1"){
        backward();
        delay(timestamp[i]);
    }
    else if(movement_record[i]=="2"){
        left();
        stop();
    }
    else if(movement_record[i]=="3"){
        right();
        stop();
    }
  }
  movement_record.clear();
  timestamp.clear();
  ws.text((uint32_t)client_id_stored,"Path retrace done.");
}
void loop() {
  // put your main code here, to run repeatedly:
  if(data234=="0"){
    //forward
    if(buffer==1){
        timestamp.push_back(millis()-time_start);
    }
    movement_record.push_back(data234);
    forward();
  }
  else if(data234=="1"){
    //backward
    if(buffer==1){
        timestamp.push_back(millis()-time_start);
    }
    movement_record.push_back(data234);
    backward();
  }
  else if(data234=="2"){
    //going right
    if(buffer==1){
        timestamp.push_back(millis()-time_start);
        buffer=0;
    }
    movement_record.push_back(data234);
    right();
    data234=5;
  }
  else if(data234=="3"){
    //going left
    if(buffer==1){
        timestamp.push_back(millis()-time_start);
        buffer=0;
    }
    movement_record.push_back(data234);
    left();
    data234=5;
  }
  else if(data234=="4"){
    reverse();
    data234="5";
  }
  else{
    //stop at that place
    if(buffer==1){
        timestamp.push_back(millis()-time_start);
        buffer=0;
    }
    stop();
    digitalWrite(LED_BUILTIN,LOW);
  }
}
